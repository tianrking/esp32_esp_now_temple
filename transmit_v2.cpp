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

int packetSetUp(uint8_t* packet, uint8_t* mac_dest, uint8_t* mac_src, uint8_t* payload, int len );

static uint8_t mac_src[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t mac_dest[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t gu8a_dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


// static uint8_t mac_src[6] = {0xC8, 0xF0, 0x9E, 0xA1, 0xCE, 0x58};
// static uint8_t mac_dest[6] = {0xC8, 0xF0, 0x9E, 0xA1, 0xCE, 0x58};
// static uint8_t gu8a_dest_mac[6] = {0xC8, 0xF0, 0x9E, 0xA1, 0xCE, 0x58};

// 要发送的字符串
static uint8_t payload[] = "abc";  
static int len = sizeof(payload);
// uint8_t esppacket[400] = {0};
uint8_t esppacket[4] = {0};

int create_raw_socket(char *dev)
{
    struct sockaddr_ll s_dest_addr; //code from sender
    struct ifreq ifr;       
    int fd, ifi, rb;

    bzero(&s_dest_addr, sizeof(s_dest_addr));
    bzero(&ifr, sizeof(ifr));
    
    (void)memset(&s_dest_addr, 0, sizeof(s_dest_addr));
    
    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1); //abort if error
    
    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr); 
    assert(ifi != -1); //abort if error
    
    s_dest_addr.sll_family = PF_PACKET;    
    //we don't use a protocol above ethernet layer, just use anything here
    s_dest_addr.sll_protocol = htons(ETH_P_ALL);  
    s_dest_addr.sll_ifindex = ifr.ifr_ifindex;
    s_dest_addr.sll_hatype = ARPHRD_ETHER;
    s_dest_addr.sll_pkttype = PACKET_OTHERHOST; //PACKET_OUTGOING
    s_dest_addr.sll_halen = ETH_ALEN;         
    //MAC - begin
    s_dest_addr.sll_addr[0] = gu8a_dest_mac[0];
    s_dest_addr.sll_addr[1] = gu8a_dest_mac[1];
    s_dest_addr.sll_addr[2] = gu8a_dest_mac[2];
    s_dest_addr.sll_addr[3] = gu8a_dest_mac[3];
    s_dest_addr.sll_addr[4] = gu8a_dest_mac[4];
    s_dest_addr.sll_addr[5] = gu8a_dest_mac[5];
    //MAC - end
    s_dest_addr.sll_addr[6] = 0x00;   //not used
    s_dest_addr.sll_addr[7] = 0x00;   //not used

    rb = bind(fd, (struct sockaddr *)&s_dest_addr, sizeof(s_dest_addr));
    assert(rb != -1); //abort if error
    
    return fd;
}

