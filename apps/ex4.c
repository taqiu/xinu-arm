#include <future.h>

void future_cons1(future);
void future_cons2(future);
void future_prod1(future);
void future_prod2(future);
void fun(int *);
void fun_cont(future);

void run_ex4(void) {
    future f1 , f2, f3, f4;
    int ret = 10;

    // Test for FT_SHARED
    f1 = future_alloc(FT_SHARED);

    resume( create(future_cons1, 1024, 20, "fcons1", 1, f1) );
    resume( create(future_cons1, 1024, 20, "fcons2", 1, f1) );
    resume( create(future_cons1, 1024, 20, "fcons3", 1, f1) );
    resume( create(future_prod1, 1024, 20, "fprod1", 1, f1) );
    
    // Test for FT_QUEUE
    f2 = future_alloc(FT_QUEUE);
    resume( create(future_cons2, 1024, 20, "fcons1", 1, f2) );
    resume( create(future_prod1, 1024, 20, "fprod1", 1, f2) );
    resume( create(future_prod2, 1024, 20, "fprod2", 1, f2) );
    
    // Test for asynch()
    f3 = future_alloc(FT_SHARED);
    asynch(&f3, fun);
    future_get(f3, &ret);
    printf("Check value: %d\n\r", ret);

    // Test for cont()
    f4 = future_alloc(FT_SHARED);
    resume( create(cont, 1024, 20, "cont", 2, &f4, fun_cont));
    //cont(&f4, fun_cont);
    future_set(&f4, &ret);
}

void fun_cont(future fut) {
    printf("future has been set\n\r");
}

void fun(int *val) {
    *val = 20;
    printf("asynch(future*, void *) will run and set the future to %d\n\r", 20);
}

void future_cons1(future fut) {
    int ret;
    future_get(fut, &ret);
    printf("future %d value is %d\n\r", fut, ret);
}

void future_cons2(future fut) {
    int ret, i;
    for (i = 0; i < 2; i ++) {
        future_get(fut, &ret);
        printf("future %d value is %d\n\r", fut, ret);
    }
}

void future_prod1(future fut) {
    int ret = 1;
    future_set(&fut, &ret);
}

void future_prod2(future fut) {
    int ret = 2;
    future_set(&fut, &ret);
}
