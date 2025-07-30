
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <kanawha/kbd.h>
#include <kanawha/time.h>
#include <kanawha/sys-wrappers.h>
#include <kanawha/kfb.h>

struct image {
    const uint32_t *width;
    const uint32_t *height;
    const uint32_t *data;
};


extern const uint32_t whiscash_width;
extern const uint32_t whiscash_height;
extern const uint32_t whiscash[];

extern const uint32_t parrot_width;
extern const uint32_t parrot_height;
extern const uint32_t parrot[];

static struct image images[] = {
    {
        .width = &whiscash_width,
        .height = &whiscash_height,
        .data = whiscash,
    },
    {
        .width = &parrot_width,
        .height = &parrot_height,
        .data = parrot,
    },
};

#define NUM_IMAGES (sizeof(images) / sizeof(images[0]))

static int kfb_framebuffer_mode = -ENXIO;
static struct kfb_framebuffer *kfb_buffer = NULL;

static int current_image = 0;

static int width;
static int height;

#define MOVE_SPEED 2

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
          case KBD_KEY_O:
            if(current_image < NUM_IMAGES-1) {
                current_image++;
                printf("Set current image to %d\n", current_image);
            }
            break;
          case KBD_KEY_I:
            if(current_image > 0) {
                current_image--;
                printf("Set current image to %d\n", current_image);
            }
            break;
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
          case KBD_KEY_L:
            width += MOVE_SPEED;
            break;
          case KBD_KEY_H:
            width -= MOVE_SPEED;
            if (width < 0) {
                width = 0;
            }
            break;
          case KBD_KEY_J:
            height += MOVE_SPEED;
            break;
          case KBD_KEY_K:
            height -= MOVE_SPEED;
            if (height < 0) {
                height = 0;
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
    struct image *cur_image = &images[current_image];
    struct kfb_image img = {
        .data = (void*)cur_image->data,
        .resx = *cur_image->width,
        .resy = *cur_image->height,
        .order = FB_LAYER_ORDER_ROW_MAJOR,
        .format = FB_LAYER_FORMAT_RGBA32,
        .offset = 0,
        .stride = 4,
        .data_size = (*cur_image->width)*(*cur_image->height)*4,
    };

    //printf("Drawing Frame (width=%lu,height=%lu)\n",
    //        (unsigned long)kfb_buffer->current_mode_info->layer_infos[0].width,
    //        (unsigned long)kfb_buffer->current_mode_info->layer_infos[0].height
    //        );
    res = kfb_blit_image_onto_layer(
            kfb_buffer,
            0,
            &img,
            0, 0,
            width,
            height
            );
    if(res) {
        fprintf(stderr, "Failed to blit frame! err=%d\n", res);
        return res;
    }

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
        fprintf(stderr, "Usage: whiscash [FB-PATH] [KBD-PATH]\n");
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

    width = kfb_buffer->current_mode_info->layer_infos[0].width;
    height = kfb_buffer->current_mode_info->layer_infos[0].height;

    while(1) {
        draw_frame();
        get_input();
    }
}

