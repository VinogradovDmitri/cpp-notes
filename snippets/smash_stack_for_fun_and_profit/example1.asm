"function":
        push    rbp
        mov     rbp, rsp
        sub     rsp, 936
        mov     DWORD PTR [rbp-1044], edi
        mov     DWORD PTR [rbp-1048], esi
        mov     DWORD PTR [rbp-1052], edx
        mov     BYTE PTR [rbp-5], 97
        mov     BYTE PTR [rbp-1031], 98
        nop
        leave
        ret
"main":
        push    rbp
        mov     rbp, rsp
        mov     edx, 3
        mov     esi, 2
        mov     edi, 1
        call    "function"
        nop
        pop     rbp
        ret