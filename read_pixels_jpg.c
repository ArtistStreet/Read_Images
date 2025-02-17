#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void read_jpeg_pixels(const char *filename) {
    FILE *infile = fopen(filename, "rb");
    FILE *out = fopen("output.txt", "w");
    if (infile == NULL) {
        perror("Error opening file");
        return;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Thiết lập cấu trúc jpeg_decompress_struct
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Đọc thông tin từ tệp JPEG
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    // Giải mã tệp JPEG
    jpeg_start_decompress(&cinfo);

    // Lấy thông tin về kích thước ảnh
    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int num_components = cinfo.output_components;  // 3 cho ảnh RGB

    // Cấp phát bộ nhớ cho pixel
    unsigned char *buffer = (unsigned char *)malloc(width * num_components);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return;
    }

    // Đọc từng dòng pixel
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *row_pointer[1];
        row_pointer[0] = buffer;

        jpeg_read_scanlines(&cinfo, row_pointer, 1);

        // Xử lý từng pixel trong dòng hiện tại
        for (int i = 0; i < width; i++) {
            unsigned char r = buffer[i * num_components];      // Đọc kênh đỏ (Red)
            unsigned char g = buffer[i * num_components + 1];  // Đọc kênh xanh (Green)
            unsigned char b = buffer[i * num_components + 2];  // Đọc kênh xanh dương (Blue)

            // In giá trị màu của pixel
            fprintf(out, "Pixel (%d, %d): R=%d, G=%d, B=%d\n", i, cinfo.output_scanline - 1, r, g, b);
        }
    }

    // Giải phóng bộ nhớ và đóng tệp
    free(buffer);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
}

int main() {
    const char *filename = "test.jpg";  // Đường dẫn tệp JPEG của bạn
    read_jpeg_pixels(filename);
    return 0;
}
