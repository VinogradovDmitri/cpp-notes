All information taken from this [cite](https://www.opennet.ru/base/sec/p49-14.txt.html)

---

Depending on the implementation the stack will either grow down (towards
lower memory addresses), or up.  In our examples we'll use a stack that grows
down.  This is the way the stack grows on many computers including the Intel, 
Motorola, SPARC and MIPS processors.  The stack pointer (SP) is also
implementation dependent.  It may point to the last address on the stack, or 
to the next free available address after the stack.  For our discussion we'll
assume it points to the last address on the stack.

 In addition to the stack pointer, which points to the top of the stack
(lowest numerical address), it is often convenient to have a frame pointer
(FP) which points to a fixed location within a frame.  Some texts also refer
to it as a local base pointer (LB).  In principle, local variables could be
referenced by giving their offsets from SP.  However, as words are pushed onto
the stack and popped from the stack, these offsets change.  Although in some
cases the compiler can keep track of the number of words on the stack and
thus correct the offsets, in some cases it cannot, and in all cases
considerable administration is required.  Futhermore, on some machines, such
as Intel-based processors, accessing a variable at a known distance from SP
requires multiple instructions.

   Consequently, many compilers use a second register, FP, for referencing
both local variables and parameters because their distances from FP do
not change with PUSHes and POPs.  On Intel CPUs, BP (EBP) is used for this 
purpose.

The first thing a procedure must do when called is save the previous FP
(so it can be restored at procedure exit).  Then it copies SP into FP to 
create the new FP, and advances SP to reserve space for the local variables. 
This code is called the procedure prolog.  Upon procedure exit, the stack 
must be cleaned up again, something called the procedure epilog.  The Intel 
ENTER and LEAVE instructions, have been provided to do most of the procedure 
prolog and epilog work efficiently. 

