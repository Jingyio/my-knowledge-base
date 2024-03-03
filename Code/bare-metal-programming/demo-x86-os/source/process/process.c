#include "debug.h"
#include "interrupt.h"
#include "kernel.h"
#include "memory.h"
#include "process.h"
#include "string.h"
#include "kheap.h"

#define MEMBER_OFFSET_OF(t, m) ((long)&(((t *)0)->m))

extern uint32_t sel_sysdata;
extern uint8_t kstack;  // Set uint8_t but not void to fix build warning 
volatile hw_tss_t tss = {0};

static process_t process_table[MAX_PROCESS] = {0};
process_t *ptr_proc_run = 0;
const uint32_t regs_offset = MEMBER_OFFSET_OF(process_t, regs);
const uint32_t regs_end_offset = MEMBER_OFFSET_OF(process_t, regs) + sizeof(stack_frame_t);
const uint32_t selector_ldt_offset = MEMBER_OFFSET_OF(process_t, selector_ldt);

// Purpose:
//   Initialize SS0 and ESP0 fields in TSS descriptor and load TSS table. When an exception or
//   interrupt occurs and enter Ring0, the value of the current process register will be saved.
//   Therefore, ESP0 needs to point to the top of the stack_frame structure.
//
// Input:
//   void
//
// Output:
//   The selector of TSS descriptor.
static uint32_t tss_init(void)
{
        uint16_t selector_tss = 0;
        desc_t tss_desc = {
                .base_address           = (unsigned int)&tss,
                .segment_limit          = sizeof(struct hw_tss),
                .segment_type           = SEGTYPE_386TSS,
                .descriptor_type        = DESC_TYPE_SYS,
                .dpl                    = 0,
                .present                = PRESENT_IN_MEMORY,
        };
        void *ptr_desc_entry = 0;

        tss.ss0 = sel_sysdata;
        tss.esp0 = (uint32_t)&kstack;
        tss.iobase = sizeof(hw_tss_t);

        ptr_desc_entry = get_available_desc();
        write_desc(ptr_desc_entry, &tss_desc);
        selector_tss = get_desc_selector(ptr_desc_entry);

        __asm__ __volatile__(
                "ltr    %0\n\t"
                ::"a"(selector_tss):
        );

        return selector_tss;
}

static process_t *get_available_process_struct(void)
{
        int32_t i = 0, j = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (!process_table[i].process_status) {
                        for (j = 0; j < sizeof(process_t); j++)
                                *((uint8_t *)&process_table[i] + j) = 0;
                        return &process_table[i];
                }
        }

        return 0;
}

int32_t create_process(proc_info_t *ptr_proc_info)
{
        desc_t ldt_desc = {0};
        void *ptr_gdt_desc = get_available_desc();
        process_t *ptr_proc_curr = get_available_process_struct();
        process_t *ptr_proc_prev = 0;
        uint8_t *process_stack = 0;
        uint32_t i = 0;

        if (weak_assert(ptr_proc_curr) || weak_assert(ptr_gdt_desc) || weak_assert(ptr_proc_info))
                return -1;
        process_stack = kmalloc(ptr_proc_info->stack_size);
        if (!process_stack)
                return -1;

        if (ptr_proc_info->priviledge < 0 || ptr_proc_info->priviledge > 3)
                ptr_proc_info->priviledge = 3;

        ldt_desc.base_address    = (uint32_t)ptr_proc_curr->ldts;
        ldt_desc.segment_limit   = 2 * 8;
        ldt_desc.segment_type    = 0x2;
        ldt_desc.descriptor_type = 0;
        ldt_desc.dpl             = 0;
        ldt_desc.present         = 1;
        write_desc(ptr_gdt_desc, &ldt_desc);

        memcpy(&ptr_proc_curr->proc_info, ptr_proc_info, sizeof(proc_info_t));
        ptr_proc_curr->regs.gs = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.fs = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.es = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.ds = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.ss = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.cs = 0x0 | 4 | ptr_proc_info->priviledge;

        ptr_proc_curr->regs.eip = (uint32_t)ptr_proc_info->f_entry;
        ptr_proc_curr->regs.esp = (uint32_t)process_stack + ptr_proc_info->stack_size;
        ptr_proc_curr->regs.eflags = 0x1202;

        ptr_proc_curr->selector_ldt = get_desc_selector(ptr_gdt_desc);

        *((uint32_t *)ptr_proc_curr->ldts + 0) = 0x0000ffff;
        *((uint32_t *)ptr_proc_curr->ldts + 1) = 0x00cf9a00 | ((ptr_proc_info->priviledge & 0x3) << 13);
        *((uint32_t *)ptr_proc_curr->ldts + 2) = 0x0000ffff;
        *((uint32_t *)ptr_proc_curr->ldts + 3) = 0x00cf9200 | ((ptr_proc_info->priviledge & 0x3) << 13);

        switch (ptr_proc_info->priviledge) {
        case 0:
        case 1:
        case 2:
                ptr_proc_curr->schedule_info.time_slice = 50;
                ptr_proc_curr->schedule_info.default_ts = 50;
                break;
        case 3:
                ptr_proc_curr->schedule_info.time_slice = 10;
                ptr_proc_curr->schedule_info.default_ts = 10;
                break;
        default:
                ptr_proc_curr->schedule_info.time_slice = 10;
                ptr_proc_curr->schedule_info.default_ts = 10;
                break;
        } 

        if (process_table[0].process_status) {
                ptr_proc_prev = process_table;
                while (ptr_proc_prev->next != process_table)
                        ptr_proc_prev = ptr_proc_prev->next;
                ptr_proc_curr->next = ptr_proc_prev->next;
                ptr_proc_prev->next = ptr_proc_curr;
                ptr_proc_curr->process_status = PS_READY_TO_RUN;
        } else {
                ptr_proc_curr->next = process_table;
                ptr_proc_curr->process_status = PS_READY_TO_RUN;
                ptr_proc_run = ptr_proc_curr;
        }
        ptr_proc_curr->pid = (ptr_proc_curr - process_table);

        for (i = 0; i < MAX_NAME_LEN; i++)
                ptr_proc_curr->name[i] = ptr_proc_info->name[i];

        return 0;
}

