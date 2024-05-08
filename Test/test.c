#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_FILE "/dev/caesardev"
#define IOCTL_SET_SHIFT _IO(0x12, 1)

int main()
{
    int fd, ret;
    char buffer[1024];
    int shift_key;

    // Open the device file
    fd = open(DEVICE_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device file");
        return 1;
    }

    // Test case 1: Write a string to the device
    char *input_str = "Hello, World!";
    printf("Writing to device: %s\n", input_str);
    ret = write(fd, input_str, strlen(input_str));
    if (ret < 0)
    {
        perror("Failed to write to device");
        close(fd);
        return 1;
    }

    // Test case 2: Encrypt the string with a shift key
    shift_key = 3;
    printf("Setting shift key to %d\n", shift_key);
    ret = ioctl(fd, IOCTL_SET_SHIFT, shift_key);
    if (ret < 0)
    {
        perror("Failed to set shift key");
        close(fd);
        return 1;
    }

    // Test case 3: Read the encrypted string from the device
    memset(buffer, 0, sizeof(buffer));
    ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0)
    {
        perror("Failed to read from device");
        close(fd);
        return 1;
    }
    printf("Encrypted string: %s\n", buffer);

    // Test case 4: Decrypt the string with a different shift key
    shift_key = 25;
    printf("Setting shift key to %d\n", shift_key);
    ret = ioctl(fd, IOCTL_SET_SHIFT, shift_key);
    if (ret < 0)
    {
        perror("Failed to set shift key");
        close(fd);
        return 1;
    }

    // Test case 5: Read the decrypted string from the device
    memset(buffer, 0, sizeof(buffer));
    ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0)
    {
        perror("Failed to read from device");
        close(fd);
        return 1;
    }
    printf("Decrypted string: %s\n", buffer);

    // Close the device file
    close(fd);

    return 0;
}