int main(int argc, char **argv)  
{
    char *dev = argv[1];
    
    int sock_fd = -1;
    int32_t s32_res = -1;
    
    sock_fd = create_raw_socket(dev); /* Creating the raw socket */
    
    if (-1 == sock_fd)
    {
        perror("Could not create the socket");
        goto LABEL_CLEAN_EXIT;
    }
    
    printf("Socket created\n");
    
    fflush(stdout);
    
    sleep(1);
    
    printf("******Sending data using raw socket over  %s \n", dev);
    
    while (1) 
    {
        int length = packetSetUp(esppacket,mac_dest,mac_src,payload,len);
        
        // 打印完整的数据包
        printf("Packet hex dump:\n");
        for (int i = 0; i < length; i++) {
            printf("%02x ", esppacket[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        // 打印payload文本
        printf("Payload: %s\n", payload);
        
        s32_res = sendto(sock_fd,esppacket,length,0,NULL,0);
        
        if (-1 == s32_res)
        {
            perror("Socket send failed");
            goto LABEL_CLEAN_EXIT;
        }
        
        usleep(1000000);
    }

LABEL_CLEAN_EXIT:
    if (sock_fd > 0) 
    {
        close(sock_fd);
    }
    
    printf("***** Raw Socket test- end\n");
    
    return EXIT_SUCCESS;
}

struct ieee80211_radiotap_header  
{
    uint8_t radiotap_version;                //= 0;  
    uint8_t radiotap_pad;                    //= 0;
    uint16_t radiotap_lenght;                //= 0x00,0x26;
    uint32_t radiotap_present_1;             //= {0x2f, 0x40, 0x00, 0xa0};  
    uint32_t radiotap_present_2;             //= {0x20, 0x08, 0x00, 0xa0};
    uint32_t radiotap_present_3;             //= {0x20, 0x08, 0x00, 0x00}; 
    uint8_t radiotap_flags;          //= 10;
    uint8_t radiotap_datarate;               //0x0c
    uint16_t radiotap_channel_freq;          //0x6c, 0x09   
    uint16_t radiotap_channel_flags_quarter; //0xc0, 0x00
    
} __attribute__((__packed__));

struct ieee80211_wlan  
{
    uint8_t wlan_fc_subtype;          //0xd0   
    uint8_t wlan_fc_order;            //0x00
    uint16_t wlan_duration;           //0x3a, 0x01
    uint8_t wlan_da[6];               //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint8_t wlan_sa[6];               //0xf8, 0x1a, 0x67, 0xb7, 0xeb, 0x0b    
    uint8_t wlan_bssid[6];            //0x84, 0xf3, 0xeb, 0x73, 0x55, 0x0d
    uint16_t wlan_seq;                //0x70, 0x51  
    uint8_t wlan_fiwed_category_code; //0x7f
    uint8_t wlan_tag_oui[3];          //0x18,0xfe, 0x34
     
    uint8_t espheader[11]; 
    
    uint8_t payload[250]; 
    
} __attribute__((__packed__));

typedef struct  
{
    struct ieee80211_radiotap_header rtap_header;
    struct ieee80211_wlan mypayload;
    
} espPacket;
  
int packetSetUp(uint8_t* packet, uint8_t* mac_dest, uint8_t* mac_src, uint8_t* payload, int len )
{
    espPacket mypacket;
    
    mypacket.rtap_header.radiotap_version = 0;
    mypacket.rtap_header.radiotap_pad = 0;
    mypacket.rtap_header.radiotap_lenght = sizeof(mypacket.rtap_header);
    mypacket.rtap_header.radiotap_present_1 = 0xfe;

    mypacket.rtap_header.radiotap_flags = 0x10;
    mypacket.rtap_header.radiotap_datarate = 0x0c;
    mypacket.rtap_header.radiotap_channel_freq = 0x096c;
    mypacket.rtap_header.radiotap_channel_flags_quarter = 0x00c0;
    
    mypacket.mypayload.wlan_fc_subtype = 0xd0;
    mypacket.mypayload.wlan_fc_order = 0x00;
    mypacket.mypayload.wlan_duration = 0x013a;

    
    // memcpy(mypacket.mypayload.wlan_sa,mac_src,6);
    
    memcpy(mypacket.mypayload.wlan_bssid,mac_dest,6);
    
    mypacket.mypayload.wlan_seq = 0x5170;
    mypacket.mypayload.wlan_fiwed_category_code = 0x7f;
    mypacket.mypayload.wlan_tag_oui[0] = 0x18;
    mypacket.mypayload.wlan_tag_oui[1] = 0xfe; 
    mypacket.mypayload.wlan_tag_oui[2] = 0x34;

    mypacket.mypayload.espheader[0] = 0xa2;//element ID
    mypacket.mypayload.espheader[1] = 0x03;//length of Organization Identifier
    mypacket.mypayload.espheader[2] = 0x92;
    mypacket.mypayload.espheader[3] = 0xb0;
    mypacket.mypayload.espheader[4] = 0xdd;
    mypacket.mypayload.espheader[5] = 0xff;
    mypacket.mypayload.espheader[6] = 0x18;//Organization ID
    mypacket.mypayload.espheader[7] = 0xfe;//Organization ID 
    mypacket.mypayload.espheader[8] = 0x34;//Organization ID
    mypacket.mypayload.espheader[9] = 0x04;//type (should be 4)
    mypacket.mypayload.espheader[10] = 0x1;//version field

    memcpy(mypacket.mypayload.payload, payload, len);

     // 填充未使用部分为 0
    memset(mypacket.mypayload.payload + len, 0, sizeof(mypacket.mypayload.payload) - len);

    
    memcpy(packet,&mypacket,sizeof(ieee80211_radiotap_header) + sizeof(ieee80211_wlan));
    
    // return sizeof(ieee80211_radiotap_header) ;
    return sizeof(ieee80211_radiotap_header) + sizeof(ieee80211_wlan);
    
}