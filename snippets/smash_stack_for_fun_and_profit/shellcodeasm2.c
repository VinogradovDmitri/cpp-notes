// gcc -no-pie -fno-stack-protector -z execstack -o shellcodeasm2 shellcodeasm2.c

int main(void) {
    __asm__ __volatile__(
        "jmp    1f\n\t"                     // 2 байта
        "2:\n\t"
        "popq   %%rdi\n\t"                  // 1 байт   rdi = &"//bin/sh" (pathname)
        "xorq   %%rdx, %%rdx\n\t"           // 3 байта  rdx = 0 (envp)
        "xorq   %%rax, %%rax\n\t"           // 3 байта  rax = 0
        "pushq  %%rdx\n\t"                  // 1 байт   argv[1] = NULL
        "pushq  %%rdi\n\t"                  // 1 байт   argv[0] = &string
        "movq   %%rsp, %%rsi\n\t"           // 3 байта  rsi = argv
        "movb   $0x3b, %%al\n\t"            // 2 байта  rax = 59 (execve)
        "syscall\n\t"                       // 2 байта
        "xorq   %%rax, %%rax\n\t"           // 3 байта  rax = 0
        "movb   $0x3c, %%al\n\t"            // 2 байта  rax = 60 (exit)
        "xorq   %%rdi, %%rdi\n\t"           // 3 байта  rdi = 0
        "syscall\n\t"                       // 2 байта
        "1:\n\t"
        "call   2b\n\t"                     // 5 байт
        ".string \"/bin/sh\"\n\t"           // 8 байт (включая \0)
        :
        :
        : "rax", "rdi", "rsi", "rdx", "memory"
    );
    return 0;
}