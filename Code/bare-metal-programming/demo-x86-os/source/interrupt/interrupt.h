#ifndef __INTERRUPT_H_
#define __INTERRUPT_H_

#include "typedef.h"

/**
 * Initializations for the interrupts:
 *      - Write interrupt descriptors to IDT for each interrupt
 *      - Initialize metadata of IDT
 *      - Initialize the chip 8259a and set the irq number of hardware interrupts
 */
void idt_init(void);


/**
 * Register the handle function for specific interrupt.
 *
 * @param interrupt_no  irq number of interrupt
 * @param handler       entry of handle function
 */
void register_interrupt_handler(uint32_t interrupt_no, void (*handler)());


/**
 * Register the handle function for specific system call.
 *
 * @param handler       entry of handle function
 * @return              the allocated syscall number
 */
int32_t register_syscall_handler(void (*handler)());


/**
 * Enable the master interrupt which managed by chip8259a.
 *
 * @param irq           master irq number(0~7) mapped to 0x20~0x27
 */
void enable_irq_master(int32_t irq);


/**
 * Disable the master interrupt which managed by chip8259a.
 *
 * @param irq           master irq number(0~7) mapped to 0x20~0x27
 */
void disable_irq_master(int32_t irq);


/**
 * Enable the slave interrupt which managed by chip8259a.
 *
 * @param irq           slave irq number(0~7) mapped to 0x28~0x2f
 */
void enable_irq_slave(int32_t irq);


/**
 * Disable the slave interrupt which managed by chip8259a
 *
 * @param irq           slave irq number(0~7) mapped to 0x28~0x2f
 */
void disable_irq_slave(int32_t irq);

#endif
