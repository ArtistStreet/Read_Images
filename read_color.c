#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

void read_file(FILE *file, void *buffer, size_t size) {
    size_t n = fread(buffer, 1, size, file); // pointer, size for each elm, number of elm, file
    if (n != size) { 
        if (ferror(file)) {
            printf("Error: Unable to read the file\n");
            fclose(file);
            exit(1);
        } else if (feof(file)) {
            printf("Error: Unexpected end of file\n");
            fclose(file);
            exit(1);
        } else {
            assert(0);
        }
    }
}

void write_bytes(FILE *file, void *buffer, size_t size) {
    size_t n = fwrite(buffer, size, 1, file);
    if (n != 1) { // Fix the condition to check the return value of fwrite
        if (ferror(file)) {
            fprintf(stderr, "Could not write %zu bytes to file: %s\n", size, strerror(errno));
            exit(1);
        } else {
            assert(0 && "Bui Thi Ly");
        }
    }
}

void read_bytes(uint8_t *buffer, size_t size) {
    printf("Signature: ");
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

uint32_t read_uint32(FILE *file) {
    uint32_t value;
    read_file(file, &value, sizeof(value));

    // Convert from big-endian to host byte order
    value = __builtin_bswap32(value);
    return value;
}

void reverse_bytes(void *buffer0, size_t size) {
    uint8_t *buffer = buffer0;
    for (size_t i = 0; i < size / 2; i++) {
        buffer[i] = buffer[i] ^ buffer[size - i - 1];
        buffer[size - i - 1] = buffer[i] ^ buffer[size - i - 1];
        buffer[i] = buffer[i] ^ buffer[size - i - 1];
    }
}

void write_pixel(FILE *input_file, FILE *output_file, uint32_t chunk_lenght, uint32_t width, uint32_t height, uint8_t color_type) {
    uint8_t *pixel_data = (uint8_t*)malloc(chunk_lenght);
    // fread(pixel_data, 1, chunk_lenght, input_file);

    printf("%u %u", height, width);

    uint32_t index = 0;
    for (uint32_t j = 0; j < 10; j++) {
        for (uint32_t i = 0; j < 10; i++) {
            // fprintf(output_file, "1\n");
            uint8_t r = pixel_data[index++];
            uint8_t g = pixel_data[index++];
            uint8_t b = pixel_data[index++];
            if (color_type == 6) {
                uint8_t a = pixel_data[index++];
                fprintf(output_file, "Pixel at (%u, %u): R=%u, G=%u, B=%u, A=%u\n", i, j, r, g, b, a);
            } else { // RGB
                fprintf(output_file, "Pixel at (%u, %u): R=%u, G=%u, B=%u\n", i, j, r, g, b);
            }
        }
    }

    free(pixel_data);
}

uint8_t chunk_buffer[32 * 1024];

int main(int arg, char **argv) {
    if (arg < 2) { // Check if the argument is empty
        printf("Usage: %s <file.png>\n", argv[0]);
        return 1;
    }

    char *input_file_name = argv[1];
    printf("File: %s\n", input_file_name);
    FILE *input_file = fopen(input_file_name, "rb");
    if (input_file == NULL) {
        printf("Error: Unable to read the file\n");
        return 1;
    }

    char *output_file_name = argv[2];
    FILE *output_file = fopen(output_file_name, "wb"); // Open the output file in write binary mode
    if (output_file == NULL) {
        printf("Error: Unable to open the output file\n");
        fclose(input_file);
        return 1;
    }

    uint8_t sig[8]; // 8 bytes signature

    read_file(input_file, sig, 8); // Read 8 bytes in input file

    read_bytes(sig, sizeof(sig));

    bool quit = false;

    uint32_t width, height;
    uint8_t color_type;

    while (!quit) {
        // uint32_t chunk_lenght = read_uint32(input_file);

        uint32_t chunk_lenght;
        read_file(input_file, &chunk_lenght, sizeof(chunk_lenght)); // Read the chunk size
        reverse_bytes(&chunk_lenght, sizeof(chunk_lenght)); // Swap the bytes

        uint8_t chunk_type[4]; // Alaways 4 bytes  
        read_file(input_file, chunk_type, sizeof(chunk_type));

        // Check if the chunk type is IHDR
        if (*(uint32_t*)chunk_type == 0x52444849) {
            width = read_uint32(input_file);
            height = read_uint32(input_file);
            uint8_t bit_depth = fgetc(input_file);
            color_type = fgetc(input_file);
            uint8_t compression_method = fgetc(input_file);
            uint8_t filter_method = fgetc(input_file);
            uint8_t interlace_method = fgetc(input_file);

            printf("Width: %u\n", width);
            printf("Height: %u\n", height);
            printf("Bit Depth: %u\n", bit_depth);
            printf("Color Type: %u\n", color_type);
            printf("Compression Method: %u\n", compression_method);
            printf("Filter Method: %u\n", filter_method);
            printf("Interlace Method: %u\n", interlace_method);
            
            // fseek(input_file, -13, SEEK_CUR);
        }

        if (*(uint32_t*)chunk_type == 0x54414449) {
            printf("%u %u %u %u", chunk_lenght, width, height, color_type);
            // write_pixel(input_file, output_file, chunk_lenght, width, height, color_type);
            // return 0;
        }

        // Check if the chunk type is IEND
        if (*(uint32_t*) chunk_type == 0x444e4549) { // Check if the chunk type is IEND
            quit = true;
        }

        if (
            *(uint32_t*)chunk_type != 0x52444849 // IHDR
        // &&
            // *(uint32_t*)chunk_type != 0x54414449 // IDAT
        ) {
            size_t n = chunk_lenght;
            while (n > 0) {
                size_t m = n;
                if (m > 32 * 1024) { // 32 KB
                    m = 32 * 1024;
                }
                read_file(input_file, chunk_buffer, m); // Read the chunk data
                n -= m;
            }
        }

        uint32_t chunk_crc;
        read_file(input_file, &chunk_crc, sizeof(chunk_crc));

        printf("Chunk length: %u\n", chunk_lenght);
        printf("Chunk type: %c%c%c%c(0x%08x)\n", chunk_type[0], chunk_type[1], chunk_type[2], chunk_type[3], *(uint32_t*) chunk_type);
        printf("Chunk CRC: 0x%08x\n", chunk_crc);
        printf("-------------------------------\n");
    }

    fclose(input_file); // Close the input file
    fclose(output_file); // Close the output file
    return 0;
}

