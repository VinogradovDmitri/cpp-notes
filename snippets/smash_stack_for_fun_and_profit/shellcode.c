#include <unistd.h>

int main(void) {
    char *argv[] = { "/bin/sh", NULL };
    execve("/bin/sh", argv, NULL);
    return 0;
}