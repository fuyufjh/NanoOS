#include "kernel.h"
#include "process.h"
#include "adt/list.h"

#define MSG_POOL_SIZE 512
static Msg msg_pool[MSG_POOL_SIZE];
static int p_free=0;

void send(pid_t dest, Msg *m)
{
    PCB* dest_proc = &pcb_pool[dest];
    m->dest = dest;
    if (m->src == 0) // if SRC was not set
        m->src = current->pid;
    msg_pool[p_free] = *m; // copy
    lock();
    list_add_before(&(dest_proc->msg_list), &(msg_pool[p_free++].list));
    if (p_free >= MSG_POOL_SIZE) p_free =0;
    unlock();
    V(&dest_proc->msg_sem);
    //printk("SENT: %d->%d (%d)\n",current->pid, dest, m->type);
}

void receive(pid_t src, Msg *m)
{
    Msg* msg_to_rcv;
    if (src==ANY)
    {
        P(&current->msg_sem);
        lock();
        msg_to_rcv = list_entry(current->msg_list.next,Msg,list);
        list_del(&(msg_to_rcv->list));
        unlock();
        *m = *msg_to_rcv;
        //printk("RECEIVED: %d<-%d (%d)\n",current->pid, msg_to_rcv->src, msg_to_rcv->type);
        return;
    }
    else
    {
        ListHead* p;
        while (1)
        {
            P(&current->msg_sem);
            list_foreach(p, &current->msg_list)
            {
                if (list_entry(p,Msg,list)->src == src)
                {
                    lock();
                    msg_to_rcv = list_entry(p,Msg,list);
                    list_del(&(msg_to_rcv->list));
                    unlock();
                    *m = *msg_to_rcv;
                    //printk("RECEIVED: %d<-%d\n",current->pid, msg_to_rcv->dest);
                    return;
                }
            }
            V(&current->msg_sem);
            wait_intr();
        }
    }
}
