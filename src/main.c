#include "../include/ft_ping.h"

typedef struct s_packet
{
	//struct iphdr	ip_header;
	struct icmphdr	header;
	//struct timeval	timestamp;
	char			data[PACKET_SIZE - sizeof(struct icmphdr)];
}	t_packet;

int ping(const char* ip_address)
{
	uint8_t buffer[MAX_PACKET_SIZE];
	int sockfd; // Socket file descriptor
	struct sockaddr_in target_addr; // Target IP address
	t_packet packet;
	struct timeval start_time, end_time; // Start and end time of the ping
	double elapsed_time; // Elapsed time in milliseconds
	int tries = 0; // Number of tries
	int num_pings = 5; // Number of pings
	int num_success = 0; // Number of successful pings

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		return 1;
	}
	int ttl = 64;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		perror("setsockopt");
		close(sockfd);
		return 1;
	}

	memset(&target_addr, 0, sizeof(target_addr));
	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = inet_addr(ip_address);


	signal(SIGALRM, handle_alarm);

	while (tries < num_pings)
	{
		memset(&packet, 0, sizeof(packet));
		packet.header.type = ICMP_ECHO;
		packet.header.code = 0;
		packet.header.un.echo.id = htons(getpid());
		packet.header.un.echo.sequence = htons(tries);
		gettimeofday(&start_time, NULL);
		//memset(packet.data, 0xA5, PACKET_SIZE - sizeof(struct icmphdr));
		packet.header.checksum = calculate_checksum(&packet, sizeof(packet));

		//printf("send PING %s (%s) %d(%lu) bytes of data.\n", ip_address, ip_address, PACKET_SIZE, PACKET_SIZE + sizeof(struct icmphdr) + sizeof(struct iphdr));

		// Send the ICMP packet to the target address
		if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
		{
			perror("sendto");
			close(sockfd);
			return 1;
		}

		alarm(TIMEOUT);

		// Wait for an ICMP packet to be received
		if (recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", tries);
				tries++;
				continue;
			}
			else
			{
				perror("recvfrom");
				close(sockfd);
				return 1;
			}
		}

		struct iphdr *ip_header = (struct iphdr *)buffer;
		struct icmphdr *icmp_header = (struct icmphdr *)(buffer + sizeof(struct iphdr));

		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			gettimeofday(&end_time, NULL);
			elapsed_time = get_elapsed_time(&start_time, &end_time);
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				PACKET_SIZE, ip_address, tries, ip_header->ttl, elapsed_time);
			num_success++;
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
		}

		tries++;
		//packet.sequence_number++;
		sleep(1);
	}

	close(sockfd);
	(void)start_time;
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
