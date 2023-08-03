#include "../include/ft_ping.h"

typedef struct s_packet
{
	//struct iphdr	ip_header;
	struct icmphdr	header;
	//struct timeval	timestamp;
	char			data[PACKET_SIZE - sizeof(struct icmphdr)];
}	t_packet;

void handle_sigint(int signal)
{
	(void)signal;
	printf("\n--- %s ping statistics ---\n", "JESUISUNEADDRESSIP");
	exit(0);
}

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
	int num_failures = 0; // Number of failed pings
	double min_rtt = DBL_MAX; // Minimum round-trip time
	double max_rtt = 0; // Maximum round-trip time
	double total_rtt = 0; // Total round-trip time
	double stddev_rtt = 0; // Standard deviation of round-trip time
	double rtt[num_pings]; // Round-trip time for each ping

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
	signal(SIGINT, handle_sigint);

	printf("PING %s (%s): %lu data bytes\n", ip_address, ip_address, PACKET_SIZE - sizeof(struct icmphdr));
	while (tries < num_pings)
	{
		memset(&packet, 0, sizeof(packet));
		packet.header.type = ICMP_ECHO;
		packet.header.code = 0;
		packet.header.un.echo.id = htons(getpid());
		packet.header.un.echo.sequence = htons(tries);
		gettimeofday(&start_time, NULL);
		memset(packet.data, 0xA5, PACKET_SIZE - sizeof(struct icmphdr));
		packet.header.checksum = calculate_checksum(&packet, sizeof(packet));

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
				num_failures++;
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
			rtt[num_success] = elapsed_time;
			gettimeofday(&end_time, NULL);
			elapsed_time = get_elapsed_time(&start_time, &end_time);
			num_success++;
			total_rtt += elapsed_time;
			if (elapsed_time < min_rtt)
				min_rtt = elapsed_time;
			if (elapsed_time > max_rtt)
				max_rtt = elapsed_time;
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				PACKET_SIZE, ip_address, tries, ip_header->ttl, elapsed_time);
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
			num_failures++;
		}

		tries++;
		usleep(1000000);
	}

	double mean_rtt = total_rtt / num_success;
	double sum_squared_diff = 0.0;
	for (int i = 0; i < num_success; i++)
	{
		double diff = rtt[i] - mean_rtt;
		sum_squared_diff += diff * diff;
	}
	double variance_rtt = sum_squared_diff / (double)num_success;
	stddev_rtt = sqrt(variance_rtt);

	// Print ping statistics
	printf("--- %s ping statistics ---\n", ip_address);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		tries, num_success, (tries - num_success) * 100 / tries);
	if (num_success > 0)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", min_rtt, total_rtt / num_success, max_rtt, stddev_rtt / num_success);

	close(sockfd);
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
