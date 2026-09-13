// $ gcc -march=x86-64 -S -nostdlib -o example1.s example1.c 

void function(int a, int b, int c) {
    char buff1[5];
    char buff2[1024];
    buff1[0] = 'a';
    buff2[9] = 'b';
}

void main() {
    function(1, 2, 3);
}

// bottom of                                                            top of
// memory                                                               memory
//            buffer2       buffer1   sfp   ret   a     b     c
// <------   [            ][        ][    ][    ][    ][    ][    ]
	   
// top of                                                            bottom of
// stack                                                                 stack