#include "kernel.h"

pid_t MM;
pid_t PM;

typedef struct {
    CR3 cr3;
    PDE pdir[NR_PDE] align_to_page;
    /* We need just 1 page now */
    PTE ptable[NR_PTE] align_to_page;
} VMemInfo;

#define KERNEL_SIZE (16*1024*4096)
#define N_KERNEL_PTE (KERNEL_SIZE/PD_SIZE)

static VMemInfo vmem_table[N_KERNEL_PTE];
static int vmem_free = 0;

CR3 do_new_proc_vmem(pid_t pid) {
    VMemInfo* p = &vmem_table[vmem_free++];
    uint32_t pdir_idx;

    /* Set cr3 */
    p->cr3.val = 0;
    p->cr3.page_directory_base = ((uint32_t)va_to_pa(p->pdir)) >> 12;

    /* Initialize pdir
    for (pdir_idx = 0; pdir_idx < NR_PDE; pdir_idx++) {
        make_invalid_pde(&p->pdir[pdir_idx]);
    }*/
    for (pdir_idx = 0; pdir_idx < NR_PDE; pdir_idx++) {
        make_pde(&p->pdir[pdir_idx], va_to_pa(&p->ptable));
    }

    /* Initialize the kernel mapping address */
    PDE* kpdir = get_kpdir();
    for (pdir_idx = 0; pdir_idx < /*N_KERNEL_PTE*/16; pdir_idx ++) {
        p->pdir[pdir_idx + KOFFSET / PD_SIZE] = kpdir[pdir_idx + KOFFSET / PD_SIZE];
    }
    //printk("RIGHT ptable=%x",p->ptable);
    return p->cr3;
}

uint32_t do_allocate_new_pages(pid_t pid, uint8_t* va, size_t size) {
    uint32_t ptable_idx;

    CR3 cr3 = pcb_pool[pid].cr3; // CR3 is right
    PDE* pde = (PDE*) (((uint32_t *)(cr3.val & ~0xfff)) + ((uint32_t)va >> 22));
    //printk("pde=%x",pde);
    //PTE* ptable = (PTE*)(((uint8_t*)pde) + PAGE_SIZE);
    //printk("%x\n",pde->val);
    //if (pde->val == 0) {

    /* allocating here */
    PTE* ptable = (PTE*)(pde->val & ~0xfff);
    //printk("ptable=%x",ptable);
        for (ptable_idx = 0; ptable_idx < NR_PTE; ptable_idx++) {
            //make_pte(&ptable[ptable_idx], (void*)(((uint32_t)va & ~0x3fffff) + (ptable_idx << 12)));
            make_pte(&ptable[ptable_idx], (void*)(((32 + vmem_free * 4) << 20)+(ptable_idx << 12)) );
        }
    //}
    uint32_t pa= (ptable[((uint32_t)va >> 12) & 0x3ff].val & ~0x3ff) + ((uint32_t)va & 0xfff);
    return pa;
}

#define MM_NEW_PROC 515
#define MM_NEW_PAGE 516

void mm_thread() {
    static Msg m;
    while (true) {
        receive(ANY, &m);
        if (m.type == MM_NEW_PROC) {
            m.ret = do_new_proc_vmem(m.src).val;
            m.dest = PM;
            m.src = MM;
            send(m.dest, &m);
        }
        else if (m.type == MM_NEW_PAGE) {
            m.ret = do_allocate_new_pages(m.src, (uint8_t*)m.i[0], m.i[1]);
            m.dest = PM;
            m.src = MM;
            send(m.dest, &m);
        }
        else
            assert(0);
    }
}

void init_mm(void) {
    PCB* p = create_kthread(mm_thread);
    MM = p->pid;
    wakeup(p);
}

