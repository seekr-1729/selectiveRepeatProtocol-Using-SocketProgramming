
/* server side for SR protocol...*/

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>

#define BUFLEN 512
#define PORT 12345 //port no. to connect...
#define P 20 //packet size...



//strcuture for home packet... 
typedef struct packet0{
	int window;
	int no_pck;
}HOME_PKT;

//structure for acknowledgement packet...
typedef struct packet1{
	int sq_no;
}ACK_PKT;

//structure for data packet...
typedef struct packet2{
	int sq_no;
	char data[P+1];
}DATA_PKT;

//to chekc error...
void die(char *s)
{
	perror(s);
	exit(1);
}


int main(void){

	//defining socket address and other attributes...
	struct sockaddr_in si_me, si_other;
	int s, slen = sizeof(si_other) , recv_len;

	DATA_PKT rcv_pkt;
	ACK_PKT ack_pkt;
	HOME_PKT home_pck;

	//creating socket...
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){die("socket");}

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//binding socket to port...
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){	die("bind");}

	FILE *p_file = fopen("out.txt","a"); 	


	int track[10000]={0};	
	int out=0;
	float drop;

	printf("enter drop rate:__ [for lossless enter 0 and for lossy enter value in (0.1,1)]\n");
	scanf("%f",&drop);

	//receiving window size from sender...
	if ((recv_len = recvfrom(s, &home_pck, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1){die("recvfrom()");}
	
	printf("received home_pkt and window size is:%d\n",home_pck.window);
	
	int index=0,l_value=0,r_value=(home_pck.window-1),base=0, flag = 0;
	printf("receiving file and set intial base: %d\n", base);	
	DATA_PKT pck[home_pck.no_pck];

	while(1)
	{	

		while(index >= l_value && index <= r_value ){

			if(!track[index]){
				//receiving packets from sender...
				if ((recv_len = recvfrom(s, &rcv_pkt, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)) == -1){die("recvfrom()");}
				if(rcv_pkt.sq_no == -999)
				{
					out=1;
					break;
				}

				track[index] = 1;
				pck[index] = rcv_pkt;

				//dropping packets according to drop rate...
				if( drand48()< drop ) track[index] = 0;
				ack_pkt.sq_no = rcv_pkt.sq_no;



				//sending ACK to sender...
				if(!track[index]){printf("packet %d is dropped!!!\n", ack_pkt.sq_no); ack_pkt.sq_no=-1;flag=1;}
				if((sendto(s, &ack_pkt, sizeof(ack_pkt),0,(struct sockaddr*)&si_other,slen))==-1){die("sendto()");}
				if(ack_pkt.sq_no >= 0)
				{

					printf("Base: %d Sent Acknowlegdement for %d: \n", base,ack_pkt.sq_no);
				}

			}


			//appending inorder packets to file...
			if(!flag){ l_value++;r_value++; base++;
				fprintf(p_file,"%s",pck[index].data);printf("packet %d: Appended \n",pck[index].sq_no);}	
			index++;

		}		
		if(out==1)
		{
			fclose(p_file);//closing file...
			break;
		}

		index = l_value; 
		flag = 0;

	}
	printf("file successfully received...\n");
	close(s);//closing socket...
	return 0;
}




