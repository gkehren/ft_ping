#include "../include/ft_ping.h"

t_ping ft_ping;

/*
	TODO:
		- handle FQDN
		- BONUS:
			-f -l -n -w -W -p -r -s -T -ttl -ip-timestamp
*/

int parse_one_arg(char *arg)
{
	if (ft_strcmp(arg, "-?") == 0)
		ft_ping.help = 1;
	else if (ft_strcmp(arg, "-v") == 0)
		ft_ping.verbose = 1;
	else if (arg[0] != '-' && ft_ping.fqdn == NULL)
		ft_ping.fqdn = ft_strdup(arg);
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
	int i = 1;
	char **arg;
	while (i < argc)
	{
		arg = ft_split(argv[i], ' ');
		int j = 0;
		while (arg[j])
		{
			if (parse_one_arg(arg[j]) == 1)
			{
				ft_free_split(arg);
				free(ft_ping.ip_address);
				exit(1);
			}
			j++;
		}
		ft_free_split(arg);
		i++;
	}
}

int init_socket()
{
	ft_ping.pid = htons(getpid());
	ft_ping.num_pings = 5;
	ft_ping.min_rtt = DBL_MAX;
	ft_ping.tries = 0;
	ft_ping.ttl = 64;
	ft_ping.num_success = 0;
	ft_ping.num_failures = 0;
	ft_ping.rtt = malloc(ft_ping.num_pings * sizeof(double));

	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	struct addrinfo *result;
	int status = getaddrinfo(ft_ping.fqdn, NULL, &hints, &result);
	if (status != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
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
		free(ft_ping.rtt);
		return 1;
	}
	if (setsockopt(ft_ping.sockfd, IPPROTO_IP, IP_TTL, &ft_ping.ttl, sizeof(ft_ping.ttl)) < 0)
	{
		perror("setsockopt");
		close(ft_ping.sockfd);
		free(ft_ping.ip_address);
		free(ft_ping.rtt);
		return 1;
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

	if (init_socket() == 1)
		return (1);
	else
		return (ping());
}
