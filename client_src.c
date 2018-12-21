
/* client side for SR protocol...*/

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/time.h>

#define MAX_BUF_LEN 1000000 //maximum length of buffer to store content of file...

#define PORT 12345 //port no. to connect...
#define P 20 //packet size
#define WIN 5 //window size


//structure for home packet...
typedef struct packet0{
	int window;
	int no_pck;
}HOME_PKT;


//structure for acknowledgement packets...
typedef struct packet1{
	int sq_no;
}ACK_PKT;


//structure for data packets...
typedef struct packet2{
	int sq_no;
	char data[P+1];
}data_packet;


//structure for closing packet...
typedef struct packet3{
	int sq_no;
}CLOSE_PKT;


//to check erorrs...
void die(char *s)
{
	perror(s);
	exit(1);
}



int main(void)
{

	//defining socket addredd and other attributes...
	struct sockaddr_in si_other;
	int s, slen=sizeof(si_other),recv_len;

	data_packet send_pkt;
	ACK_PKT rcv_ack;
	HOME_PKT home_pkt;
	CLOSE_PKT close_pkt;


	//creating socket...
	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	//assigning address of receiver to sender socket and setting other attributes...
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.s_addr = inet_addr("127.0.0.1");


	int l_value=0,r_value=WIN-1;
	int index=0,base=0;
	int sent=0,flag = 0;

	//reading data from file and writing in buffer...
	char src[MAX_BUF_LEN + 1];
	FILE *fp = fopen("in.txt", "r");
	if (fp != NULL) {
		size_t new_len = fread(src, sizeof(char), MAX_BUF_LEN, fp);
		if ( ferror( fp ) != 0 ) {
			fputs("Error reading file", stderr);
		} else {
			src[new_len++] = '\0'; //to avoid any memory leak...
		}

		fclose(fp);
	}

	int file_len=strlen(src),pck_size=P,packet_cnt,c=0,pck_no=0,out=0;



	//calculating number of packets required to send...
	if(file_len%pck_size==0)
	{
		packet_cnt=(file_len/pck_size);
	}
	else
	{
		packet_cnt=(file_len/pck_size)+1;
	}

	data_packet packets[packet_cnt];



	//making packets from reader buffer...
	while(c<file_len)
	{
		packets[pck_no].sq_no=pck_no;

		for(int i=0;i<pck_size;i++)
		{
			if(c<file_len)
			{
				packets[pck_no].data[i]=src[c];
				c++;
			}
			else
			{
				packets[pck_no].data[i]='\0';
			}
		}
		packets[pck_no].data[pck_size]='\0';
		pck_no++;
	}	

	//intializing track array with 0...
	int track[10000]={0};	


	//sending window size...
	home_pkt.window=WIN;
	home_pkt.no_pck=packet_cnt;
	if((sendto(s, &home_pkt, sizeof(home_pkt),0,(struct sockaddr*)&si_other,slen))==-1){die("sendto()");}
	printf("Sent home Packet\n");

	printf("sending file and set intial base: %d \n", base);


	while(1)
	{	

		while(index >= l_value && index <= r_value ){

			//sending packets to server...
			if(!track[index]){
				send_pkt = packets[index];
				if((sendto(s, &send_pkt, sizeof(send_pkt),0,(struct sockaddr*)&si_other,slen))==-1){die("sendto()");}
				printf("Sent Packet :%d\n", send_pkt.sq_no);	


				sleep(2);
				recv_len=recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *) &si_other, &slen);
				track[index] = 1;
				if( rcv_ack.sq_no < 0 ){ flag =1;
					track[index] = 0;}
				else 	{ 
					/*receiving acknowledgement from server*/
					printf("base:%d and Received acknowledgement for packet: %d\n",base,rcv_ack.sq_no);
					if(base == packets[packet_cnt-1].sq_no)
					{
						out=1;
						break;
					}						
				}
			}
			if( !flag ){
				r_value++;l_value++; base++;}	
			index++;
			//t++;	
		}


		if(out==1)
		{
			fclose(fp);//closing file...
			break;
		}

		//timed out for lost packets...
		printf("timed out for packet: %d !!!\n", packets[l_value].sq_no);
		index = l_value;

		flag = 0; 

	}

	//sending closing packet...
	close_pkt.sq_no=-999;
	if((sendto(s, &close_pkt, sizeof(close_pkt),0,(struct sockaddr*)&si_other,slen))==-1){die("sendto()");}	

	printf("file sent successfully\n");
	close(s);//closing socket...
	return 0;
}




