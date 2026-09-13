// gcc -g -O0 -fno-stack-protector -no-pie -z execstack example3.c -o res

#include <stdio.h>
#include <stdint.h>

void function(int a, int b, int c) {
    volatile char buffer1[16];
    volatile char buffer2[10];

    for (int i = 0; i < 10; ++i)
        buffer2[i] = 'a';

    // Смещение от buffer1 до адреса возврата = 40 (ниже пояснение)
    uintptr_t *ret = (uintptr_t *)(buffer1 + 40);

    // Пропускаем инструкцию x = 1 (7 байт)
    // objdump -d res | grep -A 30 '<main>:'
    // c7 45 fc 01 00 00 00    movl   $0x1,-0x4(%rbp)
    *ret += 7;
}

int main(void) {
    int x = 0;

    function(1, 2, 3);

    x = 1;                     // эту строку хотим пропустить
    printf("x = %d\n", x);     // должно напечатать x = 0
    return 0;
}
// $ objdump -D res | grep -A 20 '<function>:'
// 0000000000401126 <function>:
//   401126:	55                   	push   %rbp
//   401127:	48 89 e5             	mov    %rsp,%rbp
//   40112a:	89 7d cc             	mov    %edi,-0x34(%rbp)
//   40112d:	89 75 c8             	mov    %esi,-0x38(%rbp)
//   401130:	89 55 c4             	mov    %edx,-0x3c(%rbp)
//   401133:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
//   40113a:	eb 0e                	jmp    40114a <function+0x24>
//   40113c:	8b 45 fc             	mov    -0x4(%rbp),%eax
//   40113f:	48 98                	cltq
//   401141:	c6 44 05 d6 61       	movb   $0x61,-0x2a(%rbp,%rax,1)
//   401146:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
//   40114a:	83 7d fc 09          	cmpl   $0x9,-0x4(%rbp)
//   40114e:	7e ec                	jle    40113c <function+0x16>
//   401150:	48 8d 45 e0          	lea    -0x20(%rbp),%rax
//   401154:	48 83 c0 28          	add    $0x28,%rax
//   401158:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
//   40115c:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
//   401160:	48 8b 00             	mov    (%rax),%rax
//   401163:	48 8d 50 07          	lea    0x7(%rax),%rdx
//   401167:	48 8b 45 f0          	mov    -0x10(%rbp),%rax

// Addr (grow down)           Что находится в памяти       Регистры / RSP
// ====================================================================
// [top addr]
//                             (аргументы вызывающего — RA optimization
//                             (first 6 integer[целочисленный]): rdi, rsi, rdx, rcx, r8, r9)
//@ +0x08 (%rbp + 8)          Адрес возврата (Return Addr)  <-- цель эксплойта
//# +0x00 (%rbp)              Старый RBP (saved RBP)        <-- RBP == RSP
// ============================ ГРАНИЦА RED ZONE ======================
//                             Red zone = [RBP-128, RBP) — 128 байт
//                             Всё, что ниже, живёт здесь. RSP не двигается.
// --------------------------------------------------------------------
//# -0x04 (%rbp - 4)          int i              [4 байта]  <-- счётчик цикла
//$ -0x08 (%rbp - 8)          padding            [4 байта]
//# -0x10 (%rbp - 16)         uintptr_t *ret     [8 байт]   <-- указатель
// --------------------------------------------------------------------
//# -0x20 (%rbp - 32)         buffer1[16]        [16 байт]  <-- начало
//#                           (rbp-0x20 .. rbp-0x10)
// --------------------------------------------------------------------
//# -0x2a (%rbp - 42)         buffer2[10]        [10 байт]  <-- начало
//#                           (rbp-0x2a .. rbp-0x20)        <-- вплотную под buffer1
//$ -0x30 (%rbp - 48)         padding            [6 байт]
// --------------------------------------------------------------------
//# -0x34 (%rbp - 52)         int a              [4 байта] \
//# -0x38 (%rbp - 56)         int b              [4 байта]  | -g (spill из регистров)
//# -0x3c (%rbp - 60)         int c              [4 байта] /
// [low addr]
// --------------------------------------------------------------------
//                             Red zone продолжается до rbp-128
//                             (rbp-0x40 .. rbp-0x80 — не используется)
// ====================================================================