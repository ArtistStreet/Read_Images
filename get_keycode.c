#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#define KEYBOARD_DEVICE "/dev/input/event1" // Cần thay đổi cho đúng bàn phím của bạn

int main() {
    int fd = open(KEYBOARD_DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Không thể mở thiết bị bàn phím");
        return 1;
    }

    struct input_event ev;
    printf("Đang theo dõi bàn phím... (Nhấn Ctrl+C để thoát)\n");

    while (read(fd, &ev, sizeof(struct input_event)) > 0) {
        if (ev.type == EV_KEY) {
            printf("Keycode: %d, Trạng thái: %d\n", ev.code, ev.value);
        }
    }

    close(fd);
    return 0;
}
