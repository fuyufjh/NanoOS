#include "kernel.h"
#include "hal.h"

pid_t FM;

#define NR_MAX_FILE 8
#define NR_FILE_SIZE (128 * 1024)

#define HASH_TABLE_SIZE 4
static ListHead ramdisk_hash_table[HASH_TABLE_SIZE];
static Sem mutex;

typedef struct {
    int name;
    unsigned size;
    unsigned pos;
    ListHead list;
} FileNode;

static FileNode file[NR_MAX_FILE];
static int empty;

inline int hash_file_name(int name) {
    return name % HASH_TABLE_SIZE;
}

void do_new_file(int name) {
    file[empty].name = name;
    file[empty].pos = empty * NR_FILE_SIZE;
    int hash = hash_file_name(name);
    P(&mutex);
    list_add_before(&ramdisk_hash_table[hash], &file[empty].list);
    V(&mutex);
    empty++;
}

void do_read(int file_name, uint8_t *buf, off_t offset, size_t len) {
    int hash = hash_file_name(file_name);
    P(&mutex);
    ListHead* p;
    list_foreach(p, &ramdisk_hash_table[hash]) {
        if (list_entry(p, FileNode, list)->name == file_name) break;
    }
    V(&mutex);
    if (p == &ramdisk_hash_table[hash]) {
        panic("There is no file named '%s'.",file_name);
    }
    dev_read("ram", current->pid, buf, list_entry(p, FileNode, list)->pos + offset, len);
}

#define FM_NEW_FILE 1
#define FM_READ 2

void ramdisk_thread() {
    static Msg m;
    while (true) {
        receive(ANY, &m);
        if (m.type == FM_NEW_FILE) {
            do_new_file(m.i[0]);
            m.ret = 0;
            m.dest = m.src;
            m.src = FM;
            send(m.dest, &m);
        }
        else if (m.type == FM_READ) {
            do_read(m.i[0], m.buf, m.offset, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = FM;
            send(m.dest, &m);
        }
        else
            assert(0);
    }
}

void init_fm(void) {
    int i;
    create_sem(&mutex, 1);
    P(&mutex);
    for (i=0; i<HASH_TABLE_SIZE; i++)
        list_init(&ramdisk_hash_table[i]);
    V(&mutex);

    PCB* p =create_kthread(ramdisk_thread);
    FM = p->pid;
    wakeup(p);
}
