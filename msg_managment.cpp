/*
	Krzysztof Bednarek
	292974
*/

#include "msg_managment.h"


int sockfd;

void init()
{
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE);
	}
}

u_int16_t compute_icmp_checksum (const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t* ptr = (const u_int16_t*)buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}


double diff_time(timeval b, timeval e)
	{return (double)(e.tv_usec - b.tv_usec)/1000 + (double)(e.tv_sec - b.tv_sec)*1000;}


/********************************************************/
/*********************PRINTING***************************/
/********************************************************/

void print_as_bytes (unsigned char* buff, ssize_t length)
{
	for (ssize_t i = 0; i < length; i++, buff++)
		printf ("%.2x ", *buff);	
}

void print_address(u_int32_t address)
{
	char address_str[20]; 
	inet_ntop(AF_INET, &(address), address_str, sizeof(address_str));
	printf("%-16s", address_str);
}


/********************************************************/
/******************GETING PARTS OF MSG*******************/
/********************************************************/

struct iphdr* get_ip_header(msg_t* msg)
	{return (struct iphdr*) msg->buffer;}

struct icmphdr* get_icmphdr(msg_t* msg)
	{return (struct icmphdr*) ((char*)msg->buffer + 4*get_ip_header(msg)->ihl);}

struct icmp* get_icmp(msg_t* msg)
	{return (struct icmp*) (get_icmphdr(msg) + 1);}

struct iphdr* get_inner_ip_header(msg_t* msg)
	{return (struct iphdr*) ((char*)get_icmphdr(msg) + sizeof(icmphdr));}

struct icmp* get_inner_icmp(msg_t* msg)
{
	struct iphdr* iph = get_inner_ip_header(msg);
	return (struct icmp*) ((char*)iph + 4*iph->ihl);
}

u_int32_t get_sender(msg_t* msg)
	{return get_ip_header(msg)->saddr;}


/********************************************************/
/*********************RECIVING MSG***********************/
/********************************************************/

bool check_msg(msg_t* msg, int nmb, int id)
{
	struct iphdr* iph = get_ip_header(msg); 
	if(iph->protocol != 1)
		return false;
	struct icmphdr* icmph = get_icmphdr(msg);
	
	if(icmph->type == ICMP_TIME_EXCEEDED)
		return icmph->code == ICMP_NET_UNREACH 
				&& ntohs(get_inner_icmp(msg)->icmp_id) == getpid() 
				&& ntohs(get_inner_icmp(msg)->icmp_seq) >= nmb*id
				&& ntohs(get_inner_icmp(msg)->icmp_seq) <= nmb*(id+1)-1;
	else if(icmph->type == ICMP_ECHOREPLY)
		return ntohs(get_icmphdr(msg)->un.echo.id) == getpid()
				&& ntohs(get_icmphdr(msg)->un.echo.sequence) >= nmb*id 
				&& ntohs(get_icmphdr(msg)->un.echo.sequence) <= nmb*(id+1)-1;

	return false;
	
	
	printf("%p %p\n", msg->buffer, get_icmp(msg));
	return true;
}

msg_t recive_msg()
{
	msg_t msg; 
	msg.address_len = sizeof(msg.address);

	msg.packet_len = recvfrom (sockfd, msg.buffer, IP_MAXPACKET, 0, 
							(struct sockaddr*)&msg.address, &msg.address_len);
	gettimeofday(&msg.time, NULL);
	if (msg.packet_len < 0) {
		fprintf(stderr, "recvfrom error: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE);
	}
	return msg;
}

int recive_msgs(msg_t* msgs, int nmb, int id)
{
	fd_set descriptors;
	FD_ZERO (&descriptors);
	FD_SET (sockfd, &descriptors);
	struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;

	struct timeval beg;
	struct timeval end;
	gettimeofday(&beg, NULL);

	int count=0;
	do
	{
		int ready;
		do
		{
			gettimeofday(&end, NULL);
		}
		while((ready = select (sockfd+1, &descriptors, NULL, NULL, &tv)) == 0 
				&& diff_time(beg, end) < 1000);

		if(ready < 0 || diff_time(beg, end) > 1000)
			return -1;

		msgs[count] = recive_msg();

		if(check_msg(&msgs[count], nmb, id))
			count++;
		gettimeofday(&end, NULL);
	}
	while(diff_time(beg, end) < 1000 && count < nmb);

	return count;
}


/********************************************************/
/*********************SENDING MSG************************/
/********************************************************/


void send_msg(char* address, int msg_id, socklen_t ttl, timeval* time)
{
	struct icmphdr icmp_header;
	icmp_header.type = ICMP_ECHO;
	icmp_header.code = 0;
	icmp_header.un.echo.id = htons(getpid());
	icmp_header.un.echo.sequence = htons(msg_id);
	icmp_header.checksum = 0;
	icmp_header.checksum = compute_icmp_checksum((u_int16_t*)&icmp_header, sizeof(icmp_header));

	struct sockaddr_in recipient;
	bzero (&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
	inet_pton(AF_INET, address, &recipient.sin_addr);

	setsockopt (sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

	ssize_t bytes_sent = sendto (
		sockfd,
		&icmp_header,
		sizeof(icmp_header),
		0,
		(struct sockaddr*)&recipient,
		sizeof(recipient)
	);
	gettimeofday(time, NULL);
	if (bytes_sent < 0) {
		fprintf(stderr, "recvfrom error: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE);
	}
}