#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>

#define TTL_VALUE 64
#define PACKET_SIZE 64
#define MAX_TRIES 3
#define TIMEOUT 1000

struct icmp_packet
{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequence_number;
	struct timeval timestamp;
	char data[];
};

uint16_t calculate_checksum(const void *data, size_t length)
{
	uint32_t sum = 0;
	const uint16_t *ptr = data;

	while (length > 1)
	{
		sum += *ptr++;
		length -= 2;
	}

	if (length == 1)
		sum += *(uint8_t *)ptr;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);

	return ~sum;
}

void handle_alarm(int signal)
{
	(void)signal;
	// Do nothing
}

double get_elapsed_time(struct timeval start_time, struct timeval end_time)
{
	return (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
}

int ping(const char* ip_address)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sockfd = -1;
	int ttl = TTL_VALUE;
	int tries = 0;
	int success = 0;
	struct icmp_packet *packet;
	struct sockaddr_in target_addr;
	struct timeval start_time, end_time;
	double elapsed_time;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	if (getaddrinfo(ip_address, NULL, &hints, &result) != 0)
	{
		fprintf(stderr, "Error: getaddrinfo() failed: %s\n", gai_strerror(errno));
		return 1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd < 0)
			continue;

		if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
		{
			perror("setsockopt");
			close(sockfd);
			return 1;
		}

		break;
	}

	if (rp == NULL)
	{
		fprintf(stderr, "Error: Could not create socket\n");
		return 1;
	}

	memcpy(&target_addr, rp->ai_addr, sizeof(struct sockaddr_in));
	freeaddrinfo(result);

	packet = malloc(sizeof(struct icmp_packet) + PACKET_SIZE - 1);
	if (packet == NULL)
	{
		fprintf(stderr, "Error: malloc() failed\n");
		close(sockfd);
		return 1;
	}

	packet->type = 8;
	packet->code = 0;
	packet->checksum = 0;
	packet->identifier = getpid() & 0xFFFF;
	packet->sequence_number = 0;
	gettimeofday(&packet->timestamp, NULL);
	gettimeofday(&start_time, NULL);

	memset(packet->data, 0xA5, PACKET_SIZE);
	packet->checksum = calculate_checksum(packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1);

	signal(SIGALRM, handle_alarm);

	while (tries < MAX_TRIES && !success)
	{
		if (sendto(sockfd, packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
		{
			perror("sendto");
			close(sockfd);
			free(packet);
			return 1;
		}

		alarm(TIMEOUT);

		if (recvfrom(sockfd, packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1, 0, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", packet->sequence_number + 1);
				tries++;
				continue;
			}
			else
			{
				perror("recvfrom");
				close(sockfd);
				free(packet);
				return 1;
			}
		}

		gettimeofday(&end_time, NULL);
		elapsed_time = get_elapsed_time(start_time, end_time);

		struct iphdr *ip_header = (struct iphdr *)packet;
		struct icmphdr *icmp_header = (struct icmphdr *)(packet + ip_header->ihl * 4);

		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
				PACKET_SIZE, ip_address, packet->sequence_number + 1, ip_header->ttl, elapsed_time);
			success = 1;
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
		}

		tries++;
	}

	close(sockfd);
	free(packet);

	return success ? 0 : 1;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <IP address>\n", argv[0]);
		return 1;
	}

	return (ping(argv[1]));
}
