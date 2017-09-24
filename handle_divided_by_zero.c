#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ptrace.h>

asm("   .text");
asm("   .type my_divide_zero,@function");
asm("my_divide_zero:");
asm("   incq (%rsp)");
asm("   iretq");

static inline void pack_gate(gate_desc *gate, unsigned type, unsigned long func,
    unsigned dpl, unsigned ist, unsigned seg){
        gate->offset_low    = PTR_LOW(func);
        gate->segment       = __KERNEL_CS;
        gate->ist       = ist;
        gate->p         = 1;
        gate->dpl       = dpl;
        gate->zero0     = 0;
        gate->zero1     = 0;
        gate->type      = type;
        gate->offset_middle = PTR_MIDDLE(func);
        gate->offset_high   = PTR_HIGH(func);
}

static unsigned long new_idt_table_page;
static struct desc_ptr default_idtr;

static void my_load_idt(void *info){
    struct desc_ptr *idtr_ptr = (struct desc_ptr *)info;
    load_idt(idtr_ptr);
}

asmlinkage void my_divide_zero(void);

static int __init handle_divided_by_zero_init(void)
{
    printk(KERN_ALERT "handle_divided_by_zero_init\n");

    struct desc_ptr idtr;
    gate_desc *old_idt, *new_idt;

    store_idt(&default_idtr);
    old_idt = (gate_desc *)default_idtr.address;

    new_idt_table_page = __get_free_page(GFP_KERNEL);
    printk(KERN_ALERT "handle_divided_by_zero_init allocate\n");
    if(!new_idt_table_page)
        return -ENOMEM;
    
    idtr.address = new_idt_table_page;
    idtr.size = default_idtr.size;

    new_idt = (gate_desc *)idtr.address;
    memcpy(new_idt, old_idt, idtr.size);
    pack_gate(&new_idt[0], GATE_INTERRUPT, (unsigned long)my_divide_zero, 0, 0, __KERNEL_CS);

    printk(KERN_ALERT "handle_divided_by_zero_init load idt\n");
    load_idt(&idtr);
    printk(KERN_ALERT "handle_divided_by_zero_init loaded idt\n");
    smp_call_function(my_load_idt, (void *)&idtr, 1);
    printk(KERN_ALERT "handle_divided_by_zero_init all loaded idt\n");

    return 0;
}

static void __exit handle_divided_by_zero_exit(void)
{
    struct desc_ptr idtr;
    store_idt(&idtr);

    if(idtr.address != default_idtr.address || idtr.size != default_idtr.size){
        printk(KERN_ALERT "handle_divided_by_zero_exit unload\n");
        load_idt(&default_idtr);
        printk(KERN_ALERT "handle_divided_by_zero_exit unloaded\n");
        smp_call_function(my_load_idt, (void *)&default_idtr, 1);
        printk(KERN_ALERT "handle_divided_by_zero_exit all unloaded\n");
        free_page(new_idt_table_page);
    }

    printk(KERN_ALERT "handle_divided_by_zero_exit\n");
}

module_init(handle_divided_by_zero_init);
module_exit(handle_divided_by_zero_exit);