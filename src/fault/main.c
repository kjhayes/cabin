
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <kanawha/sys-wrappers.h>
#include <kanawha/sleep.h>

int main(int argc, const char **argv)
{
    int res;

    pid_t pid = fork();

    if(pid == 0) {

        // Child
        printf("Hello From Child! pid=%lld\n", (long long)getpid());

        char * argv[] = {
            "cat",
            "/sys/initrd/hello.txt",
            0,
        };
        execvp("cat", argv);
        perror("execvp");
        return -1;

    } else {
        printf("Hello From Parent! pid=%lld, child_pid=%d\n", (long long)getpid(), pid);
        // Parent
        wait(NULL);
        printf("Waited on Child!\n");
    }
    return 0;
}
