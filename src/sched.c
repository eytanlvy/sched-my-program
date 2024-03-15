#include "sched.h"
#include<unistd.h>
#include <pthread.h>

struct scheduler{
    int nthreads;          // Nombre de threads
    int qlen;              // Nombre maximum de tâches simultanées
    pthread_t *threads;    // Tableau des threads
    int stack_size;        // Taille actuelle de la pile
    taskfunc *stack;       // Pile de tâches
};


int sched_init(int nthreads, int qlen, taskfunc f, void *closure){//créé un ordonnanceur.
    if(nthreads){
        nthreads = sched_default_threads(); 
    }
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
    
}