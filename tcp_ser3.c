/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd);                                                        // transmitting and receiving function

int main(void)
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;

//	char *buf;
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);                              //listen
	if (ret <0) {
		printf("error in listening");
		exit(1);
	}

	while (1)
	{
		printf("waiting for data\n");
		sin_size = sizeof (struct sockaddr_in);
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
		if (con_fd <0)
		{
			printf("error in accept\n");
			exit(1);
		}

		if ((pid = fork())==0)                                         // creat acception process
		{
			close(sockfd);
			str_ser(con_fd);                                          //receive packet and response
			close(con_fd);
			exit(0);
		}
		else close(con_fd);                                         //parent process
	}
	close(sockfd);
	exit(0);
}

void compareFile(){
	FILE *fp1, *fp2;
    int ch1, ch2;
	char fname1[] = "myfile.txt";
	char fname2[] = "myTCPreceive.txt";
	fp1 = fopen(fname1, "r");
	fp2 = fopen(fname2, "r");
	ch1 = getc(fp1);
	ch2 = getc(fp2);
	while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
		ch1 = getc(fp1);
		ch2 = getc(fp2);
	}
	if (ch1 == ch2)
		printf("Files are identical \n");
	else if (ch1 != ch2)
		printf("Files are NOT identical \n");

	fclose(fp1);
	fclose(fp2);
}

void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	struct pack_so pack;
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	long len=0;
	end = 0;
	
	printf("receiving data!\n");
	srand(time(NULL));

	while(!end)
	{
		if ((n= recv(sockfd, &pack, sizeof(pack), 0))==-1)                                   //receive the packet
		{
			printf("ser: error when receiving\n");
			exit(1);
		}
		if (pack.data[pack.len-1] == '\0')									//if it is the end of the file
		{
			end = 1;
			pack.len--;
		}
		if (ack.num == pack.num) //normal
		{
			memcpy((buf+lseek), pack.data, pack.len);
			lseek += pack.len;
		} 
		else // client is resending, discard duplicate frame
		{
			if(end) //TODO: wont reach here
				lseek -= ack.len - 1;
			else
				lseek -= ack.len;
			memcpy((buf+lseek), pack.data, pack.len);
			lseek += pack.len;
		}

		ack.num = !pack.num;
		if(end)
			ack.len = pack.len + 1;
		else
			ack.len = pack.len;

		int r = rand() % 100 + 1;
		int prevAckLen = ack.len;
		if(end != 1)
		{
			if(r <= ERROR_RATE)
			{
				printf("error!\n");
				ack.len = 255;
			}
		}

		if ((n = send(sockfd, &ack, sizeof(ack), 0))==-1)
		{
				printf("ser: send error!");								//send the ack
				exit(1);
		}
		ack.len = prevAckLen;
		len += sizeof(pack);
	}
	
	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)len);
	compareFile();
}
