#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Hàm đọc một lượng byte từ tệp
void read_file(FILE *file, uint8_t *buffer, size_t size) {
    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read != size) {
        perror("Lỗi khi đọc tệp");
        exit(1);  // Hoặc bạn có thể trả về nếu không muốn thoát ngay lập tức
    }
}

// Hàm ghi một lượng byte vào tệp
void write_file(FILE *file, uint8_t *buffer, size_t size) {
    size_t bytes_written = fwrite(buffer, 1, size, file);
    if (bytes_written != size) {
        perror("Lỗi khi ghi tệp");
        exit(1);  // Hoặc xử lý lỗi khác nếu cần
    }
}

// Hàm xử lý đọc và ghi đồng thời
void read_and_write(FILE *input_file, FILE *output_file, uint8_t *buffer, size_t size) {
    read_file(input_file, buffer, size);
    write_file(output_file, buffer, size);
}

// Hàm kiểm tra lỗi khi cấp phát bộ nhớ
void check_memory_allocation(void *ptr) {
    if (ptr == NULL) {
        perror("Lỗi cấp phát bộ nhớ");
        exit(1);  // Nếu không thể cấp phát bộ nhớ, thoát chương trình
    }
}

// Hàm trích xuất dữ liệu APPn từ tệp JPEG và ghi vào tệp đầu ra
void extract_appn_data(FILE *input_file, FILE *output_file) {
    uint8_t marker[2];  // 2 bytes của marker APPn

    // Đọc marker đầu tiên
    read_file(input_file, marker, 2);

    // Vòng lặp để xử lý các marker APPn từ 0xFFE0 đến 0xFFEF
    while (marker[0] == 0xFF && (marker[1] >= 0xE0 && marker[1] <= 0xEF)) {
        // Ghi marker hiện tại vào tệp đầu ra
        write_file(output_file, marker, 2);

        // Đọc chiều dài của đoạn APPn (2 byte)
        uint8_t length_bytes[2];
        read_and_write(input_file, output_file, length_bytes, 2);

        // Chuyển đổi chiều dài từ bytes sang uint16_t
        uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
        uint8_t *appn_data = malloc(length - 2); // Trừ đi trường độ dài 2 byte

        // Kiểm tra xem việc cấp phát bộ nhớ có thành công không
        check_memory_allocation(appn_data);

        // Đọc và ghi dữ liệu APPn
        read_and_write(input_file, output_file, appn_data, length - 2);

        // Giải phóng bộ nhớ đã cấp phát cho dữ liệu APPn
        free(appn_data);

        // Đọc marker tiếp theo
        read_file(input_file, marker, 2);

        // In ra để kiểm tra marker hiện tại
        printf("Found marker: 0x%02X%02X\n", marker[0], marker[1]);
    }
}

int main() {
    const char *input_filename = "600px-Cat03.jpg";  // Đường dẫn tệp JPEG của bạn
    const char *output_filename = "output.jpg";  // Tệp đầu ra để ghi dữ liệu APPn

    // Mở tệp đầu vào và đầu ra
    FILE *input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        perror("Không thể mở tệp đầu vào");
        return 1;
    }

    FILE *output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        perror("Không thể mở tệp đầu ra");
        fclose(input_file);
        return 1;
    }

    // Trích xuất dữ liệu APPn và ghi vào tệp
    extract_appn_data(input_file, output_file);

    // Đóng tệp sau khi hoàn thành
    fclose(input_file);
    fclose(output_file);

    return 0;
}

