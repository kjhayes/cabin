
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define REST (0)
#define C2  (65)
#define CS2 (69)
#define D2  (73)
#define DS2 (78)
#define E2  (82)
#define F2  (87)
#define FS2 (92)
#define G2  (98)
#define GS2 (104)
#define A2  (110)
#define AS2 (116)
#define B2  (123)
#define C3  (131)
#define CS3 (139)
#define D3  (147)
#define DS3 (156)
#define E3  (165)
#define F3  (175)
#define FS3 (185)
#define G3  (196)
#define GS3 (208)
#define A3  (220)
#define AS3 (233)
#define B3  (247)

#define _QUARTER(NOTE) NOTE, NOTE, NOTE, NOTE,
#define _HALF(NOTE) _QUARTER(NOTE) _QUARTER(NOTE) 
#define _FULL(NOTE) _HALF(NOTE) _HALF(NOTE)
#define _DOUBLE(NOTE) _FULL(NOTE) _FULL(NOTE)
#define _QUAD(NOTE) _DOUBLE(NOTE) _DOUBLE(NOTE)

#define QUARTER(NOTE) _QUARTER(NOTE) _QUARTER(REST)
#define HALF(NOTE)    _HALF(NOTE) _QUARTER(REST)
#define FULL(NOTE)    _FULL(NOTE) _QUARTER(REST)
#define DOUBLE(NOTE)  _DOUBLE(NOTE) _QUARTER(REST)
#define QUAD(NOTE)    _QUAD(NOTE) _QUARTER(REST)

uint16_t
tune_0[] =
{
    FULL(A2)
    FULL(B2)
    FULL(D3)
    FULL(B2)

    DOUBLE(F3)
    QUARTER(REST)
    DOUBLE(F3)
    QUARTER(REST)
    DOUBLE(E3)

    QUAD(REST)

    FULL(A2)
    FULL(B2)
    FULL(D3)
    FULL(B2)

    DOUBLE(E3)
    QUARTER(REST)
    DOUBLE(E3)
    QUARTER(REST)
    DOUBLE(D3)
    FULL(CS3)
    DOUBLE(B2)

    QUAD(REST)

    FULL(A2)
    FULL(B2)
    FULL(D3)
    FULL(B2)

    DOUBLE(D3)
    DOUBLE(E3)
    QUAD(CS3)
    QUARTER(REST)
    FULL(A2)
    DOUBLE(E3)
    DOUBLE(D3)
};

static uint16_t
tune_1[] = {
    DOUBLE(C2)
    DOUBLE(D2)
    DOUBLE(E2)
    DOUBLE(F2)
    DOUBLE(G2)
    DOUBLE(A2)
    DOUBLE(B2)
    DOUBLE(C3)
};

int main(int argc, const char **argv)
{
    FILE *file = fopen("/dev/snd/pc-speaker", "a");
    if(file == NULL) {
        printf("Failed to open /dev/snd/pc-speaker!\n");
        exit(-1);
    }

    uint16_t *tune = tune_1;
    int num_notes = sizeof(tune_1) / 2;

    printf("Playing tune of length %d\n", num_notes);
    ssize_t res = fwrite(tune, 2, num_notes, file);
    if(res < 0) {
        printf("Failed to write to /dev/snd/pc-speaker!\n");
        exit(res);
    }
    printf("Finished playing\n");

    return 0;
}

