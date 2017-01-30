/*
   tlininterp.c
   David Rowe
   Jan 2017

   Fast linear interpolator for high oversam[pling rates.  Upsample
   with a decent filter first such that the signal is "low pass" wrt
   to the input sample rate.

   build: gcc tlininterp.c -o tlininterp -Wall -O2

*/

#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define SIGNED_16BIT   0
#define SIGNED_8BIT    1

void display_help(void) {
    fprintf(stderr, "\nusage: tlininterp inputRawFile OutputRawFile OverSampleRatio [-c]\n");
    fprintf(stderr, "\nUse - for stdin/stdout\n\n");
    fprintf(stderr, "-c complex signed 16 bit input and output\n");
    fprintf(stderr, "-d complex signed 16 bit input, complex signed 8 bit output\n\n");
}

int main(int argc, char *argv[]) {
    FILE       *fin, *fout;
    short       left[2], right[2], out[2], i;
    float       oversample, t;
    int8_t      out_s8[2];

    if (argc < 3) {
	display_help();
	exit(1);
    }

    if (strcmp(argv[1], "-") == 0) 
        fin = stdin;
    else
        fin = fopen(argv[1], "rb");
    assert(fin != NULL);

    if (strcmp(argv[2], "-") == 0) 
        fout = stdout;
    else
        fout = fopen(argv[2], "wb");
    assert(fout != NULL);

    oversample = atof(argv[3]);

    int channels = 1;
    int format = SIGNED_16BIT;
    int opt;
    while ((opt = getopt(argc, argv, "cd")) != -1) {
        switch (opt) {
        case 'c': channels = 2; break;
        case 'd': channels = 2; format = SIGNED_8BIT; break;
        default:
            display_help();
            exit(1);
        }
    }

    for (i=0; i<channels; i++)
        left[i] = 0;
    t = 0.0;
    while(fread(&right, sizeof(short)*channels, 1, fin) == 1) {
        while (t < 1.0) {

            for (i=0; i<channels; i++) {
                out[i] = (1.0 - t)*left[i] + t*right[i];
            }

            if (format == SIGNED_16BIT) {
                fwrite(&out, sizeof(short), channels, fout);
            } else {
                //fprintf(stderr,"8 bit out\n");
                for (i=0; i<channels; i++) {
                    out_s8[i] = out[i] >> 8;
                }
                fwrite(&out_s8, sizeof(int8_t), channels, fout);
            }

            t += 1.0/oversample;
        }
        t -= 1.0;
        for (i=0; i<channels; i++)
            left[i] = right[i];
    }

    fclose(fout);
    fclose(fin);

    return 0;
}
