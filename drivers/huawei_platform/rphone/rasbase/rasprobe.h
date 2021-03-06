#ifndef __RAS_RPOBE__
#define __RAS_PROBE__

#include <linux/kprobes.h> 
#include <asm/ptrace.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#define rasprobe_instance kretprobe_instance
#define rasprobe kretprobe



//////////////////////////////
struct RasRegs {
	void* args[4];
};

#define rasprobe_name(fun)    rasprobe_##fun##_kretname
#define rasprobe_handler(fun) rasprobe_##fun##_handler

#define rasprobe_init(fun) { \
	.kp.symbol_name = #fun, \
	.handler = rasprobe_handler(fun), \
	.entry_handler = rasprobe_entry, \
	.data_size = sizeof(struct RasRegs), \
}

#define rasprobe_define(fun) \
static struct rasprobe rasprobe_name(fun) = rasprobe_init(fun)

int rasprobe_entry(struct rasprobe_instance *ri, struct pt_regs *regs);
void rasprobe_seturn(struct pt_regs *regs, long val);

int register_rasprobes(struct rasprobe **rps, int num);
void unregister_rasprobes(struct rasprobe **rps, int num);

#endif

