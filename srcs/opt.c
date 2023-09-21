#include "ft_ping.h"

void print_help()
{
	printf("Usage: ping [-vh] <hostname>\n");
	printf("Options:\n");
	printf("  -v: verbose mode\n");
	printf("  -h: print this help message\n");
}

opt_t parse_opt(int ac, char **av, env_t *env)
{
	opt_t opt = {0, 0, 0, 0, 0, NULL};
	int i = 1;

	if (ac < 2)
	{
		fprintf(stderr, "Error: You must specify a hostname\n");
		print_help();
		exit(1);
	}

	while (i < ac)
	{
		if (ft_strcmp(av[i], "-v") == 0)
		{
			opt.verbose = 1;
		}
		else if (ft_strcmp(av[i], "-h") == 0)
		{
			opt.help = 1;
			return opt;
		}
		else if (ft_strcmp(av[i], "-a") == 0)
		{
			opt.audible = 1;
		}
		else if (ft_strcmp(av[i], "-t") == 0) {
			opt.ttl = 1;
			env->ttl = atoi(av[i + 1]);
			i++;
		}
		else if (av[i][0] != '-')
		{
			if (opt.hostname != NULL)
			{
				opt.err = 1;
				fprintf(stderr, "Error: You can only specify one hostname\n");
				return opt;
			}
			opt.hostname = av[i];
		}
		else
		{
			opt.err = 1;
			fprintf(stderr, "Invalid option: %s\n", av[i]);
			return opt;
		}
		i++;
	}
	return opt;
}

int handle_opt(opt_t opt)
{
	if (opt.err)
	{
		print_help();
		exit(1);
	}
	if (opt.help)
	{
		print_help();
		exit(0);
	}
	if (opt.verbose)
	{
		printf("verbose mode enabled\n");
	}
	if (opt.hostname == NULL)
	{
		fprintf(stderr, "Error: hostname is required\n");
		print_help();
		exit(1);
	}
	return 0;
}
