/* ex4.c - main, produce, consume */
//#include <stdio.h>
//#include <stdlib.h>

void produce(void), consume(void);
int n = 0; /* external variables are shared by all processes */

/*------------------------------------------------------------------------
 * main -- example of unsynchronized producer and consumer processes
 *------------------------------------------------------------------------
 */
void run_ex4(void) {
    resume(create(consume, 1024, 20, "cons", 0));
    resume(create(produce, 1024, 20, "prod", 0));
}

/*------------------------------------------------------------------------
 * produce -- increment n 2000 times and exit
 *------------------------------------------------------------------------
 */
void produce(void) {
    int i;
    for( i=1 ; i<=2000 ; i++ )
        n++;
}

/*------------------------------------------------------------------------
 * consume -- print n 2000 times and exit
 *------------------------------------------------------------------------
 */

void consume(void) {
    int i;
    for( i=1 ; i<=50 ; i++ )
        printf("The value of n is %d \n", n);
}

