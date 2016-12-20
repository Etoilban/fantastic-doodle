#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

Result svcGetDebugThreadContext(scenic_debug_thread_ctx* context, Handle debug, u32 threadId, u32 controlFlags);
Result svcSetDebugThreadContext(scenic_debug_thread_ctx* context, Handle debug, u32 threadId, u32 controlFlags);
Result svcSetHardwareBreakpoint(s32 registerId, u32 control, u32 value);

Result fixed_svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount);
Result svcGetThreadList(s32* threadCount, u32* threadIds, s32 threadIdMaxCount, Handle domain);

#ifdef __cplusplus
}
#endif
