#include "../include/ft_ping.h"

t_ping ft_ping;

/*
	TODO:
		- BONUS:
			-f -l -n -w -W -p -r -s -T -ttl -ip-timestamp
*/

int	ft_atoi(const char *str)
{
	int	i;
	int	nbr;
	int	sign;

	i = 0;
	nbr = 0;
	sign = 1;
	while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] != '\0' && str[i] >= '0' && str[i] <= '9')
	{
		nbr = (nbr * 10) + str[i] - 48;
		i++;
	}
	return (nbr * sign);
}

int get_next_arg(char *arg)
{
	if (arg == NULL)
	{
		printf("ft_ping: option requires an argument -- '%c'\n", arg[1]);
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	int i = 0;
	while (arg[i])
	{
		if (arg[i] < '0' || arg[i] > '9')
		{
			printf("ft_ping: invalid value ('%s')\n", arg);
			return (1);
		}
		i++;
	}
	return (0);
}

int parse_one_arg(char *arg, char *next_arg)
{
	if (ft_strcmp(arg, "-?") == 0)
		ft_ping.help = 1;
	else if (ft_strcmp(arg, "-v") == 0)
		ft_ping.verbose = 1;
	else if (arg[0] != '-' && ft_ping.fqdn == NULL)
		ft_ping.fqdn = ft_strdup(arg);
	else if (ft_strcmp(arg, "-f") == 0)
		ft_ping.flood = 1;
	else if (ft_strcmp(arg, "-l") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		ft_ping.preload = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-n") == 0)
		ft_ping.numeric = 1;
	else if (ft_strcmp(arg, "-w") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		if (ft_atoi(next_arg) <= 0)
		{
			printf("ft_ping: option value too small: '%s'\n", next_arg);
			return (1);
		}
		ft_ping.w_timeout = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-W") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		if (ft_atoi(next_arg) <= 0)
		{
			printf("ft_ping: option value too small: '%s'\n", next_arg);
			return (1);
		}
		ft_ping.linger = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-p") == 0)
	{
		// need to handle -p differently because the next arg is a pattern
	}
	else if (ft_strcmp(arg, "-r") == 0)
		ft_ping.ignore_routing = 1;
	else if (ft_strcmp(arg, "-s") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		ft_ping.size_number = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-T") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		ft_ping.tos = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-ttl") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		ft_ping.ttl = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-ip-timestamp") == 0)
		ft_ping.ip_timestamp = 1;
	else if (arg[0] == '-')
	{
		printf("ft_ping: invalid value ('%c')\n", arg[1]);
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	return (0);
}

void	ft_free_split(char **arg)
{
	int i = 0;
	while (arg[i])
	{
		free(arg[i]);
		i++;
	}
	free(arg);
}

void	parse_args(int argc, char **argv)
{
	(void)argc;
	int i = 1;
	char **arg;

	arg = ft_split(argv, ' ');

	i = 0;
	while (arg[i])
	{
		if (parse_one_arg(arg[i], arg[i + 1]) == 1)
		{
			ft_free_split(arg);
			free(ft_ping.fqdn);
			exit(1);
		}
		i++;
	}
	ft_free_split(arg);
}

int init_socket()
{
	ft_ping.pid = htons(getpid());
	ft_ping.num_pings = 5;
	ft_ping.min_rtt = DBL_MAX;
	ft_ping.tries = 0;
	ft_ping.num_success = 0;
	ft_ping.num_failures = 0;
	ft_ping.rtt = malloc(sizeof(double));
	ft_ping.packet_size = ft_ping.size_number + 8;

	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	struct addrinfo *result;
	int status = getaddrinfo(ft_ping.fqdn, NULL, &hints, &result);
	if (status != 0)
	{
		printf("ft_ping: unknown host\n");
		free(ft_ping.ip_address);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		exit(1);
	}
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
	ft_ping.ip_address = ft_strdup(inet_ntoa(ipv4->sin_addr));
	freeaddrinfo(result);

	ft_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ft_ping.sockfd < 0)
	{
		perror("socket");
		free(ft_ping.ip_address);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		return 1;
	}
	if (setsockopt(ft_ping.sockfd, IPPROTO_IP, IP_TTL, &ft_ping.ttl, sizeof(ft_ping.ttl)) < 0)
	{
		perror("setsockopt");
		free(ft_ping.ip_address);
		free(ft_ping.packet->data);
		free(ft_ping.packet);
		close(ft_ping.sockfd);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		return 1;
	}
	if (ft_ping.linger > 0)
		ft_ping.timeout.tv_sec = ft_ping.linger;
	else
		ft_ping.timeout.tv_sec = 1;
	ft_ping.timeout.tv_usec = 0;
	if (setsockopt(ft_ping.sockfd, SOL_SOCKET, SO_RCVTIMEO, &ft_ping.timeout, sizeof(ft_ping.timeout)) < 0)
	{
		perror("setsockopt");
		free(ft_ping.ip_address);
		free(ft_ping.packet->data);
		free(ft_ping.packet);
		close(ft_ping.sockfd);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		return 1;
	}

	if (ft_ping.ignore_routing == 1)
	{
		int on = 1;
		if (setsockopt(ft_ping.sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
		{
			perror("setsockopt");
			free(ft_ping.ip_address);
			free(ft_ping.packet->data);
			free(ft_ping.packet);
			close(ft_ping.sockfd);
			free(ft_ping.fqdn);
			free(ft_ping.rtt);
			return 1;
		}
	}

	if (ft_ping.tos > 0)
	{
		if (setsockopt(ft_ping.sockfd, IPPROTO_IP, IP_TOS, &ft_ping.tos, sizeof(ft_ping.tos)) < 0)
		{
			perror("setsockopt");
			free(ft_ping.ip_address);
			free(ft_ping.packet->data);
			free(ft_ping.packet);
			close(ft_ping.sockfd);
			free(ft_ping.fqdn);
			free(ft_ping.rtt);
			return 1;
		}
	}

	memset(&ft_ping.target_addr, 0, sizeof(ft_ping.target_addr));
	ft_ping.target_addr.sin_family = AF_INET;
	ft_ping.target_addr.sin_addr.s_addr = inet_addr(ft_ping.ip_address);
	return (0);
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
	ft_ping.fqdn = NULL;
	ft_ping.ttl = 64;
	ft_ping.size_number = 56;
	ft_ping.packet_size = 64;
	ft_ping.linger = 0;
	ft_ping.w_timeout = 0;
	ft_ping.numeric = 0;
	ft_ping.ignore_routing = 0;
	ft_ping.tos = 0;
	ft_ping.pattern = 0xA;

	parse_args(argc, argv);

	if (ft_ping.help == 1)
	{
		printf("Usage: %s [options] <destination>\n", argv[0]);
		printf("Send ICMP ECHO_REQUEST packets to network hosts\n\n");
		printf(" Options:\n");
		printf("  -v\t\tverbose output\n");
		printf("  -?\t\tgive this help list\n");
		printf("\n");
		free(ft_ping.fqdn);
		return (0);
	}

	if (init_socket() == 1)
		return (1);
	else
		return (ping());
}
