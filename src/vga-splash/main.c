
#include "kanawha/sys-wrappers.h"
#include "kanawha/uapi/file.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static const char *prog_name = "vga-fb-term";

static inline uint16_t
vga_encode(uint8_t c, uint8_t attr) {
    return (((uint16_t)attr) << 8) | c;
}

#define FB_CHARSIZE 2
#define FB_WIDTH 80
#define FB_HEIGHT 25

/*
 * Emulate a terminal on top of a vga 80x25 textmode framebuffer file
 */
int main(int argc, const char **argv)
{
    int res;

    if(argc > 0) {
        prog_name = argv[0];
    }
    if(argc != 3) {
        fprintf(stderr, "Usage: %s [VGA-FB-PATH] [SPLASH-TXT-PATH]\n",
                prog_name);
        return -EINVAL;
    }
    const char *fb_path = argv[1];
    const char *sp_path = argv[2];

    fd_t fb_file;
    fd_t sp_file;

    res = kanawha_sys_open(
            fb_path,
            FILE_PERM_READ|FILE_PERM_WRITE,
            0,
            &fb_file);
    if(res) {
        fprintf(stderr, "Failed to open file: %s\n",
                fb_path);
        return res;
    }
    res = kanawha_sys_open(
            sp_path,
            FILE_PERM_READ,
            0,
            &sp_file);
    if(res) {
        fprintf(stderr, "Failed to open file: %s\n",
                sp_path);
        return res;
    }

    void *fb_buffer;
    res = kanawha_sys_mmap(
            fb_file,
            0,
            &fb_buffer,
            ((FB_CHARSIZE * FB_WIDTH * FB_HEIGHT) + 0xFFF) & ~0xFFF,
            MMAP_SHARED|MMAP_PROT_READ|MMAP_PROT_WRITE);
    if(res) {
        fprintf(stderr, "Failed to mmap file: %s\n",
                fb_path);
        return res;
    }

    char *sp_buffer;
    res = kanawha_sys_mmap(
            sp_file,
            0,
            (void**)&sp_buffer,
            ((FB_CHARSIZE * FB_WIDTH * FB_HEIGHT) + 0xFFF) & ~0xFFF,
            MMAP_SHARED|MMAP_PROT_READ);
    if(res) {
        fprintf(stderr, "Failed to mmap file: %s\n",
                fb_path);
        return res;
    }

    for(size_t i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        uint8_t backcolor = 0x1; // dark blue
        uint8_t forecolor = 0xF;
        uint8_t attr = (backcolor << 4) | (forecolor & 0x0F);
        ((uint16_t*)fb_buffer)[i] = vga_encode(sp_buffer[i], attr);
    }
    kanawha_sys_flush(fb_file, 0);

    getchar();

    return 0;
}

