// gcc -g -O0 -fno-stack-protector -no-pie -z execstack SeeStack.c.c -o res

#include <stdio.h>
#include <stdint.h>

void function(int a, int b, int c) {
    volatile char buffer1[16];
    volatile char buffer2[32];
    // На всякий случай чтобы компилятор не выкинул буфферы
    for (int i = 0; i < 16; i++) buffer1[i] = 'A';
    for (int i = 0; i < 32; i++) buffer2[i] = 'B';

    // Печатаем реальные адреса
    printf("&buffer1 = %p\n", (void*)buffer1);
    printf("&buffer2 = %p\n", (void*)buffer2);

    uintptr_t rbp;
    __asm__ volatile("mov %%rbp, %0" : "=r"(rbp));
    printf("rbp      = 0x%lx\n", (unsigned long)rbp);
    printf("rbp+8    = 0x%lx  (адрес возврата)\n", (unsigned long)(rbp + 8));
    printf("ret value= 0x%lx\n", *(unsigned long*)(rbp + 8));
    printf("offset buffer1 -> ret = %ld\n",
           (long)((char*)(rbp + 8) - (char*)buffer1));
}

int main(void) {
    int x = 0;
    function(1, 2, 3);
    x = 1;
    printf("x = %d\n", x);
    return 0;
}