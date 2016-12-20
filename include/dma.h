#pragma once
#include "proc.h"

#ifdef __cplusplus
extern "C"
{
#endif
u32 dma_protect(scenic_process *p, void* addr, u32 size);
int dma_copy(scenic_process *dst, void* dst_p, scenic_process *src, void* src_p, u32 size);
int dma_copy_from_self(scenic_process *dst, void* dst_p, void *src_p, u32 size);
int dma_copy_to_self(void* dst_p, scenic_process *src, void* src_p, u32 size);
void dma_kill_cache(); // for instruction cache

#ifdef __cplusplus
}
#endif
