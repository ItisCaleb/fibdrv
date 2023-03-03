#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

void u128tos(__int128_t n, char buf[])
{
    if (n == 0) {
        buf[0] = '0';
        return;
    }
    char str[40] = {0};
    char *s = str + sizeof(str) - 1;  // start at the end
    int len = 0;
    while (n != 0) {
        if (s == str)
            return;  // never happens
        len++;
        *--s = "0123456789"[n % 10];  // save last digit
        n /= 10;                      // drop it
    }
    strncpy(buf, s, len);
}

int main()
{
    long long sz;

    char buf[16];
    char write_buf[] = "testing writing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }

    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, 1);
        __int128_t *r = (__int128_t *) buf;
        char num[40] = {0};
        u128tos(*r, num);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%s.\n",
               i, num);
    }

    /*for (int i = offset; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, 1);
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%lld.\n",
               i, sz);
    }*/

    close(fd);
    return 0;
}
