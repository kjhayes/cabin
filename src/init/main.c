
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/spawn.h"
#include "kanawha/uapi/environ.h"
#include "kanawha/uapi/errno.h"
#include "kanawha/uapi/mount.h"
#include "kanawha/uapi/file.h"

#include <string.h>

//#define STDIN_PATH  "/kbd/ps2-kbd-0"
//#define STDOUT_PATH "/chr/vga-serial"
//#define STDERR_PATH "/chr/vga-serial"
#define STDIN_PATH  "/chr/COM0"
#define STDOUT_PATH "/chr/COM0"
#define STDERR_PATH "/chr/COM0"

static const char *exec_path = NULL;

static int
setup_stdstreams(void)
{
    int res;

    fd_t _stdin, _stdout, _stderr;

    res = kanawha_sys_open(
            STDIN_PATH,
            FILE_PERM_READ,
            0,
            &_stdin);
    if(res) {
        goto err0;
    }

    if(_stdin != 0) {
        res = kanawha_sys_fmove(_stdin, 0, FMOVE_SWAP);
        if(res) {
            goto err1;
        }
        _stdin = 0;
    }

    res = kanawha_sys_open(
            STDOUT_PATH,
            FILE_PERM_WRITE,
            0,
            &_stdout);
    if(res) {
        goto err1;
    }

    if(_stdout != 1) {
        res = kanawha_sys_fmove(_stdout, 1, FMOVE_SWAP);
        if(res) {
            goto err2;
        }
        _stdout = 1;
    }

    res = kanawha_sys_open(
            STDERR_PATH,
            FILE_PERM_WRITE,
            0,
            &_stderr);
    if(res) {
        goto err1;
    }

    if(_stderr != 2) {
        res = kanawha_sys_fmove(_stderr, 2, FMOVE_SWAP);
        if(res) {
            goto err2;
        }
        _stderr = 2;
    }

    return 0;

err3:
    kanawha_sys_close(_stderr);
err2:
    kanawha_sys_close(_stdout);
err1:
    kanawha_sys_close(_stdin);
err0:
    return res;
}

int
run_exec_thread(
        int(*thread_f)(void),
        pid_t *pid)
{
    extern void _thread_start(void);

    int res;

    res = kanawha_sys_spawn(
            _thread_start,
            (void*)thread_f,
            SPAWN_MMAP_SHARED|SPAWN_ENV_CLONE|SPAWN_FILES_CLONE,
            pid);

    if(res) {
        kanawha_sys_exit(-res);
    }

    return res;
}

int exec_thread(void)
{
    // This thread shares the address space,
    // has a copy of the environment, and no files opened

    int res;

    if(exec_path == NULL) {
        return 1;
    }
 
    res = setup_stdstreams();
    if(res) {
        return res;
    }
   
    fd_t exec_fd;

    res = kanawha_sys_open(
            exec_path,
            FILE_PERM_READ|FILE_PERM_EXEC,
            0,
            &exec_fd);

    if(res) {
        return res;
    }

    // We're going to be leaking a whole stack here (whoops).
    res = kanawha_sys_exec(exec_fd, 0);
    if(res) {
        kanawha_sys_exit(res);
    }

    kanawha_sys_exit(1);
}

int main(int argc, const char **argv)
{
    int res;

    fd_t root;
    res = kanawha_sys_open(
            "/",
            0,
            0,
            &root);
    if(res) {
        return res;
    }

    res = kanawha_sys_mount(
            "chardev",
            root,
            "chr",
            "sys",
            MOUNT_SPECIAL);
    if(res) {
        return res;
    }

    res = kanawha_sys_mount(
            "kbd",
            root,
            "kbd",
            "sys",
            MOUNT_SPECIAL);
    if(res) {
        return res;
    }

    pid_t child_pid;

    if(argc < 2) {
        return 2;
    }

    exec_path = argv[1];

    if(exec_path == NULL) {
        return 3;
    }

    res = run_exec_thread(exec_thread, &child_pid);
    if(res) {
        kanawha_sys_exit(-res);
    }
    int exitcode;
    do {
        int reap_ret = kanawha_sys_reap(child_pid, 0, &exitcode);
        if(reap_ret == 0) {
            break;
        }
        if(reap_ret == -ENXIO) {
            return -1;
        }
    } while(1);

    return exitcode;
}

