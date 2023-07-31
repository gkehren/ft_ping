#include "../include/ft_ping.h"

typedef struct s_packet
{
	struct iphdr	ip_header;
	struct icmphdr	header;
	struct timeval	timestamp;
	char			data[PACKET_SIZE];
}	t_packet;

int ping(const char* ip_address)
{
	int sockfd; // Socket file descriptor
	struct sockaddr_in target_addr; // Target IP address
	t_packet packet;
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

	signal(SIGALRM, handle_alarm);

	while (tries < num_pings)
	{
		//packet.code = 0;
		//packet.identifier = getpid() & 0xFFFF;
		//packet.type = 8;
		//packet.checksum = 0;
		//gettimeofday(&packet.timestamp, NULL);
		//memset(packet.data, 0xA5, PACKET_SIZE);
		//packet.checksum = calculate_checksum(&packet, sizeof(struct icmp_packet) + PACKET_SIZE - 1);

		//// Debug: Print the sent packet
		//printf("Sent packet: type=%d code=%d id=%d seq=%d\n", packet.type, packet.code, packet.identifier, packet.sequence_number);

		packet.ip_header.version = 4;
		packet.ip_header.ihl = 5;
		packet.ip_header.tos = 0;
		packet.ip_header.tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + PACKET_SIZE;
		packet.ip_header.id = htons(getpid());
		packet.ip_header.frag_off = 0;
		packet.ip_header.ttl = 64;
		packet.ip_header.protocol = IPPROTO_ICMP;
		packet.ip_header.check = 0;
		packet.ip_header.daddr = target_addr.sin_addr.s_addr;

		packet.header.type = ICMP_ECHO;
		packet.header.code = 0;
		packet.header.un.echo.id = htons(getpid());
		packet.header.un.echo.sequence = htons(tries);
		packet.header.checksum = 0;
		gettimeofday(&packet.timestamp, NULL);
		memset(packet.data, 0xA5, PACKET_SIZE);
		packet.header.checksum = calculate_checksum(&packet, sizeof(packet));

		// Send the ICMP packet to the target address
		if (sendto(sockfd, &packet, sizeof(packet) + PACKET_SIZE - 1, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
		{
			perror("sendto");
			close(sockfd);
			return 1;
		}


		alarm(TIMEOUT);

		// Wait for an ICMP packet to be received
		if (recvfrom(sockfd, &packet, sizeof(packet) + PACKET_SIZE - 1, 0, NULL, NULL) < 0)
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

		// Debug: Print the received packet
		//printf("Received packet: type=%d code=%d id=%d seq=%d\n", packet.type, packet.code, packet.identifier, packet.sequence_number);

		gettimeofday(&end_time, NULL);

		//struct iphdr *ip_header = (struct iphdr *)&packet;
		//struct icmphdr *icmp_header = (struct icmphdr *)((char *)ip_header + ip_header->ihl * 4);
		struct timeval *sent_time = (struct timeval *)((char *)&packet.header + sizeof(struct icmphdr));

		elapsed_time = get_elapsed_time(sent_time, &end_time);

		if (packet.header.type == ICMP_ECHOREPLY)
		{
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
				PACKET_SIZE, ip_address, tries, tries, elapsed_time);
			num_success++;
		}
		else
		{
			printf("Received ICMP packet with type %d\n", packet.header.type);
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
