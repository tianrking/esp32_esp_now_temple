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

// 默认广播地址
static uint8_t gu8a_dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int create_raw_socket(char *dev)
{
    struct sockaddr_ll s_dest_addr;
    struct ifreq ifr;
    int fd, ifi, rb;

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1);

    memset(&s_dest_addr, 0, sizeof(s_dest_addr));
    s_dest_addr.sll_family = PF_PACKET;
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;
    s_dest_addr.sll_hatype = ARPHRD_ETHER;
    s_dest_addr.sll_pkttype = PACKET_OTHERHOST;
    s_dest_addr.sll_halen = ETH_ALEN;
    memcpy(s_dest_addr.sll_addr, gu8a_dest_mac, ETH_ALEN);

    rb = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));
    assert(rb != -1);

    return fd;
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s <interface> [destination_mac]\n", argv[0]);
        exit(1);
    }

    char *dev = argv[1];
    int sock_fd = -1;

    if (argc == 3) {
        if (sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &gu8a_dest_mac[0], &gu8a_dest_mac[1], &gu8a_dest_mac[2],
                   &gu8a_dest_mac[3], &gu8a_dest_mac[4], &gu8a_dest_mac[5]) != 6) {
            fprintf(stderr, "Invalid MAC address format. Use XX:XX:XX:XX:XX:XX\n");
            exit(1);
        }
    }

    sock_fd = create_raw_socket(dev);
    if (sock_fd == -1)
    {
        perror("Could not create the socket");
        return EXIT_FAILURE;
    }

    printf("Socket created on interface %s\n", dev);
    printf("Sending to MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           gu8a_dest_mac[0], gu8a_dest_mac[1], gu8a_dest_mac[2],
           gu8a_dest_mac[3], gu8a_dest_mac[4], gu8a_dest_mac[5]);

    // 创建一个包含1到100的数组
    uint8_t data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i + 1;
    }

    while (1)
    {
        int s32_res = sendto(sock_fd, data, sizeof(data), 0, NULL, 0);

        if (s32_res == -1)
        {
            perror("Socket send failed");
            close(sock_fd);
            return EXIT_FAILURE;
        }

        printf("Sent %d bytes\n", s32_res);
        for (int i = 0; i < sizeof(data); i++)
        {
            if (i % 10 == 0) printf("\n");
            printf("%3d ", data[i]);
        }
        printf("\n\n");

        sleep(1);  // 每秒发送一次
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
