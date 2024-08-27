#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <linux/filter.h>

#define MAX_PACKET_LEN 1000

#define FILTER_LENGTH 20
static struct sock_filter bpfcode[FILTER_LENGTH] = {
    {0x30, 0, 0, 0x00000003},
    {0x64, 0, 0, 0x00000008},
    {0x7, 0, 0, 0x00000000},
    {0x30, 0, 0, 0x00000002},
    {0x4c, 0, 0, 0x00000000},
    {0x7, 0, 0, 0x00000000},
    {0x50, 0, 0, 0x00000000},
    {0x54, 0, 0, 0x000000fc},
    {0x15, 0, 10, 0x000000d0},
    {0x40, 0, 0, 0x00000018},
    {0x15, 0, 8, 0x7f18fe34},
    {0x50, 0, 0, 0x00000020},
    {0x15, 0, 6, 0x000000dd},
    {0x40, 0, 0, 0x00000021},
    {0x54, 0, 0, 0x00ffffff},
    {0x15, 0, 3, 0x0018fe34},
    {0x50, 0, 0, 0x00000025},
    {0x15, 0, 1, 0x00000004},
    {0x6, 0, 0, 0x00040000},
    {0x6, 0, 0, 0x00000000},
};

void print_packet(uint8_t *data, int len)
{
    printf("----------------------------new packet-----------------------------------\n");
    for (int i = 0; i < len; i++)
    {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%02x, ", data[i]);
    }
    printf("\n\n");
}

int create_raw_socket(char *dev, struct sock_fprog *bpf)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd, ifi, rb, attach_filter;

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1);

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    rb = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    assert(rb != -1);

    attach_filter = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
    assert(attach_filter != -1);

    return fd;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        exit(1);
    }

    uint8_t buff[MAX_PACKET_LEN] = {0};
    char *dev = argv[1];
    struct sock_fprog bpf = {FILTER_LENGTH, bpfcode};

    int sock_fd = create_raw_socket(dev, &bpf);

    printf("Waiting to receive packets on interface %s...\n", dev);

    while (1)
    {
        int len = recvfrom(sock_fd, buff, MAX_PACKET_LEN, 0, NULL, 0);

        if (len < 0)
        {
            perror("Socket receive failed");
            break;
        }
        else
        {
            printf("Received packet length: %d\n", len);
            print_packet(buff, len);
        }
    }
    close(sock_fd);
    return 0;
}
