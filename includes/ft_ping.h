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
#include <errno.h>
#include <math.h>
#include <signal.h>

# define PKT_SIZE 64
# define MAX_TTL 64
# define PING_SLEEP_RATE 1
# define RECV_TIMEOUT 1

typedef struct s_opt
{
	unsigned int verbose: 1;
	unsigned int help: 1;
	unsigned int err: 1;
	char *hostname;
} opt_t;

typedef struct	s_pkt
{
// if macos
# if defined(__APPLE__) || defined(__MACH__)
	struct icmp		hdr;
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
	opt_t opt;

	char *hostname; // Hostname (e.g. google.com) or IP address
	char addrstr[INET_ADDRSTRLEN]; // IP address as string
	struct addrinfo		hints, *res;
	pid_t pid;

	int sockfd;
	int ttl;
	t_pkt pkt;
	unsigned int pkt_sent, pkt_recv;
	unsigned int seq;

	t_res response;

	struct timeval start, end, diff, send, receive;
	double min_rtt, max_rtt, avg_rtt, stddev_rtt;
	double *rtt;
}				env_t;

static bool g_running[2];

void exit_clean(env_t *env, char *msg);

/** opt.c **/
opt_t parse_opt(int ac, char **av);
int handle_opt(opt_t opt);
void dns_lookup(env_t *env);

/** init.c **/
void set_socket(env_t *env);
void init_send(env_t *env);
void init_recv(env_t *env);

/** print.c **/
void print_stats(env_t *env, unsigned int ret);
void print_errors(env_t *env);
void print_stats_rtt(env_t *env);
void print_final_stats(env_t *env);

/**
 * Utils 
**/
double ft_pow(int base, int exp);
double ft_sqrt(double x);
void check_root();
unsigned short	checksum(unsigned short *data, int len);
int ft_strcmp(char *s1, char *s2);

#endif