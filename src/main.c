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
	int sockfd; // Socket file descriptor
	struct sockaddr_in target_addr; // Target IP address
	struct icmp_packet *s_packet, *r_packet; // ICMP packet
	struct timeval start_time, end_time; // Start and end time of the ping
	float elapsed_time; // Elapsed time in milliseconds
	int tries = 0; // Number of tries
	int num_pings = 5; // Number of pings
	int num_success = 0; // Number of successful pings

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		return 1;
	}

	memset(&target_addr, 0, sizeof(target_addr));
	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = inet_addr(ip_address);

	s_packet = malloc(sizeof(struct icmp_packet) + PACKET_SIZE - 1);
	if (s_packet == NULL)
	{
		fprintf(stderr, "Error: malloc() failed\n");
		close(sockfd);
		return 1;
	}

	s_packet->sequence_number = 0;
	s_packet->code = 0;
	s_packet->identifier = getpid() & 0xFFFF;
	s_packet->type = 8;


	r_packet = malloc(sizeof(struct icmp_packet) + PACKET_SIZE - 1);
	if (r_packet == NULL)
	{
		fprintf(stderr, "Error: malloc() failed\n");
		close(sockfd);
		free(s_packet);
		return 1;
	}

	signal(SIGALRM, handle_alarm);

	while (tries < num_pings)
	{
		s_packet->checksum = 0;
		gettimeofday(&s_packet->timestamp, NULL);
		gettimeofday(&start_time, NULL);
		memset(s_packet->data, 0xA5, PACKET_SIZE);
		s_packet->checksum = calculate_checksum(s_packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1);

		// Send the ICMP s_packet to the target address
		if (sendto(sockfd, s_packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
		{
			perror("sendto");
			close(sockfd);
			free(s_packet);
			free(r_packet);
			return 1;
		}

		// Debug: Print the sent packet
		//printf("Sent packet: type=%d code=%d id=%d seq=%d\n", s_packet->type, s_packet->code, s_packet->identifier, s_packet->sequence_number);

		alarm(TIMEOUT);


		// Wait for an ICMP packet to be received
		if (recvfrom(sockfd, r_packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1, 0, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", r_packet->sequence_number);
				tries++;
				continue;
			}
			else
			{
				perror("recvfrom");
				close(sockfd);
				free(s_packet);
				free(r_packet);
				return 1;
			}
		}

		// Debug: Print the received packet
		//printf("Received packet: type=%d code=%d id=%d seq=%d\n", r_packet->type, r_packet->code, r_packet->identifier, r_packet->sequence_number);

		gettimeofday(&end_time, NULL);
		elapsed_time = get_elapsed_time(start_time, end_time);

		struct iphdr *ip_header = (struct iphdr *)r_packet;
		struct icmphdr *icmp_header = (struct icmphdr *)(r_packet + ip_header->ihl * 4);

		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
				PACKET_SIZE, ip_address, s_packet->sequence_number, ip_header->ttl, elapsed_time);
			num_success++;
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
		}

		tries++;
		s_packet->sequence_number++;
		sleep(1);
	}

	close(sockfd);
	free(s_packet);
	free(r_packet);

	if (num_success > 0)
		return 0;
	else
		return 1;
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
