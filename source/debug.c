#include <3ds.h>
#include <stdio.h>
#include "dma.h"
#include "proc.h"
#include "debug.h"
#include "kernel/kproc.h"

void debug_enable()
{
	scenic_kproc *p = kproc_find_by_id((u32)-1);
	u32 flags;
	kproc_get_flags(p, &flags);
	flags |= 1 << 1; // Force debug.
	kproc_set_flags(p, flags);
	kproc_close(p);
}

int debug_freeze(scenic_process *p)
{
	if(!p) { return -1; }
	if(p->debug == -1)
	{
		proc_debug(p);
	}

	Result r;
	if((r = svcBreakDebugProcess(p->debug)) != 0)
	{
		printf("svcBreakDebugProcess failed with %08lx\n", (u32)r);
		return -1;
	}

	p->frozen = true;

	return 0;
}

int debug_resume(scenic_process *p)
{
	if(!p) { return -1; }
	if(p->debug == -1 || !p->frozen) { return 0; } // process is already running if we aren't debugging it...

	debug_sink_events(p);
	p->frozen = false;
	return 0;
}

void debug_sink_events(scenic_process *p)
{
	DebugEventInfo info;
	Result r;

	while(true)
	{
		if((r = svcGetProcessDebugEvent(&info, p->debug)) != 0)
		{
			if(r == 0xd8402009)
			{
				// Would block! we are done.
				printf("Returning...\n");
				break;
			}
			printf("svcGetProcessDebugEvent failed with %08lx.\n", (u32)r);
		}
		else
		{
			// todo: better handling of debug events..
			printf("got debug event of type %x\n", info.type);
			if(info.type == 4)
			{
				printf("got exception of type %x\n", info.exception.type);
			}
		}
		svcContinueDebugEvent(p->debug, 3);
	}
}

int debug_get_thread_ctx(scenic_thread *t, ThreadContext *ctx)
{
	if(!t || !ctx) { return -1; }
	Result r = svcGetDebugThreadContext(ctx, t->proc->debug, t->tid, 3); // get all 16 regs
	// todo: floating point registers.
	if(r < 0)
	{
		return -1;
	}
	return 0;
}

int debug_set_thread_ctx(scenic_thread *t, ThreadContext *ctx)
{
	if(!t || !ctx) { return -1; }
	Result r = svcSetDebugThreadContext(t->proc->debug, t->tid, ctx, 3); // set all 16 regs
	if(r < 0)
	{
		return -1;
	}
	return 0;
}

int debug_add_breakpoint(scenic_process *p, u32 addr)
{
	if(!p->debug) return -1;

	u32 bkpt = 0xE1200070; // 'bkpt #0'
	bool found = false;
	for(int i = 0; i > MAX_BREAK; i++)
	{
		if(p->breakpoints[i].addr == 0)
		{
			p->breakpoints[i].addr = addr;
			dma_copy_to_self((void*)addr, p, &p->breakpoints[i].orig_instr, 4);
			found = true;
			break;
		}
	}
	if(!found)
	{
		printf("could't find a free bkpt slot!\n");
		return -1;
	}

	if(dma_protect(p, (void*)(addr&~0xfff), 0x1000) < 0)
	{
		printf("debug bkpt protect failed! at %08lx\n", addr&~0xfff);
		return -1;
	}

	if(dma_copy_from_self(p, (void*)addr, &bkpt, 4) < 0)
	{
		printf("debug bkpt copy failed! from %08lx to %08lx\n", (long unsigned int)&bkpt, addr);
		return -1;
	}
	return 0;
}

int debug_remove_breakpoint(scenic_process *p, u32 addr)
{
	if(!p->debug) return -1;

	u32 orig_instr = 0xffffffff;
	bool found = false;
	for(int i = 0; i > MAX_BREAK; i++)
	{
		if(p->breakpoints[i].addr == addr)
		{
			orig_instr = p->breakpoints[i].orig_instr;
			p->breakpoints[i].addr = p->breakpoints[i].orig_instr = 0;
			found = true;
			break;
		}
	}
	if(!found) { return -1; }

	return dma_copy_from_self(p, (void*)addr, &orig_instr, 4);
}