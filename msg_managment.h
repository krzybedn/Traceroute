/*
	Krzysztof Bednarek
	292974
*/

#ifndef MSG_MANAGMENT
#define MSG_MANAGMENT 1

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

struct msg_t
{
	struct sockaddr_in	address;
	socklen_t 			address_len;
	ssize_t				packet_len;
	unsigned char		buffer[IP_MAXPACKET];
	timeval				time;
};


void init();
int recive_msgs(msg_t* msgs, int nmb, int id);
void send_msg(char* address, int msg_id, socklen_t ttl, timeval* time);

u_int32_t get_sender(msg_t* msg);

struct iphdr* get_ip_header(msg_t* msg);
struct icmphdr* get_icmphdr(msg_t* msg);

void print_address(u_int32_t address);


double diff_time(timeval b, timeval e);









#endif