#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

#define MAX_PACKET_SIZE 250
#define HEADER_SIZE 81

// ESP-NOW 帧结构常量
#define ESPNOW_CATEGORY 0x7F
#define ESPNOW_ORGANIZATION_ID 0x18FE34
#define ESPNOW_TYPE 0x04
#define ESPNOW_VERSION 0x01

static uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int create_raw_socket(char *dev)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd;

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        close(fd);
        return -1;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    return fd;
}

void build_esp_now_frame(uint8_t *frame, const uint8_t *data, size_t data_len)
{
    int offset = 0;

    // Fixed header based on the original code
    uint8_t header[HEADER_SIZE] = {
        0x00, 0x00, 0x26, 0x00, 0x2f, 0x40, 0x00, 0xa0, 0x20, 0x08, 0x00, 0xa0, 0x20, 0x08, 0x00, 0x00,
        0xdf, 0x32, 0xfe, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0c, 0x6c, 0x09, 0xc0, 0x00, 0xd3, 0x00,
        0x00, 0x00, 0xd3, 0x00, 0xc7, 0x01, 0xd0, 0x00, 0x3a, 0x01, 0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d,
        0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b, 0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d, 0x70, 0x51, 0x7f, 0x18,
        0xfe, 0x34, 0xa2, 0x03, 0x92, 0xb0, 0xdd, 0xff, 0x18, 0xfe, 0x34, 0x04, 0x01, 0x29
    };

    memcpy(frame, header, HEADER_SIZE);
    offset += HEADER_SIZE;

    // Copy actual data
    memcpy(frame + offset, data, data_len);
    offset += data_len;

    // Fill the rest with 0x12 (as in the original code)
    memset(frame + offset, 0x12, MAX_PACKET_SIZE - offset - 4);

    // Add the trailing bytes (as in the original code)
    frame[MAX_PACKET_SIZE - 4] = 0x1c;
    frame[MAX_PACKET_SIZE - 3] = 0xd5;
    frame[MAX_PACKET_SIZE - 2] = 0x35;
    frame[MAX_PACKET_SIZE - 1] = 0xd3;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *dev = argv[1];
    char *message = argv[2];
    int sock_fd;
    uint8_t frame[MAX_PACKET_SIZE + HEADER_SIZE] = {0};

    sock_fd = create_raw_socket(dev);
    if (sock_fd < 0) {
        fprintf(stderr, "Failed to create socket\n");
        return EXIT_FAILURE;
    }

    printf("Socket created on interface %s\n", dev);

    size_t message_len = strlen(message);
    if (message_len > MAX_PACKET_SIZE) {
        fprintf(stderr, "Message too long. Maximum length is %d bytes.\n", MAX_PACKET_SIZE);
        close(sock_fd);
        return EXIT_FAILURE;
    }

    build_esp_now_frame(frame, (uint8_t*)message, message_len);

    printf("Sending ESP-NOW frame with message: %s\n", message);

    int bytes_sent = send(sock_fd, frame, sizeof(frame), 0);
    if (bytes_sent < 0) {
        perror("Socket send failed");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("Frame sent successfully (%d bytes)\n", bytes_sent);

    close(sock_fd);
    return EXIT_SUCCESS;
}
