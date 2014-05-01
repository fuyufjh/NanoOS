#include "kernel.h"
#include "process.h"
#include "adt/list.h"

void send(pid_t dest, Msg *m)
{
    PCB* dest_proc = &pcb_pool[dest];
    m->dest = dest;
    lock();
    list_add_before(&(dest_proc->ts.msg_list), &(m->list));
    unlock();
    V(&dest_proc->ts.msg_sem);
    //printk("SENT: %d->%d\n",current->ts.pid, dest);
}

void receive(pid_t src, Msg *m)
{
    Msg* msg_to_rcv;
    //printk("%d TRY TO RECEIVE from %d\n",current->ts.pid,src);
    if (src==ANY)
    {
        P(&current->ts.msg_sem);
        lock();
        msg_to_rcv = list_entry(current->ts.msg_list.next,Msg,list);
        list_del(&(msg_to_rcv->list));
        unlock();
        *m = *msg_to_rcv;
        //printk("RECEIVED: %d<-%d\n",current->ts.pid, msg_to_rcv->src);
        return;
    }
    else
    {
        ListHead* p;
        while (1)
        {
            P(&current->ts.msg_sem);
            //printk("P GET!!!!");
            list_foreach(p, &current->ts.msg_list)
            {
                //printk("CMP %x %x",list_entry(p,Msg,list)->src ,src);
                if (list_entry(p,Msg,list)->src == src)
                {
                    lock();
                    msg_to_rcv = list_entry(p,Msg,list);
                    list_del(&(msg_to_rcv->list));
                    unlock();
                    *m = *msg_to_rcv;
                    //printk("RECEIVED: %d<-%d\n",current->ts.pid, msg_to_rcv->dest);
                    return;
                }
            }
            V(&current->ts.msg_sem);
        }
    }
    //V(&current->ts.msg_sem);
    //sleep_sem(&current->ts.msg_sem);
}
