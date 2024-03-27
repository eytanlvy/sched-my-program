#include "sched.h"
#include<unistd.h>
#include <pthread.h>

typedef struct task{
    taskfunc function;
    void* arg;
} task;

typedef struct scheduler{
    int nthreads;          // Nombre de threads
    int qlen;              // Nombre maximum de tâches simultanées
    pthread_t *threads;    // Tableau des threads
    int stack_size;        // Taille actuelle de la pile
    task *stack;           // Pile de tâches
} scheduler;

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){//créé un ordonnanceur.
    if(nthreads < 0){
        write(2,"[!] Error : wrong parameter in sched_init, nthreads must be >= 0\n",65);
        return -1;
    }
    if(qlen <= 0){
        write(2,"[!] Error : wrong parameter in sched_init, qlen must be > 0\n",60);
        return -1;
    }
    if(!nthreads){
        nthreads = sched_default_threads(); 
    }
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
    return 1;
}
