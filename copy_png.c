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
    write_bytes(output_file, sig, 8); // Write 8 bytes to output file

    read_bytes(sig, sizeof(sig));

    bool quit = false;

    while (!quit) {
        // uint32_t chunk_length = read_uint32(input_file);
        // write_bytes(output_file, &chunk_length, sizeof(chunk_length));

        uint32_t chunk_length;
        read_file(input_file, &chunk_length, sizeof(chunk_length)); // Read the chunk size
        write_bytes(output_file, &chunk_length, sizeof(chunk_length)); // Write the chunk size to the output file
        reverse_bytes(&chunk_length, sizeof(chunk_length)); // Swap the bytes

        uint8_t chunk_type[4]; // Alaways 4 bytes  
        read_file(input_file, chunk_type, sizeof(chunk_type));
        write_bytes(output_file, chunk_type, sizeof(chunk_type));

        // Check if the chunk type is IHDR
        if (*(uint32_t*)chunk_type == 0x52444849) {
            uint32_t width, height;
            width = read_uint32(input_file);
            height = read_uint32(input_file);
            printf("Width: %u\n", width);
            printf("Height: %u\n", height);

            // Skip the remaining IHDR data (5 bytes)
            fseek(input_file, -8, SEEK_CUR);
        }

        // Check if the chunk type is IEND
        if (*(uint32_t*) chunk_type == 0x444e4549) { // Check if the chunk type is IEND
            quit = true;
        }

        size_t n = chunk_length;
        while (n > 0) {
            size_t m = n;
            if (m > 32 * 1024) { // 32 KB
                m = 32 * 1024;
            }
            read_file(input_file, chunk_buffer, m); // Read the chunk data
            write_bytes(output_file, chunk_buffer, m); // Write the chunk data to the output file
            n -= m;
        }

        uint32_t chunk_crc;
        read_file(input_file, &chunk_crc, sizeof(chunk_crc));
        write_bytes(output_file, &chunk_crc, sizeof(chunk_crc));

        printf("Chunk length: %u\n", chunk_length);
        printf("Chunk type: %.*s(0x%08x)\n", sizeof(chunk_type), chunk_type, *(uint32_t*) chunk_type);
        printf("Chunk CRC: 0x%08x\n", chunk_crc);
        printf("-------------------------------\n");
    }

    fclose(input_file); // Close the input file
    fclose(output_file); // Close the output file
    return 0;
}
