/* ex4.c - main, produce, consume */
/* we don't have to include any libs, because this program 
 * is just a part of shell. Shell run this program as a
 * function
 */
/* Add semaphore 
 * Group members: Tianhao Cao, Tanghong Qiu and Tzu-Feng Wu
 * */
// #include <stdio.h>
// #include <stdlib.h>
 

void produce(int consumed, int produced), consume(int consumed, int produced);
int n = 0; /* external variables are shared by all processes */

/*------------------------------------------------------------------------
 * main -- example of unsynchronized producer and consumer processes
 *------------------------------------------------------------------------
 */
void run_ex4(void) {
    n = 0;
    int produced, consumed;
    consumed = semcreate(0);
    produced = semcreate(1);

    resume(create(consume, 1024, 20, "cons", 2, consumed, produced));
    resume(create(produce, 1024, 20, "prod", 2, consumed, produced));
}

/*------------------------------------------------------------------------
 * produce -- increment n 2000 times and exit
 *------------------------------------------------------------------------
 */
void produce(int consumed, int produced) {
    int i;
    for( i=1 ; i<= 2000 ; i++ ) {
        wait(consumed);
        n++;
        signal(produced);
    }
}

/*------------------------------------------------------------------------
 * consume -- print n 60 times and exit
 *------------------------------------------------------------------------
 */

void consume(int consumed, int produced) {
    int i;
    for( i=1 ; i<= 2000 ; i++ ) {
        wait(produced);
        printf("n is %d\r\n", n);
        signal(consumed);
    }
}

