#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "beidou.h"

uint32_t ref_time_int = 0;
uint32_t ref_time_fract = 0;
pthread_t beidou_thread; // 线程句柄
static int beidou_thread_id;
uint32_t transmit_time_int = 0;

uint32_t receive_time_int = 0;
uint32_t receive_time_fract = 0;

uint32_t originate_time_int = 0;
uint32_t originate_time_fract = 0;

void fill_uint32_to_char(char *buf, uint32_t data)
{
    uint8_t i = 0;
    for (i = 0; i < 4; i++)
    {
        buf[i] = (data >> (8 * (3 - i))) & 0xFF;
    }
}

void createNtpResponse(char *buffer, char *revFrame)
{

    memset(buffer, 0, NTP_PACKET_SIZE);

    // Set response mode (4) and version (3)
    buffer[0] = 0x24;
    // 1C 即 00 00 011 100
    // 00 100 100 即 0x24

    // Set stratum level, in this case, a server
    buffer[1] = 2;

    // Set poll interval and precision
    buffer[2] = 0x0C;
    buffer[3] = 0xF7;

    buffer[4] = 0x0;
    buffer[5] = 0x0;
    buffer[6] = 0x09;
    buffer[7] = 0x40;

    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10] = 0x12;
    buffer[11] = 0x1b;

    buffer[12] = 0x0a;
    buffer[13] = 0x89;
    buffer[14] = 0x35;
    buffer[15] = 0x07;

    // 8 bytes for the reference timestamp
    fill_uint32_to_char(&buffer[16], ref_time_int);
    fill_uint32_to_char(&buffer[20], ref_time_fract);

    // 8 bytes for the originate timestamp
    for (int i = 0; i < 8; i++)
    {
        buffer[24 + i] = revFrame[40 + i];
    }

    // 8 bytes for the receive timestamp
    fill_uint32_to_char(&buffer[32], receive_time_int);
    fill_uint32_to_char(&buffer[36], receive_time_fract);

    // 8 bytes for the transmit timestamp
    uint32_t current_time = time(NULL) + NTP_UNIX_OFFSET;
    fill_uint32_to_char(&buffer[40], current_time);
    fill_uint32_to_char(&buffer[44], get_current_fractional_time());
}

// wasted
long parse_transmit_timestamp(char *data)
{
    struct timeval tv;
    int i = 0;

    originate_time_int = originate_time_int | (uint32_t)(data[0]) << 24;
    originate_time_int = originate_time_int | (uint32_t)(data[1]) << 16;
    originate_time_int = originate_time_int | (uint32_t)(data[2]) << 8;
    originate_time_int = originate_time_int | (uint32_t)(data[3]);

    originate_time_fract = originate_time_fract | (uint32_t)(data[4]) << 24;
    originate_time_fract = originate_time_fract | (uint32_t)(data[5]) << 16;
    originate_time_fract = originate_time_fract | (uint32_t)(data[6]) << 8;
    originate_time_fract = originate_time_fract | (uint32_t)(data[7]);

    // printf("transmit_time:%d\n", transmit_time);

    if (gettimeofday(&tv, NULL) == 0)
    {
        // srand(time(NULL));
        // int random_number = rand() % 1000;
        // unsigned long long nseconds = tv.tv_usec * 1000 + (long long)random_number;
        // printf("us: %llu\n", nseconds);
        // float ntp_double = (float)nseconds / 1000000;
        //  printf("ntp_double: %f\n", ntp_double);
        // unsigned long long sec = (unsigned long long)tv.tv_sec;
        // printf("sec: %llu\n", sec);
    }
    // uint32_t current_time = time(NULL) + NTP_UNIX_OFFSET;
    printf("transmit_time:%lu current_time:%lu\n", originate_time_int, originate_time_fract);
    // 3906924501
    // 3724188984
}

int main(void)
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    char ntpRevPacket[REV_FULL_SIZE];
    char ntpFBPacket[NTP_PACKET_SIZE];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(NTP_PORT);

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    // 建一个beidou的线程
    if (pthread_create(&beidou_thread, NULL, beidou_process, &beidou_thread_id) != 0)
    {
        perror("create beidou thread failed");
        return 1;
    }

    while (1)
    {

        // Receive request from client
        int revLen = recvfrom(sockfd, ntpRevPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (revLen < 0)
        {
            perror("Receive failed");
            close(sockfd);
            exit(1);
        }

        if (revLen != NTP_PACKET_SIZE)
        {
            perror("Not ntp package");
            continue;
        }

        receive_time_int = time(NULL) + NTP_UNIX_OFFSET;
        receive_time_fract = get_current_fractional_time();

        // Print a message when NTP request is received
        printf("Received a time request from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // parse_transmit_timestamp(&ntpPacket[40]);
        // parse_originate_timestamp(&ntpPacket[24]);
        memset(ntpFBPacket, 0, NTP_PACKET_SIZE);
        // Create and send NTP response
        createNtpResponse(ntpFBPacket, ntpRevPacket);
        if (sendto(sockfd, ntpFBPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_len) < 0)
        {
            perror("Send failed");
            close(sockfd);
            exit(1);
        }
    }

    close(sockfd);
    return 0;
}
