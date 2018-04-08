/*
	Krzysztof Bednarek
	292974
*/

#include "msg_managment.h"



int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "recvfrom error: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE);
	}


	init();


	char address[20];
	strcpy(address, argv[1]);

	printf("traceroute to: %s\n", address);

	u_int32_t to_who;	
	inet_pton(AF_INET, address, &to_who);
	for(int i = 0; i<30; i++)
	{
		u_int32_t senders[3];
		timeval beg_times[3];
		msg_t msgs[3];
		printf("%d:\t", i+1);
		for(int j=0; j<3; j++)
		{
			send_msg(address, i*3+j, i+1, &beg_times[j]);
		}

		int successes  = recive_msgs(msgs, 3, i);
		for(int j=0; j<successes; j++)
			senders[j] = get_sender(&msgs[j]);


		if(successes > 0)
		{
			print_address(senders[0]);
			if(successes > 1 && senders[0] != senders[1])
			{
				printf(", ");
				print_address(senders[1]);
			}	
			if(successes > 2 && senders[0] != senders[2] && senders[1] != senders[2])
			{
				printf(", ");
				print_address(senders[2]);
			}	
			if(successes == 3)
				printf("%.3lfms\n", diff_time(beg_times[0], msgs[0].time)+ 
					diff_time(beg_times[1], msgs[1].time)+
					diff_time(beg_times[2], msgs[2].time)/3);
			else
				printf("???\n");
		}
		else
		{
			printf("*\n");
		}

		if(successes > 2 && senders[0] == senders[2] && senders[0] == senders[1] && to_who == senders[0])
			break;
	}

	return EXIT_SUCCESS;

}