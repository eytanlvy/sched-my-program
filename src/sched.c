#include "sched.h"
#include<unistd.h>

struct scheduler{
    int nthreads;
    int qlen;
};


int sched_init(int nthreads, int qlen, taskfunc f, void *closure){//créé un ordonnanceur.
    if(nthreads){
        nthreads = sched_default_threads(); 
    }
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
    
}