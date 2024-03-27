#include "sched.h"
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>

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

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){//creates a scheduler
    if(qlen <= 0){
        write(2,"[!] Error : wrong parameter in sched_init, qlen must be > 0\n",60);
        return -1;
    }
    if (!f) {
        write(2,"[!] Error : wrong parameter in sched_init, taskfunc must be != NULL\n",69);
        return -1;
    }
    if(nthreads <= 0){
        nthreads = sched_default_threads();
    }

    //Scheduler creation
    scheduler *s = mmap(NULL, sizeof(scheduler), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(s == MAP_FAILED){
        perror("[!] Error : mmap failed in sched_init : ");
        return -1;
    }
    s->nthreads = nthreads;
    s->qlen = qlen;

    //Creation des threads
    s->threads = mmap(NULL, nthreads*sizeof(pthread_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(s->threads == MAP_FAILED){
        perror("[!] Error : mmap failed in sched_init : ");
        munmap(s, sizeof(scheduler));
        return -1;
    }

    //Tasks stack creation
    s->stack_size = 0;
    s->stack = mmap(NULL, qlen*sizeof(task), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s->stack == MAP_FAILED) {
        perror("[!] Error : mmap failed in sched_init : ");
        munmap(s->threads, nthreads*sizeof(pthread_t));
        munmap(s, sizeof(scheduler));
        return -1;
    }

    // Threads creation
    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&s->threads[i], NULL, worker_thread, s) != 0) {
            perror("[!] Error : pthread_create failed in sched_init : ");
            for (int j = 0; j < i; j++) {
                pthread_cancel(s->threads[j]);
            }
            munmap(s->threads, nthreads*sizeof(pthread_t));
            munmap(s->stack, qlen*sizeof(task));
            munmap(s, sizeof(scheduler));
            return -1;
        }
    }

    // Initial task
    sched_spawn(f, closure, s);
    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
    if(s->stack_size >= s->qlen){
        errno = EAGAIN;
        return -1;
    }
    task *newTask = mmap(NULL, sizeof(task), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if((void *) newTask == MAP_FAILED){
        perror("[!] Error mmap failed :" );
        return -1;
    }
    newTask->function = f;
    newTask->arg = closure;
    s->stack[s->stack_size++] = *newTask;
    return 1;
}

void *worker_thread(void *arg){
    write(1,"[+] Worker thread started\n",27);
    return NULL;
}