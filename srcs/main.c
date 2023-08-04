#include "ft_ping.h"

void	sig_handler(int sig)
{
	(void)sig;
	g_running = false;
	printf("\n--- ft_ping statistics ---\n");
	return;
}

void dns_lookup(char *hostname) {
	struct addrinfo hints, *res;
	struct sockaddr_in *addr;
	char addrstr[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM;
	int err = getaddrinfo(hostname, NULL, &hints, &res);
	if (err != 0) {
		fprintf(stderr, "ping: %s: Name or service not known\n", hostname);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}

	addr = (struct sockaddr_in *)res->ai_addr;
	inet_ntop(AF_INET, &(addr->sin_addr), addrstr, INET_ADDRSTRLEN);

	printf("PING %s (%s): 56 data bytes\n", hostname, addrstr);

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
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	g_running = true;
	dns_lookup(opt.hostname);

	struct timeval tv;
	gettimeofday(&tv, NULL);
	while (g_running)
	{
		printf("ping\n");
		sleep(1);
	}
	return 0;
}