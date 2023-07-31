#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define TTL_VALUE 64

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <IP address>\n", argv[0]);
		return 1;
	}

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		return 1;
	}

	int ttl = TTL_VALUE;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		perror("setsockopt");
		return 1;
	}

	close(sockfd);
}
