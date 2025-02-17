#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>

// Hàm đọc tệp PNG
void read_png_file(const char *filename, png_bytep **row_pointers, png_uint_32 *width, png_uint_32 *height, png_byte *color_type, png_byte *bit_depth) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        perror("png_create_read_struct");
        exit(EXIT_FAILURE);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        perror("png_create_info_struct");
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png))) {
        perror("setjmp");
        exit(EXIT_FAILURE);
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    *color_type = png_get_color_type(png, info);
    *bit_depth = png_get_bit_depth(png, info);

    if (*bit_depth == 16)
        png_set_strip_16(png);

    if (*color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (*color_type == PNG_COLOR_TYPE_GRAY && *bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (*color_type == PNG_COLOR_TYPE_RGB ||
        *color_type == PNG_COLOR_TYPE_GRAY ||
        *color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (*color_type == PNG_COLOR_TYPE_GRAY ||
        *color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * (*height));
    for (int y = 0; y < *height; y++) {
        (*row_pointers)[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, *row_pointers);

    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
}

// Hàm ghi tệp JPG
void write_jpeg_file(const char *filename, png_bytep *row_pointers, png_uint_32 width, png_uint_32 height, int quality, png_byte color_type) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE *outfile = fopen(filename, "wb");
    if (!outfile) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // RGB
    cinfo.in_color_space = JCS_RGB;  // Đảm bảo RGB khi ghi JPG

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row_pointer = row_pointers[cinfo.next_scanline];

        // Nếu ảnh PNG có alpha, bạn cần xử lý alpha (chuyển thành RGB)
        if (color_type == PNG_COLOR_TYPE_RGBA) {
            for (int x = 0; x < width; x++) {
                row_pointer[x*3 + 0] = row_pointer[x*4 + 0];  // R
                row_pointer[x*3 + 1] = row_pointer[x*4 + 1];  // G
                row_pointer[x*3 + 2] = row_pointer[x*4 + 2];  // B
            }
        }

        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file.png> <output_file.jpg>\n", argv[0]);
        return 1;
    }

    png_bytep *row_pointers = NULL;
    png_uint_32 width, height;
    png_byte color_type, bit_depth;

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];
    int quality = 10;

    read_png_file(input_file_name, &row_pointers, &width, &height, &color_type, &bit_depth);
    write_jpeg_file(output_file_name, row_pointers, width, height, quality, color_type);

    // Giải phóng bộ nhớ
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);

    printf("Conversion from PNG to JPG completed successfully\n");
    return 0;
}
