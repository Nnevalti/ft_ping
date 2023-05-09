#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>

# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <string.h>
# include <netinet/in.h>

# include <netinet/ip_icmp.h>

# define PKT_SIZE 64

typedef struct s_opt
{
	int verbose;
	int help;
	int err;
	char *hostname;
} opt_t;

typedef struct	s_pkt
{
	char			hdr_buf[PKT_SIZE];
	struct iphdr		*ip; // !! struct iphdr under linux and ip under mac
	struct icmphdr		*hdr; // !! struct icmphdr under linux and icmp under mac
}				t_pkt;

opt_t parse_opt(int ac, char **av);
int handle_opt(opt_t opt);

/**
 * Utils 
**/
int ft_strcmp(char *s1, char *s2);

#endif