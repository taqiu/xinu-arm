#include <future.h>

void future_cons(future);
void future_prod(future);

void run_ex4(void) {
    future f1, f2, f3;
    f1 = future_alloc(0);
    f2 = future_alloc(0);
    f3 = future_alloc(0);

    resume( create(future_cons, 1024, 20, "fcons1", 1, f1) );
    resume( create(future_prod, 1024, 20, "fprod1", 1, f1) );
    resume( create(future_cons, 1024, 20, "fcons2", 1, f2) );
    resume( create(future_prod, 1024, 20, "fprod2", 1, f2) );
    resume( create(future_cons, 1024, 20, "fcons3", 1, f3) );
    resume( create(future_prod, 1024, 20, "fprod3", 1, f3) );
}

void future_cons(future fut) {
    int ret;
    int i;
    for (i = 0; i < 3; i++) {
    	future_get(fut, &ret);
    	printf("future %d value is %d\n\r", fut, ret);
    }
}

void future_prod(future fut) {
    int i;
    for (i = 0; i < 3; i++)
        future_set(fut, i);
}