int32_t get_current_pid(void)
{
        if (ptr_proc_run != 0)
                return ptr_proc_run->pid;
        else
                return -1;
}

static process_t *pid2proc(uint32_t pid)
{
        process_t *current = process_table;

        while (current) {
                if (pid == current->pid)
                        break;
                else
                        current = current->next;
        }

        return current;
}

static int32_t get_pid_by_name(uint8_t *name)
{
        uint32_t i = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (strcmp(name, process_table[i].name, MAX_NAME_LEN))
                        return i;
        }

        return -1;
}

// Only for single process
void enter_critical_area(void)
{
        int32_t i = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (process_table[i].process_status == PS_NULL)
                        continue;
                if (ptr_proc_run != (process_table + i)) {
                        process_table[i].process_previous_status = process_table[i].process_status;
                        process_table[i].process_status = PS_PENDING;
                }
        }
}

void exit_critical_area(void)
{
        int32_t i = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (process_table[i].process_status == PS_NULL)
                        continue;
                if (ptr_proc_run != (process_table + i)) {
                        process_table[i].process_status = process_table[i].process_previous_status;
                        process_table[i].process_previous_status = -1;
                }
        }        
}

void unblock(uint32_t pid)
{
        process_t *ptr_proc = pid2proc(pid);

        ptr_proc->process_status = PS_READY_TO_RUN;
}

void block(uint32_t pid)
{
        process_t *ptr_proc = pid2proc(pid);
        
        if (weak_assert(ptr_proc->next))
                return;
        

        ptr_proc->process_status = PS_PENDING;

        while (ptr_proc->next) {
                ptr_proc = ptr_proc->next;
                if (ptr_proc->process_status == PS_READY_TO_RUN)
                        break;
        }

        ptr_proc_run = ptr_proc;
        ptr_proc_run->schedule_info.time_slice = ptr_proc_run->schedule_info.default_ts;
        tss.esp0 = (uint32_t)&ptr_proc_run->regs + sizeof(stack_frame_t);
        __asm__ __volatile__(
                "movl   %0,     %%edi   \n\t"
                "lldt   (%%edi)         \n\t"
                ::"a"((uint8_t *)ptr_proc_run + selector_ldt_offset):"edi"
        );
}

static void process_schedule(void)
{
        ptr_proc_run->schedule_info.time_slice -= 1;
        if (ptr_proc_run->schedule_info.time_slice < 0) {
                ptr_proc_run->schedule_info.time_slice = ptr_proc_run->schedule_info.default_ts;

                while (ptr_proc_run->next) {
                        ptr_proc_run = ptr_proc_run->next;
                        if (ptr_proc_run->process_status == PS_READY_TO_RUN)
                                break;
                }
                tss.esp0 = (uint32_t)&ptr_proc_run->regs + sizeof(stack_frame_t);

                __asm__ __volatile__(
                        "movl   %0,     %%edi   \n\t"
                        "lldt   (%%edi)         \n\t"
                        ::"a"((uint8_t *)ptr_proc_run + selector_ldt_offset):"edi"
                );
        }

}

void process_pre_init(void)
{
        int32_t i = 0;

        tss_init();
        
        for (i = 0; i < MAX_PROCESS * sizeof(process_t); i++)
                *((uint8_t *)process_table + i) = 0;

        // Mapping: irq 0x20 - master 0, Timer Interrupt
        register_interrupt_handler(0x20, process_schedule);
        enable_irq_master(0);
}

