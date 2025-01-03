
#define CONFIG_X64
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/spawn.h"
#include "kanawha/uapi/process.h"

#include "thread.h"
#include <stdio.h>

static int
thread_entry(void)
{
    struct thread_entry_info *info = &shared_thread_entry_info;
    int(*func)(void*) = info->func;
    void *arg = info->arg;
    spin_unlock(&shared_thread_entry_info_lock);
    return (*func)(arg);
}

int
create_thread(
        int(*thread_f)(void *arg),
        void *arg,
        pid_t *pid)
{
    extern void _thread_start(void);

    spin_lock(&shared_thread_entry_info_lock);

    shared_thread_entry_info.func = thread_f;
    shared_thread_entry_info.arg = arg;

    int res;
    res = kanawha_sys_spawn(
            _thread_start,
            (void*)thread_entry,
            SPAWN_MMAP_SHARED|SPAWN_ENV_CLONE|SPAWN_FILES_CLONE,
            pid);

    if(res) {
        puts("kanawha_sys_spawn Failed!\n");
        kanawha_sys_exit(-res);
    }

    return res;
}



