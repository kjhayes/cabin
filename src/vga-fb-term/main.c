
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
    if(argc != 2) {
        fprintf(stderr, "Usage: %s [VGA-FB-PATH]\n",
                prog_name);
        return -EINVAL;
    }
    const char *fb_path = argv[1];

    fd_t file;
    res = kanawha_sys_open(
            fb_path,
            FILE_PERM_READ|FILE_PERM_WRITE,
            0,
            &file);
    if(res) {
        fprintf(stderr, "Failed to open file: %s\n",
                fb_path);
        return res;
    }

    void *fb_buffer;
    res = kanawha_sys_mmap(
            file,
            0,
            &fb_buffer,
            ((FB_CHARSIZE * FB_WIDTH * FB_HEIGHT) + 0xFFF) & ~0xFFF,
            MMAP_SHARED|MMAP_PROT_READ|MMAP_PROT_WRITE);
    if(res) {
        fprintf(stderr, "Failed to mmap file: %s\n",
                fb_path);
        return res;
    }

    int cur_x = 0;
    int cur_y = 0;

#define CR() \
    do {\
        cur_x = 0;\
    } while(0)
#define LF() \
    do {\
        cur_y++;\
        if(cur_y >= FB_HEIGHT) {\
            memmove(fb_buffer,\
                    fb_buffer + (FB_CHARSIZE * FB_WIDTH),\
                    (FB_CHARSIZE * FB_WIDTH * (FB_HEIGHT-1)));\
            memset(fb_buffer + (FB_CHARSIZE * FB_WIDTH * (FB_HEIGHT-1)), 0, FB_CHARSIZE * FB_WIDTH);\
            cur_y = FB_HEIGHT-1;\
        }\
    } while(0)
#define ADVANCE_CUR() \
    do {\
        cur_x++;\
        if(cur_x >= FB_WIDTH) {\
            CR();\
            LF();\
        }\
    } while(0)

#define SET_CUR(_c)\
    do {\
        ((uint16_t*)fb_buffer)[cur_x + (cur_y * FB_WIDTH)] = \
            vga_encode(_c, 0x7); \
    } while(0)

#define CLEAR() \
    do {\
        memset(fb_buffer, 0, FB_CHARSIZE * FB_WIDTH * FB_HEIGHT);\
        cur_x = 0;\
        cur_y = 0;\
    } while(0)

    CLEAR();
    kanawha_sys_flush(file, 0);
    while(1) {
        char c = getchar();
        if(c == '$') {
            break;
        }
        if(c == '\n') {
            CR();
            LF();
        }
        if(c == '\r') {
            // Ignore \r
        }
        if(c == '\t') {
            for(int i = 0; i < 4; i++) {
                SET_CUR(' ');
                ADVANCE_CUR();
            }
        }
        if(c == '\b') {
            cur_x--;
            if(cur_x >= 0) {
                SET_CUR(' ');
            } else {
                cur_x = 0;
            }
        }
        else {
            SET_CUR(c);
            ADVANCE_CUR();
        }
        kanawha_sys_flush(file, 0);
    }

    return 0;
}

