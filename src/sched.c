#include "sched.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>

//ctrl+f XXX pour voir les commentaires/print a supprimer

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
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;   
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

    //Creations des primites de synchronisation
    pthread_mutex_t m;
    int return_value = pthread_mutex_init(&m, NULL);
    if(return_value){
        fprintf(stderr, "[!] Error : pthread_mutex_init failed in shched_init : %s\n", strerror(return_value));
        return -1;
    }
    s->mutex = &m;

    pthread_cond_t c;
    return_value = pthread_cond_init(&c, NULL);
    if(return_value){
        fprintf(stderr, "[!] Error : pthread_cond_init failed in shched_init : %s\n", strerror(return_value));
        return -1;
    }
    s->cond = &c;

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

int sched_spawn(taskfunc f, void *closure, scheduler *s){
    if(s->stack_size >= s->qlen){
        errno = EAGAIN;
        return -1;
    }
    task* newTask = mmap(NULL, sizeof(task), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if((void *) newTask == MAP_FAILED){
        perror("[!] Error mmap failed : " );
        return -1;
    }
    newTask->function = f;
    newTask->arg = closure;
    int return_value = pthread_mutex_lock(s->mutex);
    if (return_value){
        fprintf(stderr, "[!] Erreur de pthread_mutex_lock : %s", strerror(return_value));
        return -1;
    }
    s->stack[s->stack_size++] = *newTask;
    return_value = pthread_mutex_unlock(s->mutex);
    if (return_value){
        fprintf(stderr, "[!] Erreur de pthread_mutex_unlock : %s", strerror(return_value));
        return -1;
    }
    return 1;
}

void* worker_thread(void* arg){
    scheduler* s = (scheduler*) arg;
    int return_value;
    printf("[+] Worker thread n %lu started\n",pthread_self());
    while (1){
        if (s->stack_size > 0){//On regarde si stack_size>0 sans prendre de lock pour optimiser les performances.
            return_value = pthread_mutex_lock(s->mutex);
            if (return_value){
                fprintf(stderr, "[!] Error : pthread_mutex_lock failed in thread n %lu: %s\n",pthread_self() ,strerror(return_value));
                continue;
            }
            if (s->stack_size > 0){//On a un lock, on vérifie que stack_size est toujours supérieur à 0.
                int index = --(s->stack_size);
                task myTask = s->stack[index];
                s->stack[index].function(s->stack[index].arg, s);
            }
            return_value = pthread_mutex_unlock(s->mutex);
            if (return_value){//XXX Là c'est la merde car il faut absolument rendre le lock.
                fprintf(stderr, "XXX [!] Erreur de pthread_mutex_unlock dans le thread n %lu : %s\n",pthread_self() , strerror(return_value));
                fflush(stderr);
            }
        }
        else {
            
        }
    }
    return NULL;
}