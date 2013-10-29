#ifndef _FUTURE_H_
#define _FUTURE_H_

#include <stddef.h>
#include <thread.h>

#define NFUT 40
#define FUT_USED 1
#define FUT_FREE 0

#define isbadfut(f) ((int)(f) < 0 || (f) >= NFUT)

int future_initialized = 0;

typedef unsigned int future;
struct futent {
   int future_flag;
   int fstate;
   int value;
   int sem_prod, sem_cons;
};

struct futent futent_tab[NFUT];

future future_alloc(int future_flags);
syscall future_free(future);
syscall future_get(future, int *);
syscall future_set(future, int);
void future_ini();
future future_new();

void future_ini() {
    int i;
    for (i = 0; i < NFUT; i ++) {
        futent_tab[i].fstate = FUT_FREE;
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

    if ((futent_tab[fut].sem_prod = semcreate(0)) == SYSERR || (futent_tab[fut].sem_cons = semcreate(1)) == SYSERR) {
        restore(mask);
        return SYSERR;
    }

    restore(mask);
    return fut;
    
}

syscall future_get(future fut, int *value) {
    irqmask mask;
    mask = disable();
    if (isbadfut(fut)) {
        restore(mask);
        return SYSERR;
    }
    wait(futent_tab[fut].sem_prod);
    *value = futent_tab[fut].value;
    signal(futent_tab[fut].sem_cons);

    restore(mask);
    return OK;
}

syscall future_set(future fut, int value) {
    irqmask mask;
    mask = disable();
    if (isbadfut(fut)) {
        restore(mask);
        return SYSERR;
    }
    wait(futent_tab[fut].sem_cons);
    futent_tab[fut].value = value;
    signal(futent_tab[fut].sem_prod);

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

    if (semfree(futent_tab[fut].sem_prod) == SYSERR || semfree(futent_tab[fut].sem_cons) == SYSERR) {
        restore(mask);
        return SYSERR;
    }
    futent_tab[fut].fstate = FUT_FREE;

    restore(mask);
    return OK;
}

#endif /* _FUTURE_H_ */

