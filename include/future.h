#ifndef _FUTURE_H_
#define _FUTURE_H_

#include <stddef.h>
#include <thread.h>
#include <queue.h>
#include <thread.h>

#define FUT_USED 1
#define FUT_FREE 0
#define FT_NULL 0
#define FT_SHARED 1
#define FT_QUEUE 2

#define FUT_INIED 1
#define FUT_UNINI 0

#define NFUT 40
#define VAL_QUE_LEN 100

#define isbadfut(f) ((int)(f) < 0 || (f) >= NFUT)

int future_initialized = 0;

typedef unsigned int future;
struct futent {
   int future_flag;
   int flag;
   int fstate;
   int value;
   int sem_prod, sem_cons;
};

struct futent futent_tab[NFUT];

int value_que[VAL_QUE_LEN];
int head, tail;

future future_alloc(int future_flags);
syscall future_free(future);
syscall future_get(future, int *);
syscall future_set(future *, int *);
void future_ini();
future future_new();

void future_ini() {
    int i;
    tail = 0;
    head = 0;
    for (i = 0; i < NFUT; i ++) {
        futent_tab[i].fstate = FUT_FREE;
        futent_tab[i].future_flag = FT_NULL;
    }
}

future future_new() {
    int i;
    static future nextfut = 0;
    future fut;
    if (future_initialized == 0) {
        future_ini();
        future_initialized == 1;
    }

    for (i = 0; i < NFUT; i ++) {
        fut = nextfut ++;
        if (nextfut >= NFUT) nextfut = 0;
        if (futent_tab[fut].fstate == FUT_FREE) {
                futent_tab[fut].fstate = FUT_USED;
                futent_tab[fut].flag = FUT_UNINI;
                return fut;
        }
    }

    return SYSERR;
}

future future_alloc(int future_flags) {
    irqmask mask;
    mask = disable();
    future fut;

    if ((fut = future_new()) == SYSERR) {
        restore(mask);
        return SYSERR;
    }

    struct futent *futptr = &futent_tab[fut];
    futptr->future_flag = future_flags;

    if (future_flags == FT_SHARED) {
        if ((futptr->sem_prod = semcreate(0)) == SYSERR) {
            restore(mask);
            return SYSERR;
        }
    } else if (future_flags == FT_QUEUE) {
        if ((futptr->sem_prod = semcreate(0)) == SYSERR) {
            restore(mask);
            return SYSERR;
        }
    } else {
        if ((futent_tab[fut].sem_prod = semcreate(0)) == SYSERR || (futent_tab[fut].sem_cons = semcreate(1)) == SYSERR) {
            restore(mask);
            return SYSERR;
        }
    }

    restore(mask);
    return fut;
}

syscall future_get(future fut, int *value) {
    irqmask mask;
    register struct thrent *thrptr;
    register struct sement *semptr;

    mask = disable();
    if (isbadfut(fut)) {
        restore(mask);
        return SYSERR;
    }

    struct futent* futptr = &futent_tab[fut];

    if (futptr->future_flag == FT_SHARED) {
        if (futptr->flag == FUT_INIED) {
            *value = futent_tab[fut].value;
        } else if (futptr->flag == FUT_UNINI) {
            thrptr = &thrtab[thrcurrent];
            semptr = &semtab[futptr->sem_prod];
            thrptr->state = THRWAIT;
            thrptr->sem = futptr->sem_prod;
            enqueue(thrcurrent, semptr->queue);
            resched();
            *value = futent_tab[fut].value;
        }
    } else if (futptr->future_flag == FT_QUEUE) {
         wait(futptr->sem_prod);
         if (tail != head) {
             *value = value_que[head];
             head = (head + 1) % VAL_QUE_LEN;
         } else wait(futptr->sem_prod);
    } else {
        wait(futent_tab[fut].sem_prod);
        *value = futent_tab[fut].value;
        signal(futent_tab[fut].sem_cons);
    }
    restore(mask);
    return OK;
}

syscall future_set(future *future, int *value) {
    register struct thrent *thrptr;
    register struct sement *semptr;
    int fut = *future;
    irqmask mask;
    mask = disable();
    if (isbadfut(fut)) {
        restore(mask);
        return SYSERR;
    }

    struct futent *futptr = &futent_tab[fut];

    if (futptr->future_flag == FT_SHARED) {
        if (futptr->flag == FUT_UNINI) {
            semptr = &semtab[futptr->sem_prod];
            futptr->value = *value;
            futptr->flag = FUT_INIED;
            while(nonempty(semptr->queue)) {
                ready(dequeue(semptr->queue), RESCHED_YES);
            }
        } else if (futptr->flag == FUT_INIED) {
            futptr->value = *value;
        }
    } else if (futptr->future_flag == FT_QUEUE) {
        if ((tail + 1) % VAL_QUE_LEN != head) {
            value_que[tail] = *value;
            tail = (tail + 1) % VAL_QUE_LEN;
        } else return SYSERR;
        signal(futptr->sem_prod);
    } else {
        wait(futent_tab[fut].sem_cons);
        futent_tab[fut].value = *value;
        signal(futent_tab[fut].sem_prod);
    }

    restore(mask);
    return OK;
}

syscall future_free(future fut) {
    irqmask mask;
    mask = disable();
    if (isbadfut(fut)) {
        restore(mask);
        return SYSERR;
    }

    struct futent *futptr = &futent_tab[fut];

    if (futptr->future_flag == FT_SHARED) {
    } else if (futptr->future_flag == FT_QUEUE) {
    } else {
        if (semfree(futent_tab[fut].sem_prod) == SYSERR || semfree(futent_tab[fut].sem_cons) == SYSERR) {
            restore(mask);
            return SYSERR;
        }
    }
    futptr->fstate = FUT_FREE;
    futptr->flag = FUT_UNINI;

    restore(mask);
    return OK;
}


syscall asynch(future *fut, void *fun) {
    irqmask mask;
    int val;
    mask = disable();
    if (isbadfut(*fut)) {
        restore(mask);
        return SYSERR;
    }
    resume( create(fun, 1024, 20, "future_asynch", 1, &val));
    future_set(fut, &val);
    restore(mask);
    return OK;
}

syscall cont(future *fut, void *fun) {
    irqmask mask;
    mask = disable();
    if (isbadfut(*fut)) {
        restore(mask);
        return SYSERR;
    }
    int ret;
    if (future_get(*fut, &ret) == OK)
        resume( create(fun, 1024, 20, "future_cont", 1, *fut));
    else return SYSERR;
    restore(mask);
    return OK;
}

#endif /* _FUTURE_H_ */

