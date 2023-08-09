#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdbool.h>

# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <string.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
#include <sys/time.h>

# define PKT_SIZE 64
# define MAX_TTL 64
# define PING_SLEEP_RATE 1
# define RECV_TIMEOUT 1

typedef struct s_opt
{
	int verbose;
	int help;
	int err;
	char *hostname;
} opt_t;

typedef struct	s_pkt
{
// if macos
# if defined(__APPLE__) || defined(__MACH__)
	struct icmp		hdr; // !! struct icmphdr under linux and icmp under mac
	char			hdr_buf[PKT_SIZE - sizeof(struct icmp)];
# elif defined(__linux__)
	struct icmphdr	hdr;
	char			hdr_buf[PKT_SIZE - sizeof(struct icmphdr)];
# endif
}				t_pkt;

typedef struct	s_res
{
	struct iovec		iov[1];
	struct msghdr		ret_hdr;
}		t_res;

typedef struct s_env
{
	char *hostname; // Hostname (e.g. google.com) or IP address
	char addrstr[INET_ADDRSTRLEN]; // IP address as string
	struct addrinfo		hints, *res;
	pid_t pid;

	int sockfd;
	int ttl;
	t_pkt pkt;
	unsigned int seq;

	t_res response;

	struct timeval start, end, diff;
}				env_t;

static bool g_running[2];

opt_t parse_opt(int ac, char **av);
int handle_opt(opt_t opt);

/**
 * Utils 
**/
void check_root();
unsigned short	checksum(unsigned short *data, int len);
int ft_strcmp(char *s1, char *s2);

#endif