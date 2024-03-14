struct scheduler;

typedef void (*taskfunc)(void*, struct scheduler *);

static inline int
sched_default_threads()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure);
int sched_spawn(taskfunc f, void *closure, struct scheduler *s);
