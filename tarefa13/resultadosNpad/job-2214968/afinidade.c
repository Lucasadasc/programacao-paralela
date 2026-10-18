#define _GNU_SOURCE
#include <omp.h>
#include <sched.h>
#include <stdio.h>

/* Diagnostico separado: nao interfere no tempo do solver.
   A CPU observada e instantanea; a mascara informa onde a thread pode executar. */
int main(void) {
    #pragma omp parallel
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        int ok = sched_getaffinity(0, sizeof(mask), &mask);
        #pragma omp critical
        {
            printf("thread=%d equipe=%d cpu=%d bind=%d cpus=",
                   omp_get_thread_num(), omp_get_num_threads(), sched_getcpu(),
                   (int)omp_get_proc_bind());
            if (ok) printf("indisponivel");
            else for (int c = 0; c < CPU_SETSIZE; c++)
                if (CPU_ISSET(c, &mask)) printf("%d,", c);
            putchar('\n');
        }
    }
    return 0;
}
