
#include <qrencode.h> // libqrencode, tvorba QR kódu
#include <png.h>      // libpng, tvorba PNG obrázku
#include <stdlib.h>
#include <string.h>

// https://github.com/fukuchi/libqrencode/blob/715e29fd4cd71b6e452ae0f4e36d917b43122ce8/qrenc.c

#define INCHES_PER_METER (100.0 / 2.54)

QRcode* qrEncodeString(const char *string) {
        QRcode *qr = QRcode_encodeString(string, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (qr == NULL) {
                fprintf(stderr, "Failed to encode string.\n");
                exit(EXIT_FAILURE);
        }
        return qr;
}

void fillRow(unsigned char *row, int num, const unsigned char color[]) {
    int i;

    for (i = 0; i < num; i++) {
        memcpy(row, color, 4);
        row += 4;
    }
}

int qrWritePNG(const QRcode *qrcode, const char *outfile) {
    static unsigned char fg_color[4] = {0, 0, 0, 255};
    static unsigned char bg_color[4] = {255, 255, 255, 255};
    static int dpi = 72;
    static int size = 3;
    static int margin = 1;

    static FILE *fp; // avoid clobbering by setjmp.
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette = NULL;
    png_byte alpha_values[2];
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth;

    realwidth = (qrcode->width + margin * 2) * size;

    row = (unsigned char *)malloc((size_t)((realwidth + 7) / 8));

    if (row == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    if (outfile[0] == '-' && outfile[1] == '\0') {
        fp = stdout;
    } else {
        fp = fopen(outfile, "wb");
        if (fp == NULL) {
            fprintf(stderr, "Failed to create file: %s\n", outfile);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "Failed to initialize PNG writer.\n");
        exit(EXIT_FAILURE);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "Failed to initialize PNG write.\n");
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fprintf(stderr, "Failed to write PNG image.\n");
        exit(EXIT_FAILURE);
    }

    palette = (png_colorp)malloc(sizeof(png_color) * 2);
    if (palette == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(EXIT_FAILURE);
    }
    palette[0].red = fg_color[0];
    palette[0].green = fg_color[1];
    palette[0].blue = fg_color[2];
    palette[1].red = bg_color[0];
    palette[1].green = bg_color[1];
    palette[1].blue = bg_color[2];
    alpha_values[0] = fg_color[3];
    alpha_values[1] = bg_color[3];
    png_set_PLTE(png_ptr, info_ptr, palette, 2);
    png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr,
                 (unsigned int)realwidth, (unsigned int)realwidth,
                 1,
                 PNG_COLOR_TYPE_PALETTE,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_set_pHYs(png_ptr, info_ptr,
                 dpi * INCHES_PER_METER,
                 dpi * INCHES_PER_METER,
                 PNG_RESOLUTION_METER);
    png_write_info(png_ptr, info_ptr);

    /* top margin */
    memset(row, 0xff, (size_t)((realwidth + 7) / 8));
    for (y = 0; y < margin * size; y++)  {
        png_write_row(png_ptr, row);
    }

    /* data */
    p = qrcode->data;
    for (y = 0; y < qrcode->width; y++) {
        memset(row, 0xff, (size_t)((realwidth + 7) / 8));
        q = row;
        q += margin * size / 8;
        bit = 7 - (margin * size % 8);
        for (x = 0; x < qrcode->width; x++) {
            for (xx = 0; xx < size; xx++) {
                *q ^= (*p & 1) << bit;
                bit--;
                if (bit < 0) {
                    q++;
                    bit = 7;
                }
            }
            p++;
        }
        for (yy = 0; yy < size; yy++) {
            png_write_row(png_ptr, row);
        }
    }
    /* bottom margin */
    memset(row, 0xff, (size_t)((realwidth + 7) / 8));
    for (y = 0; y < margin * size; y++) {
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    free(row);
    free(palette);

    return 0;
}
