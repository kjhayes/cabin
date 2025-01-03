#ifndef __CABIN_SH__THREAD_H__
#define __CABIN_SH__THREAD_H__

static struct thread_entry_info {
    int(*func)(void *arg);
    void *arg;
} shared_thread_entry_info;

static int shared_thread_entry_info_lock = 0;

static int
spin_lock(int *lock)
{
    int val;

    do {
        val = __sync_lock_test_and_set(lock, 1);
    } while(val);

    return 0;
}

static int
spin_unlock(int *lock)
{
    __sync_lock_release(lock);

    return 0;
}

int
create_thread(
        int(*thread_f)(void *arg),
        void *arg,
        pid_t *pid);

#endif
