#include <stdio.h>
#include <iostream>
#include <chrono>
#include <getopt.h>
#include <errno.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/gpio.h>

#define u8 unsigned char
#define coutf std::cout.width(40);std::cout << std::left
#define HRC std::chrono::high_resolution_clock
#define GPIO_DEV_NAME "/dev/gpiochip0"
#define LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("%s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)


static void gpio_write(const char *dev_name, int offset, uint8_t value)
{
    struct gpiohandle_request rq;
    struct gpiohandle_data data;
    int fd, ret;
    //printf("Write value %d to GPIO at offset %d (OUTPUT mode) on chip %s\n", value, offset, dev_name);
    fd = open(dev_name, O_RDONLY);
    if (fd < 0)
    {
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
        return;
    }
    rq.lineoffsets[0] = offset;
    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
    rq.lines = 1;
    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
    close(fd);
    if (ret == -1)
    {
        printf("Unable to line handle from ioctl : %s", strerror(errno));
        return;
    }
    data.values[0] = value;
    ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if (ret == -1)
    {
        printf("Unable to set line value using ioctl : %s", strerror(errno));
    }
    else
    {
         usleep(100);
    }
    close(rq.fd);
}



int  change_options_by_commandline(int argc, char *argv[], int *period, int *ln, int *itime, int *v)
{
	int  opt;

	while((opt =  getopt(argc, argv, "p:n:t:v:")) != -1)
	{
		switch(opt)
		{
			default:
				printf("  -----------------------------------------------------------------------------\n");
				printf("                                                                               \n");
				printf("  Usage: %s [-p period] [-n loops number] \n", argv[0]);
				printf("                                                                               \n");
				printf("  -p,  Period Time (milliseconds, default 40).                                 \n");
                printf("  -n,  loops number(default 80000)                                             \n");
                printf("  -t,  Impuls active time (microseconds, default 200)                          \n");
                printf("  -v,  verbose mode (1- on, 0- off)                                            \n");
				printf("_______________________________________________________________________________\n");
				printf("                                                                               \n");
				return(+1);
                case 'p':  *period    = atol(optarg);  printf("TR: Setting Period to %d milliseconds\n",*period );  break;
                case 'n':  *ln        = atol(optarg);  printf("TR: Setting loops number to %d \n",*ln );  break;
                case 't':  *itime     = atol(optarg);  printf("TR: Setting impulse active time to %d microseconds\n",*itime );  break;
                case 'v':  *v         = atol(optarg);  printf("TR: Verbose mode %d \n",*v );  break;
             
		}
	}

	if(argc<2)
	{
		printf("  Hint: Incorrect command line option (see:  %s -? )\n", argv[0]);
	}

	return(0);
}


int main(int argc, char *argv[]) 
{
    int j, ret;
    int optPeriod = 40; // in milliseconds    
    int optLoopNumber = 80000;
    int optImpulsTime = 200; // in microseconds
    int optVerbose = 0;
        
    HRC::time_point beg;
    HRC::time_point end; 
    double impulsetime = 0;
    //double periodfact = 0;
    
    ret =  change_options_by_commandline(argc, argv, &optPeriod, &optLoopNumber, &optImpulsTime, &optVerbose);
    ASSERT(ret==0);
    
    //int gpio 11 output to 0 (17 is gpio offset for 11 pin)
    gpio_write(GPIO_DEV_NAME, 17, 0);
       
    for(j=0;j<optLoopNumber;j++)
    {            
            // run hardware trigget on pin 11 gpio RPi
            if (optVerbose) std::cout << "TR: П_П_П_" << j << "_ impuls period: " << optPeriod << "[ms] _" << optLoopNumber << "_П_П_П" << std::endl;  
            beg = HRC::now();
            
            gpio_write(GPIO_DEV_NAME, 17, 1);
            usleep(optImpulsTime);
            gpio_write(GPIO_DEV_NAME, 17, 0);
                        
            end = HRC::now();
            if (optVerbose) {coutf << "TR: (_П_) active impulse time:" << std::chrono::duration_cast<std::chrono::microseconds>(end - beg).count() << "[mcs]" << std::endl;}
                        
            impulsetime = (end - beg).count()/1000;      
            beg = HRC::now();       
            usleep((optPeriod*1000)-impulsetime); 
            end = HRC::now();            
            if (optVerbose) {coutf << std::left << "TR: (П___П) passive impulse time:" << std::chrono::duration_cast<std::chrono::microseconds>(end - beg).count() << "[mcs]" << std::endl;}
                                
    }   
    return 0;
}
    
