.globl  global_vector_table
.globl  syscall

.section        .text
.code32

.macro DECLARE_EXCEPTION_NAME irq_no
.align  32,     0x90
        pushl   \irq_no
        jmp     exception_handler
.align  32,     0x90
.endm

.macro DECLARE_INTERRUPT_NAME irq_no
.align  32,     0x90
        pushl   \irq_no
        jmp     interrupt_handler
.align  32,     0x90
.endm

.align 32
global_vector_table:
        DECLARE_EXCEPTION_NAME  $0x00
        DECLARE_EXCEPTION_NAME  $0x01
        DECLARE_EXCEPTION_NAME  $0x02
        DECLARE_EXCEPTION_NAME  $0x03
        DECLARE_EXCEPTION_NAME  $0x04
        DECLARE_EXCEPTION_NAME  $0x05
        DECLARE_EXCEPTION_NAME  $0x06
        DECLARE_EXCEPTION_NAME  $0x07
        DECLARE_EXCEPTION_NAME  $0x08
        DECLARE_EXCEPTION_NAME  $0x09
        DECLARE_EXCEPTION_NAME  $0x0a
        DECLARE_EXCEPTION_NAME  $0x0b
        DECLARE_EXCEPTION_NAME  $0x0c
        DECLARE_EXCEPTION_NAME  $0x0d
        DECLARE_EXCEPTION_NAME  $0x0e
        DECLARE_EXCEPTION_NAME  $0x0f
        DECLARE_EXCEPTION_NAME  $0x10
        DECLARE_EXCEPTION_NAME  $0x11
        DECLARE_EXCEPTION_NAME  $0x12
        DECLARE_EXCEPTION_NAME  $0x13
        DECLARE_EXCEPTION_NAME  $0x14
        DECLARE_EXCEPTION_NAME  $0x15
        DECLARE_EXCEPTION_NAME  $0x16
        DECLARE_EXCEPTION_NAME  $0x17
        DECLARE_EXCEPTION_NAME  $0x18
        DECLARE_EXCEPTION_NAME  $0x19
        DECLARE_EXCEPTION_NAME  $0x1a
        DECLARE_EXCEPTION_NAME  $0x1b
        DECLARE_EXCEPTION_NAME  $0x1c
        DECLARE_EXCEPTION_NAME  $0x1d
        DECLARE_EXCEPTION_NAME  $0x1e
        DECLARE_EXCEPTION_NAME  $0x1f
        DECLARE_INTERRUPT_NAME  $0x20
        DECLARE_INTERRUPT_NAME  $0x21
        DECLARE_INTERRUPT_NAME  $0x22
        DECLARE_INTERRUPT_NAME  $0x23
        DECLARE_INTERRUPT_NAME  $0x24
        DECLARE_INTERRUPT_NAME  $0x25
        DECLARE_INTERRUPT_NAME  $0x26
        DECLARE_INTERRUPT_NAME  $0x27
        DECLARE_INTERRUPT_NAME  $0x28
        DECLARE_INTERRUPT_NAME  $0x29
        DECLARE_INTERRUPT_NAME  $0x2a
        DECLARE_INTERRUPT_NAME  $0x2b
        DECLARE_INTERRUPT_NAME  $0x2c
        DECLARE_INTERRUPT_NAME  $0x2d
        DECLARE_INTERRUPT_NAME  $0x2e
        DECLARE_INTERRUPT_NAME  $0x2f

#
# Handle function for exceptions(IRQ0~IRQ31).
#
# Input  : None
# Output : None
#
.align  32
exception_handler:
        call    exception_handler
        addl    $4,             %esp
        hlt

#
# Handle function for interrupts(IRQ32~IRQ48).
#
# Input  : None
# Output : None
#
.align  32
interrupt_handler:
        pusha
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs

        movl    %esp,           %ebp
        movw    %ss,            %dx
        movw    %dx,            %ds
        movw    %dx,            %es

        movl    $kstack,        %esp

        movl    48(%ebp),       %eax
        call    disable_hw_interrupt

        # Response the interrupt
        movb    $0x20,          %al
        out     %al,            $0x20
        nop
        out     %al,            $0xa0

        incl    k_reenter
        movl    k_reenter,      %eax
        cmp     $0,             %eax
        jne     1f

        sti
        movl    48(%ebp),        %esi
        call    *interrupt_handler_table(, %esi, 4)
        cli

        movb    $0x20,          %al
        outb    %al,            $0x20

1:
        decl    k_reenter

        movl    48(%ebp),       %eax
        call    enable_hw_interrupt

        movl    ptr_proc_run,   %esp
        addl    regs_offset,    %esp

        pop     %gs
        pop     %fs
        pop     %es
        pop     %ds
        popa
        addl    $4,             %esp
        iret

#
# Handle function for syscall whose irq number is 100.
#
# Input  : None
# Output : None
#
.align  32
syscall:
        subl    $4,             %esp
        pusha
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs

        movl    %edx,           %esi
        movw    %ss,            %dx
        movw    %dx,            %ds
        movw    %dx,            %es
        movl    %esi,           %edx

        movl    %esp,           %esi
        movl    $kstack,        %esp

        incl    k_reenter
        cmpl    $0,             k_reenter
        jne     1f

        push    %esi
        push    %edx
        push    %ecx
        push    %ebx
        push    %eax
        call    *syscall_handler_table(, %edi, 4)
        addl    $16,            %esp
        popl    %esi

        movl    %eax,           44(%esi) # return value of syscall handler function
        movl    ptr_proc_run,   %esp
        addl    regs_offset,    %esp
1:
        decl    k_reenter
        popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popa
        addl    $4,             %esp
        iret
 
#
# Diable hardware interrupt based on irq number
#
# Input  : eax, irq number (32~48)
# Output : None
#
disable_hw_interrupt:
        pushl   %eax
        pushl   %ebx
        pushl   %ecx
        pushl   %edx

        movl    %eax,           %edx
        cmpl    $40,            %eax
        jnb     2f
1:
        in      $0x21,          %al     # ignore current interrupt (master)
        movb    %dl,            %cl
        subb    $32,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        or      %bl,            %al
        out     %al,            $0x21
        jmp     3f
2:
        in      $0xa1,          %al     # ignore current interrupt (slave)
        movb    %dl,            %cl
        subb    $40,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        orb     %bl,            %al
        out     %al,            $0xa1
3:
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        ret

#
# Enable hardware interrupt based on irq number
#
# Input  : eax, irq number (32~48)
# Output : None
#
enable_hw_interrupt:
        pushl   %eax
        pushl   %ebx
        pushl   %ecx
        pushl   %edx

        movl    %eax,           %edx
        cmpl    $40,            %eax
        jnb     2f
1:                                      # master
        in      $0x21,          %al
        movb    %dl,            %cl
        subb    $32,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        not     %bl
        and     %bl,            %al
        out     %al,            $0x21
        jmp     3f

2:                                      # slave
        in      $0xa1,          %al
        movb    %dl,            %cl
        sub     $40,            %cl
        movb    $1,             %bl
        shl     %cl,            %bl
        not     %bl
        and     %bl,            %al
        out     %al,            $0xa1
3:
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        ret

