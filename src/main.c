#include "../include/ft_ping.h"

t_ping ft_ping;

void handle_sigint(int signal)
{
	(void)signal;
	if (ft_ping.tries > 0)
	{
		if (ft_ping.num_success > 1)
		{
			double mean_rtt = ft_ping.total_rtt / ft_ping.num_success;
			double sum_squared_diff = 0.0;
			for (int i = 0; i < ft_ping.num_success; i++)
			{
				double diff = ft_ping.rtt[i] - mean_rtt;
				sum_squared_diff += diff * diff;
			}
			double variance_rtt = sum_squared_diff / (double)ft_ping.num_success;
			ft_ping.stddev_rtt = sqrt(variance_rtt);
		}
		else
			ft_ping.stddev_rtt = 0.0;

		printf("--- %s ping statistics ---\n", ft_ping.ip_address);
		printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
			ft_ping.tries, ft_ping.num_success, (ft_ping.tries - ft_ping.num_success) * 100 / ft_ping.tries);
		if (ft_ping.num_success > 0)
			printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ft_ping.min_rtt, ft_ping.total_rtt / ft_ping.num_success, ft_ping.max_rtt, ft_ping.stddev_rtt / ft_ping.num_success);
	}
	close(ft_ping.sockfd);
	free(ft_ping.rtt);
	exit(0);
}


int ping(const char* ip_address)
{
	uint8_t buffer[PACKET_SIZE];

	ft_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ft_ping.sockfd < 0)
	{
		perror("socket");
		free(ft_ping.rtt);
		return 1;
	}
	int ttl = 64;
	if (setsockopt(ft_ping.sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		perror("setsockopt");
		close(ft_ping.sockfd);
		free(ft_ping.rtt);
		return 1;
	}

	memset(&ft_ping.target_addr, 0, sizeof(ft_ping.target_addr));
	ft_ping.target_addr.sin_family = AF_INET;
	ft_ping.target_addr.sin_addr.s_addr = inet_addr(ip_address);


	signal(SIGALRM, handle_alarm);
	signal(SIGINT, handle_sigint);
	if (ft_ping.verbose == 1)
	{
		printf("PING %s (%s): %lu data bytes, id 0x%x = %u\n", ip_address, ip_address, PACKET_SIZE - sizeof(struct icmphdr), ft_ping.pid, ft_ping.pid);
	}
	else
		printf("PING %s (%s): %lu data bytes\n", ip_address, ip_address, PACKET_SIZE - sizeof(struct icmphdr));
	while (ft_ping.tries < ft_ping.num_pings)
	{
		memset(&ft_ping.packet, 0, sizeof(ft_ping.packet));
		ft_ping.packet.header.type = ICMP_ECHO;
		ft_ping.packet.header.code = 0;
		ft_ping.packet.header.un.echo.id = ft_ping.pid;
		ft_ping.packet.header.un.echo.sequence = htons(ft_ping.tries);
		gettimeofday(&ft_ping.start_time, NULL);
		memset(ft_ping.packet.data, 0xA5, PACKET_SIZE - sizeof(struct icmphdr));
		ft_ping.packet.header.checksum = calculate_checksum(&ft_ping.packet, sizeof(ft_ping.packet));

		// Send the ICMP packet to the target address
		if (sendto(ft_ping.sockfd, &ft_ping.packet, sizeof(ft_ping.packet), 0, (struct sockaddr *)&ft_ping.target_addr, sizeof(ft_ping.target_addr)) < 0)
		{
			perror("sendto");
			close(ft_ping.sockfd);
			free(ft_ping.rtt);
			return 1;
		}

		alarm(TIMEOUT);

		// Wait for an ICMP packet to be received
		if (recvfrom(ft_ping.sockfd, buffer, PACKET_SIZE, 0, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", ft_ping.tries);
				ft_ping.tries++;
				ft_ping.num_failures++;
				continue;
			}
			else
			{
				perror("recvfrom");
				close(ft_ping.sockfd);
				free(ft_ping.rtt);
				return 1;
			}
		}

		struct iphdr *ip_header = (struct iphdr *)buffer;
		struct icmphdr *icmp_header = (struct icmphdr *)(buffer + sizeof(struct iphdr));

		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			ft_ping.rtt[ft_ping.num_success] = ft_ping.elapsed_time;
			gettimeofday(&ft_ping.end_time, NULL);
			ft_ping.elapsed_time = get_elapsed_time(&ft_ping.start_time, &ft_ping.end_time);
			ft_ping.num_success++;
			ft_ping.total_rtt += ft_ping.elapsed_time;
			if (ft_ping.elapsed_time < ft_ping.min_rtt)
				ft_ping.min_rtt = ft_ping.elapsed_time;
			if (ft_ping.elapsed_time > ft_ping.max_rtt)
				ft_ping.max_rtt = ft_ping.elapsed_time;
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				PACKET_SIZE, ip_address, ft_ping.tries, ip_header->ttl, ft_ping.elapsed_time);
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
			ft_ping.num_failures++;
		}

		ft_ping.tries++;
		usleep(1000000);
	}

	double mean_rtt = ft_ping.total_rtt / ft_ping.num_success;
	double sum_squared_diff = 0.0;
	for (int i = 0; i < ft_ping.num_success; i++)
	{
		double diff = ft_ping.rtt[i] - mean_rtt;
		sum_squared_diff += diff * diff;
	}
	double variance_rtt = sum_squared_diff / (double)ft_ping.num_success;
	ft_ping.stddev_rtt = sqrt(variance_rtt);

	// Print ping statistics
	printf("--- %s ping statistics ---\n", ip_address);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		ft_ping.tries, ft_ping.num_success, (ft_ping.tries - ft_ping.num_success) * 100 / ft_ping.tries);
	if (ft_ping.num_success > 0)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ft_ping.min_rtt, ft_ping.total_rtt / ft_ping.num_success, ft_ping.max_rtt, ft_ping.stddev_rtt / ft_ping.num_success);

	close(ft_ping.sockfd);
	free(ft_ping.rtt);
	if (ft_ping.num_success > 0)
		return 0;
	else
		return 1;
}

