
#include <kanawha/sys-wrappers.h>
#include <kanawha/uapi/mount.h>
#include <kanawha/uapi/file.h>
#include <stdio.h>

int main(int argc, const char **argv) {
    int res;

    if(argc < 5) {
        puts("mount: Too Few Arguments\n");
        puts("Usage: mount [SOURCE] [DIR] [MOUNTPOINT] [TYPE] {SPECIAL}\n");
        return -1;
    }

    unsigned long flags = MOUNT_FILE;

    if(argc > 5) {
        flags = MOUNT_SPECIAL;
    }

    const char *source   = argv[1];
    const char *dir_path = argv[2];
    const char *mnt_pnt  = argv[3];
    const char *fs_type  = argv[4];

    fd_t dir;
    res = kanawha_sys_open(
            dir_path,
            FILE_PERM_READ,
            0,
            &dir);
    if(res) {
        fprintf(stderr, "mount: Failed to open directory \"%s\"\n", dir_path);
        return res;
    }

    res = kanawha_sys_mount(
            source,
            dir,
            mnt_pnt,
            fs_type,
            flags);
    if(res) {
        fprintf(stderr, "mount: mount syscall failed!\n");
        return res;
    }

    return 0;
}

