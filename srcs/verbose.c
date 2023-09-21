#include "ft_ping.h"

void check_icmp_errors(struct icmp *icmp)
{
	switch (icmp->icmp_type)
	{
	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		break;
	case ICMP_UNREACH:
		switch (icmp->icmp_code)
		{
		case ICMP_UNREACH_NET:
			printf("Destination Net Unreachable\n");
			break;
		case ICMP_UNREACH_HOST:
			printf("Destination Host Unreachable\n");
			break;
		case ICMP_UNREACH_PROTOCOL:
			printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_UNREACH_PORT:
			printf("Destination Port Unreachable\n");
			break;
		case ICMP_UNREACH_NEEDFRAG:
			printf("frag needed and DF set (MTU %d)\n",
				   ntohs(icmp->icmp_nextmtu));
			break;
		case ICMP_UNREACH_SRCFAIL:
			printf("Source Route Failed\n");
			break;
		case ICMP_UNREACH_FILTER_PROHIB:
			printf("Communication prohibited by filter\n");
			break;
		default:
			printf("Dest Unreachable, Bad Code: %d\n",
				   icmp->icmp_code);
			break;
		}
		break;
	case ICMP_SOURCEQUENCH:
		printf("Source Quench\n");
		break;
	case ICMP_REDIRECT:
		switch (icmp->icmp_code)
		{
		case ICMP_REDIRECT_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", icmp->icmp_code);
			break;
		}
		printf("(New addr: %s)\n", inet_ntoa(icmp->icmp_gwaddr));
		break;
	case ICMP_ECHO:
		printf("Echo Request\n");
		break;
	case ICMP_TIMXCEED:
		switch (icmp->icmp_code)
		{
		case ICMP_TIMXCEED_INTRANS:
			printf("Time to live exceeded\n");
			break;
		case ICMP_TIMXCEED_REASS:
			printf("Frag reassembly time exceeded\n");
			break;
		default:
			printf("Time exceeded, Bad Code: %d\n",
				   icmp->icmp_code);
			break;
		}
		break;
	case ICMP_TSTAMP:
		printf("Timestamp\n");
		break;
	case ICMP_TSTAMPREPLY:
		printf("Timestamp Reply\n");
		break;
	case ICMP_IREQ:
		printf("Information Request\n");
		break;
	case ICMP_IREQREPLY:
		printf("Information Reply\n");
		break;
	case ICMP_MASKREQ:
		printf("Address Mask Request\n");
		break;
	case ICMP_MASKREPLY:
		printf("Address Mask Reply\n");
		break;
	case ICMP_ROUTERADVERT:
		printf("Router Advertisement\n");
		break;
	case ICMP_ROUTERSOLICIT:
		printf("Router Solicitation\n");
		break;
	default:
		printf("Bad ICMP type: %d\n", icmp->icmp_type);
	}
}
