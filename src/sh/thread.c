
#define CONFIG_X64
#include <kanawha/sys-wrappers.h>
#include <kanawha/spawn.h>
#include <kanawha/process.h>

#include "thread.h"

static int shared_thread_entry_info_lock = 0;

struct thread_entry_info shared_thread_entry_info;

int
init_threads(void) {
    shared_thread_entry_info_lock = 0;
    return 0;
}

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
        spin_unlock(&shared_thread_entry_info_lock);
        return res;
    }

    return res;
}

