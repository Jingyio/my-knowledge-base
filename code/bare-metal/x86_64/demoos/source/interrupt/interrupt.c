#include "debug.h"
#include "descriptor.h"
#include "interrupt.h"
#include "string.h"
#include "tty.h"
#include "log.h"

#define INT_M_CTL               (0x20)
#define INT_M_CTLMASK           (0x21)
#define INT_S_CTL               (0xA0)
#define INT_S_CTLMASK           (0xA1)
#define	INT_VECTOR_IRQ0         (0x20)
#define	INT_VECTOR_IRQ8         (0x28)

#define MAX_INTERRUPT_COUNT     (128)
#define MAX_SYSCALL_COUNT       (128)
#define VECTOR_ENTRY_SIZE       (32)
#define IDT_ENTRY_SIZE          (8)

int32_t         k_reenter                                               = -1;
static uint8_t  available_syscall_index                                 = 0;
uint8_t         idt_meta[6]                                             = {0};
static uint8_t  idt[IDT_ENTRY_SIZE * MAX_INTERRUPT_COUNT]               = {0};
void            (*interrupt_handler_table[MAX_INTERRUPT_COUNT])(void)   = {0};
void            (*syscall_handler_table[MAX_SYSCALL_COUNT])(void)       = {0};
void            global_vector_table(void);
void            syscall(void);

/**
 * Exception handle function.
 *
 * *Note:
 *      1. An excpetion occurred
 *      2. Fall into Ring0 and execute the handle function(in .S) according to
 *         the information of idt entry
 *      3. The handle function in step2 calls this handle function(in .c)
 *
 * @param vector_no             irq number of current exception
 */
void exception_handler(int vector_no)
{
        char *err_msg[] = {
                "#DE Divide Error",
                "#DB RESERVED",
                "#NI NMI Interrupt",
                "#BP Breakpoint",
                "#OF Overflow",
                "#BR BOUND Range Exceeded",
                "#UD Invalid Opcode (Undefined Opcode)",
                "#NM Device Not Available (No Math Coprocessor)",
                "#DF Double Fault",
                "#SO Coprocessor Segment Overrun (reserved)",
                "#TS Invalid TSS",
                "#NP Segment Not Present", 
                "#SS Stack-Segment Fault",
                "#GP General Protection",
                "#PF Page Fault",
                "#IR (Intel reserved. Do not use.)",
                "#MF x87 FPU Floating-Point Error (Math Fault)",
                "#AC Alignment Check",
                "#MC Machine Check",
                "#XF SIMD Floating-Point Exception"
	};
        void *handler = syscall_handler_table[g_syscall_tty_write_index];

        ((void (*)(uint8_t*, uint32_t))handler)(err_msg[vector_no], strlen(err_msg[vector_no]));
}

static inline void out_byte(uint16_t port, uint8_t data)
{
        __asm__ __volatile__("outb %0, %1\n\t"::"r"(data), "r"(port):);
}

/**
 *  Initialize chip8259a and set the number of hardware interrupts starting from 0x20.
 *
 */
static void chip8259a_init(void)
{
        out_byte(INT_M_CTL,     0x11);
        out_byte(INT_S_CTL,     0x11);
        out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);
        out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);
        out_byte(INT_M_CTLMASK, 0x04);
        out_byte(INT_S_CTLMASK, 0x02);
        out_byte(INT_M_CTLMASK, 0x01);
        out_byte(INT_S_CTLMASK, 0x01);
        out_byte(INT_M_CTLMASK, 0xff);
        out_byte(INT_S_CTLMASK, 0xff);
}

/**
 * Register the handle function for specific interrupt.
 *
 * @param interrupt_no  irq number of interrupt
 * @param handler       entry of handle function
 */
void register_interrupt_handler(uint32_t interrupt_no, void (*handler)())
{ 
        if (weak_assert(interrupt_no <= MAX_INTERRUPT_COUNT))
                return;

        interrupt_handler_table[interrupt_no] = handler;
}

/**
 * Register the handle function for specific system call.
 *
 * @param handler       entry of handle function
 * @return              the allocated syscall number
 */
int32_t register_syscall_handler(void (*handler)())
{
        available_syscall_index += 1;
        if (weak_assert(available_syscall_index <= MAX_SYSCALL_COUNT))
                return -1;

        syscall_handler_table[available_syscall_index] = handler;
        return available_syscall_index;
}

/**
 * Enable the master interrupt which managed by chip8259a.
 *
 * @param irq           master irq number(0~7) mapped to 0x20~0x27
 */
void enable_irq_master(int irq)
{
        unsigned char mask = ~((unsigned char)(1 << irq));

        __asm__ __volatile__(
                "inb    $0x21,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "andb   %%ah,           %%al    \n\t"
                "outb   %%al,           $0x21   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

/**
 * Disable the master interrupt which managed by chip8259a.
 *
 * @param irq           master irq number(0~7) mapped to 0x20~0x27
 */
void disable_irq_master(int irq)
{
        unsigned char mask = (unsigned char)(1 << irq);
        
        __asm__ __volatile__(
                "inb    $0x21,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "orb    %%ah,           %%al    \n\t"
                "outb   %%al,           $0x21   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

/**
 * Enable the slave interrupt which managed by chip8259a.
 *
 * @param irq           slave irq number(0~7) mapped to 0x28~0x2f
 */
void enable_irq_slave(int irq)
{
        unsigned char mask = ~((unsigned char)(1 << irq));
        
        __asm__ __volatile__(
                "inb    $0xa1,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "andb   %%ah,           %%al    \n\t"
                "outb   %%al,           $0xa1   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

/**
 * Disable the slave interrupt which managed by chip8259a
 *
 * @param irq           slave irq number(0~7) mapped to 0x28~0x2f
 */
void disable_irq_slave(int irq)
{
        unsigned char mask = (unsigned char)(1 << irq);
        
        __asm__ __volatile__(
                "inb    $0xa1,          %%al    \n\t"
                "movb   %0,             %%ah    \n\t"
                "orb    %%ah,           %%al    \n\t"
                "outb   %%al,           $0xa1   \n\t"
                :
                :"g"(mask)
                :"ax"
        );
}

/**
 * Initializations for the interrupts:
 *      - Write interrupt descriptors to IDT for each interrupt
 *      - Initialize metadata of IDT
 *      - Initialize the chip 8259a and set the irq number of hardware interrupts
 */
void idt_init(void)
{
        gate_t gate = {0};
        int i = 0;

        gate.selector     = 0x8;
        gate.present      = 0x1;
        gate.dpl          = 0x3;
        gate.segment_type = 0xe;
        gate.param_count  = 0x0;

        for (i = 0; i < MAX_INTERRUPT_COUNT; i++) {
                gate.handler_entry_offset = (uint32_t)global_vector_table + VECTOR_ENTRY_SIZE * i;
                write_gate(&idt[IDT_ENTRY_SIZE * i], &gate);
        }

        gate.handler_entry_offset = (uint32_t)syscall;
        write_gate(&idt[IDT_ENTRY_SIZE * 100], &gate);

        *(uint16_t *)idt_meta = IDT_ENTRY_SIZE * MAX_INTERRUPT_COUNT;
        *((uint32_t *)&idt_meta[2]) = (uint32_t)&idt;

        chip8259a_init();
}

