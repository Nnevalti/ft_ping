#include "ft_ping.h"

void dns_lookup(env_t *env) {
	int err;
	struct sockaddr_in *addr;

	memset(&env->hints, 0, sizeof(struct addrinfo));
	env->hints.ai_family = AF_INET; // IPv4
	env->hints.ai_socktype = SOCK_STREAM; // TCP protocol
	err = getaddrinfo(env->hostname, NULL, &env->hints, &env->res);
	if (err != 0) {
		fprintf(stderr, "ft_ping: cannot resolve %s: Unknown host\n", env->hostname);
		// fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(errno);
	}

	addr = (struct sockaddr_in *)env->res->ai_addr;
	inet_ntop(AF_INET, &(addr->sin_addr), env->addrstr, INET_ADDRSTRLEN);
}