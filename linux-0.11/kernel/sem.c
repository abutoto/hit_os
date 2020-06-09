/*
    2020.06.07 abutoto
    semaphore on linux-0.11
*/
#include <linux/sched.h>
#include <unistd.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <linux/sem.h>
#include <linux/kernel.h>

sem_t semtable[SEM_CNT];

void show_queue(sem_t *sem, char *name)
{
    int i;
    fprintk(3, "%s | %s | value %d | cnt %d | pid %d | %d\n", name, sem->name, sem->value, sem->cnt, current->pid, current);
    if(sem->cnt == 0)
        return;
    for(i = 0; i < sem->cnt; i++){
        fprintk(3, "%d ", sem->queue[i]->pid);
    }
    fprintk(3, "\n");
}

int search(sem_t *sem, struct task_struct *target){
    int i;
    for(i = 0; i < sem->cnt; i++){
        if(sem->queue[i] == target){
            return i;
        }
    }
    return -1;
}
sem_t *sys_sem_open(const char *name, unsigned int value)
{
    int i, j, empty = -1;
    sem_t *sem = NULL;
    char kernelname[100];
    int len = 0;
    while(len <= SEM_NAME_LEN && (kernelname[len]=get_fs_byte(name+len)) != '\0')len++;
    if(len > SEM_NAME_LEN) return NULL;
    int sem_name_len;
    for(i=0; i<SEM_CNT; i++){
        sem_name_len = strlen(semtable[i].name);
        if(sem_name_len == len && !strcmp(kernelname, semtable[i].name)){
            sem = &semtable[i];
            break;
        }
        if(empty == -1 && semtable[i].name[0] == '\0')empty = i;
    }

    if(sem == NULL){
        j = 0;
        while(j < len)semtable[empty].name[j] = kernelname[j++];
        semtable[empty].name[j] = '\0';
        semtable[empty].value = value;
        semtable[empty].cnt = 0;
        sem = &semtable[empty];
    }
    return sem;
}

int sys_sem_wait(sem_t *sem)
{
    int i, match=-1;
    cli();
    sem->value--;
    if(sem->value < 0){
        match = search(sem, current);
        if(match == -1){
            sem->queue[sem->cnt] = current;
            match = sem->cnt;
            sem->cnt++;
        }
        sleep_on(&sem->queue[match]);
    }
    sti();
    return 0;
}

int sys_sem_post(sem_t *sem)
{
    cli();
    sem->value++;
    if(sem->value <= 0){
        wake_up(&sem->queue[0]);
        sem->queue[0] = sem->queue[sem->cnt - 1];
        sem->cnt--;
    }
    sti();
    return 0;
}

int sys_sem_unlink(const char *name)
{
    int i, j, sd = -1;
    char kernelname[100];
    int len = 0;
    while(len <= SEM_NAME_LEN && (kernelname[len]=get_fs_byte(name+len)) != '\0')len++;
    if(len > SEM_NAME_LEN) return NULL;
    int sem_name_len;
    for(i=0; i<SEM_CNT; i++){
        sem_name_len = strlen(semtable[i].name);
        if(sem_name_len == len && !strcmp(kernelname, semtable[i].name)){
            sd = i;
            break;
        }
    }
    if(sd != -1){
        semtable[sd].name[0] = '\0';
        sd = 0;
    }
    return sd;
}

