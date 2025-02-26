#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DNS_SERVER "8.8.8.8"
#define DNS_PORT 53
#define BUFFER_SIZE 1024

// DNS header structure
struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

// DNS question structure
struct dns_question {
    uint16_t qtype;
    uint16_t qclass;
};

// DNS resource record structure
struct dns_rr {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
};

// Function to build DNS query
void build_dns_query(char *domain, char *buffer, int *len) {
    struct dns_header *header = (struct dns_header *)buffer;
    header->id = htons(0x1234); // Random ID
    header->flags = htons(0x0100); // Standard query, recursion desired
    header->qdcount = htons(1); // One question
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;

    // Point to the question section
    char *qname = buffer + sizeof(struct dns_header);

    // Convert domain to QNAME format
    char *part = strtok(domain, ".");
    while (part != NULL) {
        *qname++ = strlen(part);
        memcpy(qname, part, strlen(part));
        qname += strlen(part);
        part = strtok(NULL, ".");
    }
    *qname++ = 0; // End of QNAME

    // Add question type and class
    struct dns_question *question = (struct dns_question *)qname;
    question->qtype = htons(1); // A record
    question->qclass = htons(1); // Internet class

    *len = qname + sizeof(struct dns_question) - buffer;
}

// Function to parse DNS response
void parse_dns_response(char *buffer, int len) {
    struct dns_header *header = (struct dns_header *)buffer;
    int qdcount = ntohs(header->qdcount);
    int ancount = ntohs(header->ancount);

    // Skip header and question section
    char *ptr = buffer + sizeof(struct dns_header);
    while (*ptr != 0) {
        ptr++;
    }
    ptr += 5; // Skip QTYPE and QCLASS

    // Parse answer section
    for (int i = 0; i < ancount; i++) {
        struct dns_rr *rr = (struct dns_rr *)ptr;
        ptr += sizeof(struct dns_rr);

        if (ntohs(rr->type) == 1) { // A record
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, ptr, ip, INET_ADDRSTRLEN);
            printf("IP: %s\n", ip);
        }

        ptr += ntohs(rr->rdlength); // Move to next record
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int len;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure DNS server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, DNS_SERVER, &server_addr.sin_addr);

    // Build DNS query
    char domain[] = "example.com";
    build_dns_query(domain, buffer, &len);

    // Send DNS query
    if (sendto(sockfd, buffer, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receive DNS response
    int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len < 0) {
        perror("Receive failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Parse DNS response
    parse_dns_response(buffer, recv_len);

    close(sockfd);
    return 0;
}