/*
	TODO:
		- handle FQDN
		- handle options:
			-v -?
		- BONUS:
			-f -l -n -w -W -p -r -s -T -ttl -ip-timestamp

*/

void parse_one_arg(char *arg)
{
	if (ft_strcmp(arg, "-?") == 0)
		ft_ping.help = 1;
	else if (ft_strcmp(arg, "-v") == 0)
		ft_ping.verbose = 1;
	else
		ft_ping.ip_address = arg;
}

void	parse_args(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (ft_strcmp(argv[i], "-v") == 0)
			ft_ping.verbose = 1;
		else if (ft_strcmp(argv[i], "-?") == 0)
			ft_ping.help = 1;
		else if (argv[i][0] != '-')
			ft_ping.ip_address = argv[i];
		else
		{
			printf("ft_ping: invalid value ('%c')\n", argv[i][1]);
			printf("Try 'ft_ping -?' for more information.\n");
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}

	ft_ping.verbose = 0;
	ft_ping.help = 0;
	ft_ping.ip_address = NULL;

	if (argc == 2)
		parse_one_arg(argv[1]);
	else
		parse_args(argc, argv);


	if (ft_ping.help == 1)
	{
		printf("Usage: %s [options] <destination>\n", argv[0]);
		printf("Send ICMP ECHO_REQUEST packets to network hosts\n\n");
		printf(" Options:\n");
		printf("  -v\t\tverbose output\n");
		printf("  -?\t\tgive this help list\n");
		printf("\n");
		return (0);
	}

	ft_ping.pid = htons(getpid());
	ft_ping.num_pings = 5;
	ft_ping.min_rtt = DBL_MAX;
	ft_ping.tries = 0;
	ft_ping.num_success = 0;
	ft_ping.num_failures = 0;
	ft_ping.ip_address = argv[1];
	ft_ping.rtt = malloc(ft_ping.num_pings * sizeof(double));

	return (ping(ft_ping.ip_address));
}
