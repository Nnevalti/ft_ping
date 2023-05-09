#include "ft_ping.h"

void dns_lookup(char *hostname) {
	struct addrinfo hints, *res;
	struct sockaddr_in *addr;
	char addrstr[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
		fprintf(stderr, "ping: %s: Name or service not known\n", hostname);
		exit(1);
	}

	addr = (struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &(addr->sin_addr), addrstr, INET_ADDRSTRLEN);

	printf("PING %s (%s): 56 data bytes\n", hostname, addrstr);
	printf("IPv4 Address: %s\n", addrstr);

	freeaddrinfo(res);
}

int main(int ac, char **av)
{
	opt_t opt;
	if (getuid() != 0)
	{
		printf("Error: You must be root to run this program\n");
		return 1;
	}
	if (ac < 2)
	{
		printf("Usage: %s <hostname>\n", av[0]);
		return 1;
	}
	opt = parse_opt(ac, av);
	handle_opt(opt);
	dns_lookup(opt.hostname);

	return 0;
}