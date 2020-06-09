#include <linux/sched.h>

#define SEM_CNT 20
#define SEM_NAME_LEN 100

typedef struct {
	char name[SEM_NAME_LEN];
	int value;
    int cnt;
    struct task_struct *queue[NR_TASKS];

} sem_t;

