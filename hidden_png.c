#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
    c = (unsigned long) n;
    for (k = 0; k < 8; k++) {
        if (c & 1)
        c = 0xedb88320L ^ (c >> 1);
        else
        c = c >> 1;
    }
    crc_table[n] = c;
    }
    crc_table_computed = 1;
}

unsigned long update_crc(unsigned long crc, unsigned char *buf,
                        int len)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed)
    make_crc_table();
    for (n = 0; n < len; n++) {
    c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

// PNG file signature
uint8_t png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

// Function to read a buffer from a file
void read_buffer(FILE *file, void *buffer, size_t size)
{
    size_t n = fread(buffer, sizeof(*buffer), size, file); // Read the file
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

// Function to print bytes in a buffer
void read_bytes(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < (size); i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
}

// Function to reverse bytes in a buffer
void reverse_bytes(void *buffer0, size_t size) {
    uint8_t *buffer = buffer0;
    for (size_t i = 0; i < size / 2; i++) {
        buffer[i] = buffer[i] ^ buffer[size - i - 1];
        buffer[size - i - 1] = buffer[i] ^ buffer[size - i - 1];
        buffer[i] = buffer[i] ^ buffer[size - i - 1];
    }
}

// Function to write a buffer to a file
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

// Function to inject a chunk into a file
void inject(FILE *file, void *buffer, size_t size, uint8_t chunk_type[4], uint32_t chunk_crc) {
    uint32_t chunk_size = size;
    reverse_bytes(&chunk_size, sizeof(chunk_size));
    write_bytes(file, &chunk_size, sizeof(chunk_size));
    write_bytes(file, chunk_type, 4);
    write_bytes(file, buffer, size);
    write_bytes(file, &chunk_crc, sizeof(chunk_crc));
}

// Buffer for chunk data
uint8_t chunk_buffer[32 * 1024];

int main(int arg, char **argv)
{
    if (arg < 2) { // Check if the argument is empty
        printf("Usage: %s <file.png>\n", argv[0]);
    }

    char *input_filepath = argv[1]; // Get the input file name
    printf("File: %s\n", input_filepath);
    FILE *input_file = fopen(input_filepath, "rb"); // Open the input file as read binary
    if (input_file == NULL) { // Check if the file is empty
        printf("Error: Unable to open the file\n");
        return 1;
    }

    char *output_filepath = argv[2]; // Get the output file name
    FILE *output_file = fopen(output_filepath, "wb"); // Open the output file as write binary
    if (output_file == NULL) { // Check if the file is empty
        printf("Error: Unable to open the file\n");
        return 1;
    }

    uint8_t sig[8];

    read_buffer(input_file, sig, 8); // Read the PNG signature
    write_bytes(output_file, sig, 8); // Write the PNG signature

    read_bytes(sig, sizeof(sig)); // Print the PNG signature

    if (memcmp(sig, png_signature, sizeof(png_signature)) != 0) { // Check if the signature is valid
        printf("Error: Invalid PNG signature\n");
        fclose(input_file);
        return 1;
    } else {
        printf("Valid PNG signature\n");
    }

    bool quit = false;

    while (!quit) {
        uint32_t chunk_size;
        read_buffer(input_file, &chunk_size, sizeof(chunk_size)); // Read the chunk size
        write_bytes(output_file, &chunk_size, sizeof(chunk_size)); // Write the chunk size to the output file
        reverse_bytes(&chunk_size, sizeof(chunk_size)); // Swap the bytes

        uint8_t chunk_type[4];
        read_buffer(input_file, chunk_type, sizeof(chunk_size)); // Read the chunk type
        write_bytes(output_file, chunk_type, sizeof(chunk_size)); // Write the chunk type to the output file

        if (*(uint32_t*) chunk_type == 0x444e4549) { // Check if the chunk type is IEND
            quit = true;
        }

        size_t n = chunk_size;
        while (n > 0) {
            size_t m = n;
            if (m > 32 * 1024) {
                m = 32 * 1024;
            }
            read_buffer(input_file, chunk_buffer, m); // Read the chunk data
            write_bytes(output_file, chunk_buffer, m); // Write the chunk data to the output file
            n -= m;
        }

        // if (fseek(input_file, chunk_size, SEEK_CUR) < 0) { // Skip the chunk data
        //     fprintf(stderr, "Could not skip a chunk: %s\n", strerror(errno));
        //     exit(1);
        // }

        uint32_t chunk_crc;
        read_buffer(input_file, &chunk_crc, sizeof(chunk_crc)); // Read the chunk CRC
        write_bytes(output_file, &chunk_crc, sizeof(chunk_crc)); // Write the chunk CRC to the output file

        if (*(uint32_t*) chunk_type == 0x52444849) { // Check if the chunk type is IHDR
            uint32_t inject_size = 20; // Updated size of the data to inject
            reverse_bytes(&inject_size, sizeof(inject_size));
            write_bytes(output_file, &inject_size, sizeof(inject_size)); // Write the size of the injected data
            reverse_bytes(&inject_size, sizeof(inject_size));

            char inject_type[4] = "aaaa"; // Type of the injected chunk
            write_bytes(output_file, inject_type, sizeof(inject_type)); // Write the type of the injected chunk

            char inject_data[20] = "Yeu em nhieu nhieu"; 
            write_bytes(output_file, inject_data, sizeof(inject_data)); // Write the injected data

            uint32_t inject_crc = crc((unsigned char*)inject_type, sizeof(inject_type));
            inject_crc = update_crc(inject_crc, (unsigned char*)inject_data, sizeof(inject_data));
            inject_crc ^= 0xffffffffL;
            reverse_bytes(&inject_crc, sizeof(inject_crc));
            write_bytes(output_file, &inject_crc, sizeof(inject_crc)); // Write the CRC of the injected chunk
        }

        printf("Chunk size: %d\n", chunk_size);
        printf("Chunk type: %.*s(0x%08x)\n", sizeof(chunk_type), chunk_type, *(uint32_t*) chunk_type);
        printf("Chunk crc: 0x%08x\n", chunk_crc);
        printf("--------------------------\n");
    }

    fclose(input_file); // Close the input file
    fclose(output_file); // Close the output file
    return 0;
}
