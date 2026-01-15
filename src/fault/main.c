
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <kanawha/sys-wrappers.h>

static inline uint64_t __attribute__((always_inline))
rdtsc (void)
{
    uint32_t lo, hi;
    __asm__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return lo | ((uint64_t)(hi) << 32);
}

struct {
    uint64_t offset;
    uint16_t selector;
} __attribute__((packed))
call_gate = {
    .selector = 56 | 0x3,
    .offset = 0,
};

static inline unsigned long
run_test(void) {
    uint32_t lo, hi;
    uint64_t before = rdtsc();
    __asm__ (
	    "leaq call_gate, %%rax;"
            "rex.w lcall *(%%rax);"
	    : "=a" (lo), "=d" (hi));
    uint64_t after = rdtsc();
    uint64_t in_kernel_time = lo | ((uint64_t)(hi) << 32);
    printf("Returned from far call... before=0x%lx, in_kernel_time=0x%lx, after=0x%lx\n",
	    before, in_kernel_time, after);
    uint64_t entry = in_kernel_time - before;
    uint64_t ret = after - in_kernel_time;
    uint64_t total = after-before;
    printf("to_kernel=%lu, ret_to_user=%lu, total=%lu\n",
	    entry,
	    ret,
	    total);
    return total;
}

int main(int argc, const char **argv)
{
    unsigned long min = run_test();
    size_t runs = 1;
    unsigned long max = min;
    unsigned long sum = 0;
    for(size_t i = 0; i < 1000; i++) {
	unsigned long cur = run_test();
	if(cur < min) {
	    min = cur;
	}
	if(cur > max) {
	    max = cur;
	}
	sum += cur;
	runs++;
    }
    printf("Minimum(%lu) Maximum(%lu) Avg(%lu)\n",
	    min, max, sum / runs);
    return 0;
}

