#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define SERIAL_PORT "/dev/ttyACM0"
//#define BAUD_RATE 1000000
#define BAUD_RATE 115200

int configureSerialPort(int fd);

int main() {
    // Open
    int serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }

    // Configure
    if (configureSerialPort(serial_fd) != 0) {
        close(serial_fd);
        return 1;
    }

    // Send and read
    do {
        // Send message
        const char *test_message = "Hello there";
        size_t message_length = strlen(test_message);
        ssize_t bytes_written = write(serial_fd, test_message, message_length);
        if (bytes_written < 0) {
            perror("Failed to write to serial port");
            close(serial_fd);
            return 1;
        }
        printf("Wrote %zd bytes: %s\n", bytes_written, test_message);

        // Read message
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        printf("Waiting for response...\n");
        ssize_t bytes_read;
        bytes_read = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0 && errno != EAGAIN) {
            perror("Failed to read from serial port");
            close(serial_fd);
            return 1;
        }
        usleep(100000);
        printf("Read %zd bytes: %s\n", bytes_read, buffer);    
    } while (true);

    // Done
    close(serial_fd);
    return 0;
}


int configureSerialPort(int fd) {
    struct termios options;

    // Get the current attributes of the serial port
    if (tcgetattr(fd, &options) != 0) {
        perror("Failed to get serial port attributes");
        return -1;
    }

    // Set the baud rates
    if (cfsetspeed(&options, BAUD_RATE) != 0) {
        perror("Failed to set baud rate");
        return -1;
    }

    // Configure 8N1
    options.c_cflag &= ~PARENB;  // No parity
    options.c_cflag &= ~CSTOPB;  // One stop bit
    options.c_cflag &= ~CSIZE;   // Clear data size bits
    options.c_cflag |= CS8;      // 8 data bits

    // Enable the receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);

    // Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;

    // Raw input/output mode
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    options.c_oflag &= ~OPOST;

    // Apply the new attributes
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("Failed to set serial port attributes");
        return -1;
    }

    return 0;
}

