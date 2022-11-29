// Client side C/C++ program to demonstrate Socket
// commanf format sample RECV:02 00 00 00 0C 00 00 AE 00 37 00 42 00 00 00 4D 03 
// OFFSET CMD x_o:55 y_o:66 num:77
// the answer format SEND:02 00 00 00 08 00 00 ae 00 00 00 01 03

#include <arpa/inet.h> // inet_addr()
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h> // bzero()
#include <cstring>
#include <sys/socket.h>
#include <unistd.h> 
#include <chrono>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>


#define SEMAPHORE_TCP_REQ_NAME "/tcp_request_semaphore"
#define SEMAPHORE_TCP_READY_NAME "/tcp_answer_semaphore"
sem_t *tcp_req_sem;
std::string sem_name; 
#define BUF_SIZE 8  // int16_t + int16_t + int32_t in bytes
#define SA struct sockaddr
#define HRC std::chrono::high_resolution_clock
#define LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("GR: %s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("GR: error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)

struct CRD {std::int16_t x; std::int16_t y; std::int32_t num;}; // keep coordinates in shared memory
union FLAG{std::uint8_t n; unsigned char  bytes[1];}; // represent 1 bytes field
union OFFSET{std::int16_t n;  unsigned char  bytes[2];}; // represent 2 bytes field
union NUM{std::uint32_t n;  unsigned char  bytes[4];}; // represent 4 bytes field 


int drop_semaphore(const char *sem_name, std::string *descr)
{
  sem_t *sem;
  std::cout << "TCP: Dropping semaphore " << *descr << " --> ";
  if ( (sem = sem_open(sem_name, 0)) == SEM_FAILED ) {
      perror("sem_open");
      return -1;
  }
  sem_post(sem);
  std::cout<<"Semaphore droped" << std::endl;
  return 0;
}

int chat(int sockfd,CRD* crd)
{
    union OFFSET X;
    union OFFSET Y;
    union NUM NUM;
    union FLAG MsgID;
    union NUM NumBytes;
    X.n=crd->x;
    Y.n=crd->y;
    NUM.n=crd->num;
    unsigned char STX[1]={0x02};    
    NumBytes.n=0x0C;
    unsigned char Checksum[1]={0x00};
    unsigned char SeqNum[1]={0x00};
    MsgID.n=0xAE;
    unsigned char ETX[1]={0x03};
    unsigned char buff[17] = {  STX[0], 
                                NumBytes.bytes[3], NumBytes.bytes[2],NumBytes.bytes[1],NumBytes.bytes[0],
                                Checksum[0], 
                                SeqNum[0],
                                MsgID.bytes[0],
                                X.bytes[1], X.bytes[0],
                                Y.bytes[1], Y.bytes[0],
                                NUM.bytes[3], NUM.bytes[2], NUM.bytes[1], NUM.bytes[0],
                                ETX[0]};
    unsigned char returned_buff[13] = {0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0xAE, 0x00, 0x00, 0x00, 0x01, 0x03};
   
    write(sockfd, buff, sizeof(buff));
    bzero(buff, sizeof(buff));
    //usleep(50);    
    read(sockfd, buff, sizeof(buff));
    if((buff[7]==returned_buff[7]) && (buff[11]==returned_buff[11])) return 0; 
    else return -1;
}

int  change_options_by_commandline(int argc, char *argv[], std::string *ipadress, int *tcpport, int *loops, int *testmode)
{
	int  opt;

	while((opt =  getopt(argc, argv, "a:p:n:t:")) != -1)
	{
		switch(opt)
		{
			default:
				printf("_______________________________________________________________________________\n");
				printf("                                                                               \n");
				printf("  Usage: %s [-a ip adress] [-p tcp port] [-n loops number] [-t mode (1=test)]\n", argv[0]);
				printf("                                                                               \n");
				printf("_______________________________________________________________________________\n");
				printf("                                                                               \n");
				return(+1);
			case 'a':  *ipadress  = std::string(optarg);  std::cout<< "TCP: Printer ip adress set to: " << *ipadress << std::endl;  break;
			case 'p':  *tcpport   = atol(optarg);  printf("TCP: Setting PORT to %d.\n",   *tcpport   );  break;
			case 'n':  *loops     = atol(optarg);  printf("TCP: Setting loops number to %d.\n",   *loops  ); break;
			case 't':  *testmode  = atol(optarg);  printf("TCP: Setting test mode to %d.\n",   *testmode  ); break;
			
		}
	}

	if(argc<2)
	{
		printf("TC: Incorrect command line option (see:  %s -? )\n", argv[0]);
	}

	return(0);
}

