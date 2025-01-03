
#include <stdio.h>

#include <kanawha/uapi/kbd.h>

const char *progname = "kbdxlate";

void
handle_kbd_event(
        struct kbd_event *evt)
{
    static int shift_pressed = 0;

    kbd_key_t key = evt->key;
    kbd_motion_t motion = evt->motion;

    if(motion == KBD_MOTION_RELEASED) {
        switch(key) {
            case KBD_KEY_LSHIFT:
                shift_pressed = 0;
                break;
            default:
                break;
        }
    } else {

        int no_char = 0;
        char c;
        switch(key) {
            case KBD_KEY_LSHIFT:
                shift_pressed = 1;
                break;
            case KBD_KEY_A: c = shift_pressed ? 'A' : 'a'; break;
            case KBD_KEY_B: c = shift_pressed ? 'B' : 'b'; break;
            case KBD_KEY_C: c = shift_pressed ? 'C' : 'c'; break;
            case KBD_KEY_D: c = shift_pressed ? 'D' : 'd'; break;
            case KBD_KEY_E: c = shift_pressed ? 'E' : 'e'; break;
            case KBD_KEY_F: c = shift_pressed ? 'F' : 'f'; break;
            case KBD_KEY_G: c = shift_pressed ? 'G' : 'g'; break;
            case KBD_KEY_H: c = shift_pressed ? 'H' : 'h'; break;
            case KBD_KEY_I: c = shift_pressed ? 'I' : 'i'; break;
            case KBD_KEY_J: c = shift_pressed ? 'J' : 'j'; break;
            case KBD_KEY_K: c = shift_pressed ? 'K' : 'k'; break;
            case KBD_KEY_L: c = shift_pressed ? 'L' : 'l'; break;
            case KBD_KEY_M: c = shift_pressed ? 'M' : 'm'; break;
            case KBD_KEY_N: c = shift_pressed ? 'N' : 'n'; break;
            case KBD_KEY_O: c = shift_pressed ? 'O' : 'o'; break;
            case KBD_KEY_P: c = shift_pressed ? 'P' : 'p'; break;
            case KBD_KEY_Q: c = shift_pressed ? 'Q' : 'q'; break;
            case KBD_KEY_R: c = shift_pressed ? 'R' : 'r'; break;
            case KBD_KEY_S: c = shift_pressed ? 'S' : 's'; break;
            case KBD_KEY_T: c = shift_pressed ? 'T' : 't'; break;
            case KBD_KEY_U: c = shift_pressed ? 'U' : 'u'; break;
            case KBD_KEY_V: c = shift_pressed ? 'V' : 'v'; break;
            case KBD_KEY_W: c = shift_pressed ? 'W' : 'w'; break;
            case KBD_KEY_X: c = shift_pressed ? 'X' : 'x'; break;
            case KBD_KEY_Y: c = shift_pressed ? 'Y' : 'y'; break;
            case KBD_KEY_Z: c = shift_pressed ? 'Z' : 'z'; break;
            case KBD_KEY_1: c = shift_pressed ? '!' : '1'; break;
            case KBD_KEY_2: c = shift_pressed ? '@' : '2'; break;
            case KBD_KEY_3: c = shift_pressed ? '#' : '3'; break;
            case KBD_KEY_4: c = shift_pressed ? '$' : '4'; break;
            case KBD_KEY_5: c = shift_pressed ? '%' : '5'; break;
            case KBD_KEY_6: c = shift_pressed ? '^' : '6'; break;
            case KBD_KEY_7: c = shift_pressed ? '&' : '7'; break;
            case KBD_KEY_8: c = shift_pressed ? '*' : '8'; break;
            case KBD_KEY_9: c = shift_pressed ? '(' : '9'; break;
            case KBD_KEY_0: c = shift_pressed ? ')' : '0'; break;
            case KBD_KEY_MINUS: c = shift_pressed ? '_' : '-'; break;
            case KBD_KEY_EQUAL_SIGN: c = shift_pressed ? '+' : '='; break;
            case KBD_KEY_BACKTICK: c = shift_pressed ? '~' : '`'; break;
            case KBD_KEY_COMMA: c = shift_pressed ? '<' : ','; break;
            case KBD_KEY_PERIOD: c = shift_pressed ? '>' : '.'; break;
            case KBD_KEY_FSLASH: c = shift_pressed ? '?' : '/'; break;
            case KBD_KEY_SEMICOLON: c = shift_pressed ? ':' : ';'; break;
            case KBD_KEY_SINGLE_QUOT: c = shift_pressed ? '"' : '\''; break;
            case KBD_KEY_OPEN_SQR: c = shift_pressed ? '{' : '['; break;
            case KBD_KEY_CLOSE_SQR: c = shift_pressed ? '}' : ']'; break;
            case KBD_KEY_BSLASH: c = shift_pressed ? '|' : '\\'; break;
            case KBD_KEY_SPACE: c = ' '; break;
            case KBD_KEY_TAB: c = '\t'; break;
            case KBD_KEY_ENTER: c = '\n'; break;
            case KBD_KEY_BACKSPACE: c = '\b'; break;
            default:
                no_char = 1;
                break;
        }

        if(!no_char) {
            putchar(c);
        } else {
            putchar('?');
        }
    }
}

int main(int argc, const char **argv)
{
    int res;

    if(argc > 0) {
        progname = argv[0];
    }

    if(argc != 2) {
        fprintf(stderr,
                "USAGE: %s [KBD-PATH]\n",
                progname);
        return -1;
    }

    const char *kbd_path = argv[1];
    
    FILE *kbd = fopen(kbd_path, "r");

    struct kbd_event event;
    
    while(1) {
        size_t read = fread(&event, sizeof(struct kbd_event), 1, kbd);
        if(read == 0) {
            continue; // Should probably break and report error
        }
        if(read != 1) {
            fprintf(stderr, "Failed to read whole keyboard event!\n");
            continue;
        }

        handle_kbd_event(&event);
    }

    return 0;
}

