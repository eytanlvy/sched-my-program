#include <unistd.h>

typedef struct scheduler scheduler;

typedef void (*taskfunc)(void*, scheduler *);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, scheduler *s);
void *worker_thread(void* arg);