int main(int argc, char **argv)
{
    int optPORT = 5001;
    std::string optIP = "127.0.0.1";
    int loopsn = 10000;
    int optTestMode = 0;
    int ret;

    ret =  change_options_by_commandline(argc, argv, &optIP, &optPORT, &loopsn, &optTestMode);
    ASSERT(ret==0);
    const char* optAdress = optIP.c_str();

    int sockfd;
    struct sockaddr_in servaddr;
    // statistic collectors:  
    double mintime=100000.0;
    double maxtime=0.0;
    double chatime=0.0;   
    int drop0 =0;
    int drop1 =0;
    int drop2 =0;
    int drop3 =0;
    int drop10 =0;
    int drop20 =0;
    int drop30 =0;
    
     // ftok to generate unique key for shared memory
    key_t key = ftok("coordinatesmemory",67);
    // shmget returns an identifier in shmid
    int shmid;
    if (optTestMode == 0)
        shmid = shmget(key, BUF_SIZE,0666|IPC_EXCL);
    
    else
        shmid = shmget(key, BUF_SIZE,0666|IPC_CREAT);
    
    if(shmid == -1)
    {
         perror("TCP: Create the Shared Memory Segment");
         return -1;
    }
    if (sem_close(tcp_req_sem)!=0)
    {
        perror("TCP: Semaphore is checked");        
    } 
    //create semaphore to catch signal from ql    
    if ( (tcp_req_sem = sem_open(SEMAPHORE_TCP_REQ_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) 
    {
        perror("TCP: Semaphore for tcp request open");
        return -1;
    } 
    // shmaid attach to shared memory -> new shared memory addres points to the new buf structure array [IMAGE_SIZE]
    CRD* crdbuf = (CRD*)shmat(shmid, (void*)0, 0);    

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("TCP: Socket creation failed...\n");
        return -1;
    }
    else
        printf("TCP: Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(optAdress);
    servaddr.sin_port = htons(optPORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("TCP: Connection with the server failed...\n");
        return -1;
    }
    else
        printf("TCP: Connected to the server..\n");
        
   


    for(int i=0;i<loopsn;i++)
    {   
        if (optTestMode == 0)
        {
            //active wait tcp_req_sem semaphore drop from ql process
            if (sem_wait(tcp_req_sem)!=0){perror("TCP: tcp_req_sem wait");}
        }
        else{
           usleep(40000); // emulate conveyor period 40ms 
        }
        HRC::time_point begin = HRC::now();
        // function for chat with printer
        int ret;
        ret = chat(sockfd, crdbuf);
        if (ret==0){
            printf("TCP: Successfully send coordinates: x = %d, y = %d, num = %d \n",crdbuf->x, crdbuf->y, crdbuf->num);
            if (optTestMode == 0){
            // send "ready for ql to next chat with printer" IPC signal 
            sem_name = "TCP_READY";
            if(drop_semaphore(SEMAPHORE_TCP_READY_NAME,&sem_name)!=0){return -1;}
            } 
        }
        else
            { 
                drop0++;
                std::cout <<"TCP: Response failed\n";
            }
      
        HRC::time_point end = HRC::now();
        chatime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
        std::cout.width(40);
        std::cout << std::left << "TCP: Session chat time:" << chatime << "[μs]" << std::endl;
        mintime = chatime<mintime?chatime:mintime;
        maxtime = chatime>maxtime?chatime:maxtime;
        if (chatime>1000){drop1++;}
        if (chatime>2000){drop2++;}
        if (chatime>3000){drop3++;}
        if (chatime>10000){drop10++;}
        if (chatime>20000){drop20++;}
        if (chatime>30000){drop30++;}
    }
    // close the socket
    close(sockfd);
    std::cout << std::endl;
    std::cout << "---------- TCP report start ----------" << std::endl;
    std::cout << "TCP: Amount of session loops_: " << loopsn << std::endl;
    std::cout << "TCP: Number of fault sessions: " << drop0 << std::endl;
    std::cout << "TCP: Delays > 1ms________: " << drop1 << std::endl;
    std::cout << "TCP: Delays > 2ms________: " << drop2 << std::endl;
    std::cout << "TCP: Delays > 3ms________: " << drop3 << std::endl;
    std::cout << "TCP: Delays > 10ms_______: " << drop10 << std::endl;
    std::cout << "TCP: Delays > 20ms_______: " << drop20 << std::endl;
    std::cout << "TCP: Delays > 30ms_______: " << drop30 << std::endl;
    std::cout << "TCP: Session minimum time: " << mintime << "[μs]" << std::endl;
    std::cout << "TCP: Session maximum time: " << maxtime << "[μs]" << std::endl;
    std::cout << "---------- TCP report end ----------" << std::endl << std::endl;

    // detach from shared memory
    //shmdt(buf); 
    if (shmdt(crdbuf)!=0)
    {
        perror("TCP: coordinates buffer detach");
        return -1;
    }  
    // destroy the shared memory
    shmctl(shmid,IPC_RMID,NULL);
   
    if (sem_close(tcp_req_sem)!=0)
    {
        perror("TCP: tcp_req_sem close");
        return -1;
    }
  
    return 0;
}
