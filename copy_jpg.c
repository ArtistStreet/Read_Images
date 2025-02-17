#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h> // Include this header for ntohs

void read_file(FILE *file, void *buffer, size_t size) {
    size_t n = fread(buffer, 1, size, file);
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

void write_file(FILE *file, void *buffer, size_t size) {
    size_t n = fwrite(buffer, 1, size, file);
    if (n != size) {
        if (ferror(file)) {
            // fprintf(stderr, "Could not write %zu bytes to file: %s\n", size, strerror(errno));
            exit(1);
        } else {
            assert(0 && "Unexpected write error");
        }
    }
}

void error(uint8_t *data, FILE *input_file, FILE *output_file) {
    if (data == NULL) {
        printf("Error: Memory allocation failed\n");
        fclose(input_file);
        fclose(output_file);
        exit(1);
    }
} 

void read_and_write(FILE *input_file, FILE *output_file, void *buffer, size_t size) {
    read_file(input_file, buffer, size);
    write_file(output_file, buffer, size);
}


u_int32_t ptr;

void extract_appn_data(FILE *input_file, FILE *output_file) {
    uint8_t marker[2]; // 2 bytes của marker APPn

    read_file(input_file, marker, 2);

    while (marker[0] == 0xFF && (marker[1] >= 0xE0 && marker[1] <= 0xEF)) {
        write_file(output_file, marker, 2);
        
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);
        
        uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
        uint8_t *appn_data = malloc(length - 2); // Trừ đi trường độ dài 2 byte
        
        error(appn_data, input_file, output_file);
        
        read_and_write(input_file, output_file, appn_data, length - 2);
        
        free(appn_data);
        
        ptr = ftell(input_file);
        // if (marker[0] == 0xFF && marker[1] == 0xE0) {
        //     printf("1");
        //     marker[0] = 0xFF, marker[1] = 0xFE; 
        //     write_file(output_file, marker, 2);
        // }
        read_file(input_file, marker, 2);
        // printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);
    }
}

void inject_data(FILE *input_file, FILE *output_file, const char *data) {
    fwrite(data, 1, strlen(data), output_file);
}

void text(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET); // Set the file position to the saved pointer
    
    uint8_t marker[2];
    read_file(input_file, marker, 2);
    
    printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);
    while (marker[1] != 0xDB) {
        write_file(output_file, marker, 2);
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);

        uint16_t length = (length_bytes[0] << 8) | length_bytes[1]; // Convert bytes to uint16_t
        uint8_t *dqt_data = malloc(length - 2); // Exclude the 2-byte length field

        error(dqt_data, input_file, output_file);

        read_and_write(input_file, output_file, dqt_data, length - 2);

        free(dqt_data);

        ptr = ftell(input_file);
        read_file(input_file, marker, 2);
    }
}

void inject(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET);

    uint8_t marker[2];
    read_file(input_file, marker, 2);

    if (marker[0] != 0xFF && marker[1] != 0xFE) {
        marker[0] = 0xFF, marker[1] = 0xFE; 
        write_file(output_file, marker, 2);
    }
    // printf("1234");
    // write_file(output_file, marker, 2);
}

void extract_dqt_data(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET); // Set the file position to the saved pointer
    
    uint8_t marker[2];
    read_file(input_file, marker, 2);
    // printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);

    while (marker[0] == 0xFF && marker[1] == 0xDB) {
        write_file(output_file, marker, 2);
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);

        uint16_t length = (length_bytes[0] << 8) | length_bytes[1]; // Convert bytes to uint16_t
        uint8_t *dqt_data = malloc(length - 2); // Exclude the 2-byte length field

        error(dqt_data, input_file, output_file);

        read_and_write(input_file, output_file, dqt_data, length - 2);

        free(dqt_data);

        ptr = ftell(input_file);
        read_file(input_file, marker, 2);
    }
}

