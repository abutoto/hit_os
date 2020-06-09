#define __LIBRARY__
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <linux/sem.h>

_syscall2(int, sem_open, const char*, name, unsigned int, value)
_syscall1(int, sem_wait, sem_t *, sem)
_syscall1(int, sem_post, sem_t *, sem)
_syscall1(int, sem_unlink, const char *, name)

#define M 500
#define BUFF_LEN 10
#define CONSUMER_NUM 5

void clear_sem(){
    sem_unlink("Empty");
    sem_unlink("Full");
    sem_unlink("Mutex");
}

void Producer(pid_t pid, int fd)
{
    int i, stat;
    sem_t *empty, *full, *mutex;
    /* printf("Producer %d run, file %d !!!\n", pid, fd); */
    fflush(stdout);

    empty = sem_open("Empty", BUFF_LEN);
    full = sem_open("Full", 0);
    mutex = sem_open("Mutex", 1);

    for(i=1; i<=M; i++){
        sem_wait(empty);
        sem_wait(mutex);

        stat = lseek(fd, 0, SEEK_CUR);
        /* printf("before write pos %d !!!\n", stat); */

        if((stat=write(fd, &i, sizeof(int))) == -1){
            printf("write %d error !!!\n", i);
        }
        else {
            /* printf("write %d succeed !!!\n", i); */
        }

        stat = lseek(fd, 0, SEEK_CUR);
        /* printf("after write pos %d !!!\n", stat); */
        fflush(stdout);

        sem_post(mutex);
        sem_post(full);
    }
}

void Consumer(pid_t pid, int fd)
{
    int i, stat, pos, buff[10];
    sem_t *empty, *full, *mutex;
    /* printf("Consumer %d run, file %d !!!\n", pid, fd); */
    fflush(stdout);

    empty = sem_open("Empty", BUFF_LEN);
    full = sem_open("Full", 0);
    mutex = sem_open("Mutex", 1);

    for(i=1; i<=M/CONSUMER_NUM; i++){
        sem_wait(full);
        sem_wait(mutex);
        if((pos=lseek(fd, 0, SEEK_CUR)) == -1){
            printf("lseek now error!!!\n");
        }
        else {
            /* printf("read pos %d\n", pos); */
        }
        if((stat=lseek(fd, 0, SEEK_SET)) == -1){
            printf("lseek head error!!!\n");
        }
        if((stat=read(fd, &buff, sizeof(int)*10)) == -1){
            printf("read buff error!!!\n");
        }
        else {
            printf("%d: %d\n", pid, buff[0]);
        }
        fflush(stdout);

        if((stat=lseek(fd, 0, SEEK_SET)) == -1){
            printf("lseek head error!!!\n");
        }
        for(stat=0; stat<BUFF_LEN-1; stat++){
            buff[stat] = buff[stat + 1];
        }
        buff[BUFF_LEN - 1] = -1;
        if((stat=write(fd, &buff, sizeof(int)*10)) == -1){
            printf("read write error !!!\n");
        }
        if((stat=lseek(fd, pos-sizeof(int), SEEK_SET)) == -1){
            printf("lseek write error!!!\n");
        }

        stat = lseek(fd, 0, SEEK_CUR);
        /* printf("after read pos %d !!!\n", stat); */

        fflush(stdout);
        sem_post(mutex);
        sem_post(empty);
    }
}

int main(int argc, char * argv[])
{
    int fd, i;
    pid_t father, child;
    int status = 0;
    pid_t wpid;

    clear_sem();

    if( (fd=open("./buffer.txt", O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR)) < 0 ){
        printf("open error\n");
        exit(-1);
    }

    father = getpid();
    for(i=0; i<CONSUMER_NUM; i++){
        if((child=fork()) == 0){
            child = getpid();
            Consumer(child, fd);
            exit(0);
        }
    }
    if((child=fork()) == 0){
        child = getpid();
        Producer(child, fd);
        exit(0);
    }

    while ((wpid = wait(&status)) > 0);

    clear_sem();
    return 0;
}
