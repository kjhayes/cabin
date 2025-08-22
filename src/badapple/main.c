
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <kanawha/kbd.h>
#include <kanawha/time.h>
#include <kanawha/sys-wrappers.h>
#include <kfb/kfb.h>
#include "loadpng.h"

static int kfb_framebuffer_mode = -ENXIO;
static struct kfb_framebuffer *kfb_buffer = NULL;

static const char *frame_path_prefix = "./";

static fd_t kbd_file;

static int get_input(void)
{
    int res;
    struct kbd_event event;
    size_t read = kanawha_sys_read(kbd_file, &event, sizeof(struct kbd_event));
    if(read == 0) {
        return 0;
    }
    if(read != sizeof(struct kbd_event)) {
        fprintf(stderr, "Failed to read whole keyboard event!\n");
        return 0;
    }

    int pressed = (event.motion == KBD_MOTION_PRESSED) || (event.motion == KBD_MOTION_HELD);
    if(pressed) {
        switch(event.key) {
          case KBD_KEY_0:
            kfb_framebuffer_mode++;
            res = kfb_set_current_mode(kfb_buffer, kfb_framebuffer_mode);
            if(res) {
                kfb_framebuffer_mode--;
                break;
            }
            printf("Changed to mode: %d\n", kfb_framebuffer_mode);
            break;
          case KBD_KEY_9:
            if(kfb_framebuffer_mode > 0) {
                kfb_framebuffer_mode--;
                res = kfb_set_current_mode(kfb_buffer, kfb_framebuffer_mode);
                if(res) {
                    kfb_framebuffer_mode++;
                }
                printf("Changed to mode: %d\n", kfb_framebuffer_mode);
            }
            break;
          default:
            break;
        }
    }
}

static int draw_frame(void)
{
    int res;

    static char image_name[128];
    static int frame_no = 1;

    snprintf(image_name, 128, "%sbad_apple_%03d.png", frame_path_prefix, frame_no);
    image_name[127] = '\0';

    struct PNGImage *cur_image = load_png_rgba(image_name);

    if(cur_image == NULL) {
	printf("Failed to load image %s (Resetting)\n", image_name);
	frame_no = 1;
	return 0;
    } else {
	frame_no++;
    }

    struct kfb_image img = {
        .data = (void*)cur_image->data,
        .resx = cur_image->width,
        .resy = cur_image->height,
        .order = GFX_ORDER_ROW_MAJOR,
        .format = GFX_FORMAT_RGBA32,
        .offset = 0,
        .stride = 4,
        .data_size = (cur_image->width)*(cur_image->height)*4,
    };

    printf("Drawing Frame: %s\n", image_name);
    //printf("Drawing Frame (width=%lu,height=%lu)\n",
    //        (unsigned long)kfb_buffer->current_mode_info->layer_infos[0].width,
    //        (unsigned long)kfb_buffer->current_mode_info->layer_infos[0].height
    //        );
    res = kfb_blit_image_onto_layer(
            kfb_buffer,
            0,
            &img,
            0, 0,
            kfb_buffer->current_mode_info->layer_infos[0].layout.width, 
            kfb_buffer->current_mode_info->layer_infos[0].layout.height 
            );
    if(res) {
        fprintf(stderr, "Failed to blit frame! err=%d\n", res);
        return res;
    }

    free_png_image(cur_image);

    res = kfb_flush_framebuffer(kfb_buffer);
    if(res) {
        fprintf(stderr, "Failed to flush framebuffer!\n");
        return res;
    }
}

int main(int argc, const char **argv)
{
    int res;

    if(argc < 3) {
        fprintf(stderr, "Usage: badapple [FB-PATH] [KBD-PATH]\n");
        exit(-1);
    }

    const char *fb_path = argv[1];
    kfb_buffer = kfb_load_framebuffer(fb_path);
    if(kfb_buffer == NULL) {
        fprintf(stderr, "Failed to open framebuffer \"%s\"!\n", fb_path);
        exit(-1);
    }

    if(!kfb_buffer->have_buffer_data) {
        fprintf(stderr, "Failed to open framebuffer \"%s\" (kfb missing buffer data)!\n", fb_path);
        exit(-1);
    }

    kfb_framebuffer_mode = kfb_get_current_mode(kfb_buffer);
    printf("Opened Framebuffer \"%s\" in mode %d\n", fb_path, (int)kfb_framebuffer_mode);

    const char *kbd_path = argv[2];
    res = kanawha_sys_open(
            kbd_path,
            FILE_PERM_READ,
            FILE_MODE_NON_BLOCK,
            &kbd_file);
    if(res) {
        fprintf(stderr, "Failed to open \"%s\"\n", kbd_path);
        exit(-1);
    }

    while(1) {
        draw_frame();
        get_input();
    }
}