void extract_sof_data(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET); // Set the file position to the saved pointer
    uint8_t marker[2];
    
    read_file(input_file, marker, 2); // Read the marker
    // printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);
    if (marker[0] == 0xFF && marker[1] == 0xDD) {
        write_file(output_file, marker, 2);
    }
    while (marker[0] == 0xFF && marker[1] == 0xDD) {
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);

        uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
        uint8_t *dri_data = malloc(length - 2); // Exclude the 2-byte length field

        read_and_write(input_file, output_file, dri_data, length - 2);
        free(dri_data);

        read_file(input_file, marker, 2);
    }
    // restart_marker(input_file, output_file);
    
    while (marker[0] == 0xFF && (marker[1] >= 0xC0 && marker[1] <= 0xC3)) { // Check if it's a SOF marker
        // printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);
        write_file(output_file, marker, 2); // Write the marker to the output file
        
        uint8_t length_bytes[2];
        // read_and_write(input_file, output_file, length_bytes, 2);
        read_file(input_file, length_bytes, 2); // Read the length of the SOF segment
        write_file(output_file, length_bytes, 2); // Write the length to the output file
        
        uint16_t length = (length_bytes[0] << 8) | length_bytes[1]; // Convert bytes to uint16_t
        uint8_t *sof_data = malloc(length - 2); // Exclude the 2-byte length field
        
        // if (marker[0] == 0 && marker[1] == 0) return;
        error(sof_data, input_file, output_file);

        // read_and_write(input_file, output_file, sof_data, 2);
        read_file(input_file, sof_data, length - 2); // Read the SOF data
        write_file(output_file, sof_data, length - 2); // Write the SOF data to the output file

        uint8_t precision = sof_data[0];

        uint8_t height_bytes[2] = { sof_data[1], sof_data[2] }; // Extract height bytes
        uint16_t height = (height_bytes[0] << 8) | height_bytes[1]; // Convert height bytes to uint16_t
        
        uint8_t width_bytes[2] = { sof_data[3], sof_data[4] }; // Extract width bytes
        uint16_t width = (width_bytes[0] << 8) | width_bytes[1]; // Convert width bytes to uint16_t
        
        uint8_t components = sof_data[5];
        // Print width and height to the screen
        printf("Precision: %d\n", precision);
        printf("Width: %d\n", width);
        printf("Height: %d\n", height);
        printf("Components: %d\n", components);
        // printf("ID: %d\n", id);
        
        free(sof_data); // Free the allocated memory

        ptr = ftell(input_file); // Save the current file position
        read_file(input_file, marker, 2); // Read the next marker
    }
}

void extract_dht_data(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET); // Set the file position to the saved pointer
    uint8_t marker[2];

    read_file(input_file, marker, 2); // Read the marker

    while (marker[0] == 0xFF && marker[1] == 0xC4) { // Check if it's a DHT marker
        write_file(output_file, marker, 2); // Write the marker to the output file
        
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);

        uint16_t length = (length_bytes[0] << 8) | length_bytes[1]; // Convert bytes to uint16_t
        uint8_t *dht_data = malloc(length - 2); // Exclude the 2-byte length field

        error(dht_data, input_file, output_file);

        read_and_write(input_file, output_file, dht_data, length - 2);

        free(dht_data); // Free the allocated memory

        ptr = ftell(input_file); // Save the current file position
        read_file(input_file, marker, 2); // Read the next marker
    }
}

void extract_sos_data(FILE *input_file, FILE *output_file) {
    fseek(input_file, ptr, SEEK_SET); // Set the file position to the saved pointer
    uint8_t marker[2];

    read_and_write(input_file, output_file, marker, 2);
    
    uint8_t length_bytes[2];
    read_and_write(input_file, output_file, length_bytes, 2);

    uint16_t length = (length_bytes[0] << 8) | length_bytes[1]; // Convert bytes to uint16_t
    uint8_t *sos_data = malloc(length - 2); // Exclude the 2-byte length field

    error(sos_data, input_file, output_file);

    read_and_write(input_file, output_file, sos_data, length - 2);

    free(sos_data); // Free the allocated memory

    while (fread(marker, 1, 2, input_file) == 2) {
        if (marker[0] == 0xFF && marker[1] == 0xD9) {// End of image
            write_file(output_file, marker, 2);
            break;
        }
        else {
            write_file(output_file, marker, 1);
            fseek(input_file, -1, SEEK_CUR);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    char *input_file_name = argv[1];
    char *output_file_name = argv[2];
    printf("Input File: %s\n", input_file_name);
    printf("Output File: %s\n", output_file_name);

    FILE *input_file = fopen(input_file_name, "rb");
    if (input_file == NULL) {
        printf("Error: Unable to open input file\n");
        return 1;
    }

    FILE *output_file = fopen(output_file_name, "wb");
    if (output_file == NULL) {
        printf("Error: Unable to open output file\n");
        fclose(input_file);
        return 1;
    }

    uint8_t soi_marker[2]; // SOI
    read_and_write(input_file, output_file, soi_marker, 2);
    
    extract_appn_data(input_file, output_file); // APPn
    
    // inject(input_file, output_file);
    inject_data(input_file, output_file, "Hello");
    
    text(input_file, output_file);

    extract_dqt_data(input_file, output_file); // DQT

    extract_sof_data(input_file, output_file); // SOF

    extract_dht_data(input_file, output_file); // DHT
    
    extract_sos_data(input_file, output_file); // SOS
    
    fclose(input_file);
    fclose(output_file);

    return 0;
}
