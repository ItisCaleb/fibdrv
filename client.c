#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "bn.h"

#define FIB_DEV "/dev/fibonacci"
#define TESTS 100
#define FIB_N 1000

bn *fib_num;

static inline void write_log(FILE *log, int n, u_int64_t ns[])
{
    fprintf(log, "n: %d ns: [", n);
    for (int i = 0; i < TESTS; i++) {
        fprintf(log, "%ld,", ns[i]);
    }
    fprintf(log, "]\n");
}

void test(int fd)
{
    FILE *kernel_ext = fopen("kernel_ext.txt", "w");
    FILE *user_ext = fopen("user_ext.txt", "w");
    FILE *transfer = fopen("transfer_ext.txt", "w");
    for (int i = 0; i <= FIB_N; i++) {
        u_int64_t kernel[TESTS], user[TESTS], trans[TESTS];
        long long sz;
        struct timespec start, end;
        for (int k = 0; k < TESTS; k++) {
            lseek(fd, i, SEEK_SET);

            // get userspace elasped time
            clock_gettime(CLOCK_REALTIME, &start);
            sz = read(fd, fib_num->number, 1);
            clock_gettime(CLOCK_REALTIME, &end);

            // get kernelspace elasped time
            ssize_t kernel_ns = write(fd, NULL, 0);

            kernel[k] = kernel_ns;
            user[k] = end.tv_nsec - start.tv_nsec;
            // kernel to user time
            trans[k] = user[k] - kernel[k];
        }
        write_log(kernel_ext, i, kernel);
        write_log(user_ext, i, user);
        write_log(transfer, i, trans);
        fib_num->size = sz;

        char *num = bn_to_string(fib_num);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%s.\n",
               i, num);
        free(num);
    }
    fclose(kernel_ext);
    fclose(user_ext);
    fclose(transfer);
}

void perf(int fd)
{
    int n = 1000, times = 1;
    lseek(fd, n, SEEK_SET);
    ssize_t sum = 0;
    for (int i = 0; i < times; i++) {
        read(fd, fib_num->number, 1);
        // get kernelspace elasped time
        sum += write(fd, NULL, 0);
    }
    // get userspace elasped time
    char *num = bn_to_string(fib_num);
    printf("Reading from " FIB_DEV
           " at offset %d, returned the sequence "
           "%s.\n",
           n, num);
    printf("average ns: %ld\n", sum / times);
    free(num);
}


int main(int argc, char *argv[])
{
    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }
    fib_num = bn_alloc(100);


    int ch;
    while ((ch = getopt(argc, argv, "p")) != -1) {
        switch (ch) {
        case 'p':
            perf(fd);
            close(fd);
            return 0;
        }
    }
    test(fd);
    close(fd);
    return 0;
}