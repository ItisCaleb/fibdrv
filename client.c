#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define TESTS 200
#define FIB_N 1000

static char *bn_to_string(const int *num, ssize_t size)
{
    // log10(x) = log2(x) / log2(10) ~= log2(x) / 3.322
    size_t len = (8 * sizeof(int) * size) / 3 + 2;
    char *s = malloc(len);
    char *p = s;

    memset(s, '0', len - 1);
    s[len - 1] = '\0';

    for (int i = size - 1; i >= 0; i--) {
        for (unsigned int d = 1U << 31; d; d >>= 1) {
            /* binary -> decimal string */
            int carry = ((d & num[i]) != 0);
            for (int j = len - 2; j >= 0; j--) {
                s[j] += s[j] - '0' + carry;  // double it
                carry = (s[j] > '9');
                if (carry)
                    s[j] -= 10;
            }
        }
    }
    // skip leading zero
    while (p[0] == '0' && p[1] != '\0') {
        p++;
    }
    memmove(s, p, strlen(p) + 1);
    return s;
}

static inline void write_log(FILE *log, int n, u_int64_t ns[])
{
    fprintf(log, "n: %d ns: [", n);
    for (int i = 0; i < TESTS; i++) {
        fprintf(log, "%ld,", ns[i]);
    }
    fprintf(log, "]\n");
}



int main()
{
    // long long sz;
    int buf[1000];
    FILE *kernel_ext = fopen("kernel_ext.txt", "w");
    FILE *user_ext = fopen("user_ext.txt", "w");
    FILE *transfer = fopen("transfer_ext.txt", "w");
    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    /*for (int i = 0; i <= offset; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }*/

    for (int i = 0; i <= FIB_N; i++) {
        u_int64_t kernel[TESTS], user[TESTS], trans[TESTS];
        long long sz;
        struct timespec start, end;
        for (int k = 0; k < TESTS; k++) {
            lseek(fd, i, SEEK_SET);

            // get userspace elasped time
            clock_gettime(CLOCK_REALTIME, &start);
            sz = read(fd, buf, 1);
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


        char *num = bn_to_string(buf, sz);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%s.\n",
               i, num);
        free(num);
    }

    /*for (int i = offset; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, 1);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%lld.\n",
               i, sz);
    }*/
    fclose(kernel_ext);
    fclose(user_ext);
    fclose(transfer);

    close(fd);
    return 0;
}
