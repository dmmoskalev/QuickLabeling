#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/videodev2.h>
#include <getopt.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <chrono>
#include <iostream>


#define SEMAPHORE_CAP_NAME "/cap_semaphore"
#define SEMAPHORE_READ_NAME "/read_semaphore"
sem_t *cap_sem;


#define hrc std::chrono::high_resolution_clock
#define u8 unsigned char
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
 
#define VIDEO_DEVICE "/dev/video0"
#define IMAGE_WIDTH 1280//sensor fixed output 1920*1080 image
#define IMAGE_HEIGHT 800
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT)
//#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT) +(IMAGE_WIDTH*IMAGE_HEIGHT)/4
//#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 1.25)
//#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 2)
//#define BUFFER_COUNT 5//Apply for 5 buffers
#define BUFFER_COUNT 3//


#define  DEMO_NAME          "mipi camera OV9281 capture"
#define  DEMO_MAINVERSION    (  0)  /**<  Main Version: X.-.-   */
#define  DEMO_VERSION        (  0)  /**<       Version: -.X.-   */
#define  DEMO_SUBVERSION     (  3)  /**<    Subversion: -.-.X   */
 
int cam_fd = -1;
struct v4l2_buffer video_buffer[BUFFER_COUNT];
u8* video_buffer_ptr[BUFFER_COUNT];
struct BUF {u8 array[IMAGE_SIZE];};
std::string sem_name;

 
int cam_open()
{
    cam_fd = open(VIDEO_DEVICE, O_RDWR);//Turn on the camera
 
    if (cam_fd >= 0) return 0;
    else return -1;
}
 
int cam_close()
{
    close(cam_fd);//turn off the camera
 
    return 0;
}
 
int cam_select(int index)
{
    int ret;
 
    int input = index;
    ret = ioctl(cam_fd, VIDIOC_S_INPUT, &input);//set input source
    return ret;
}
 
int cam_init()
{
    uint i;
    int ret;
    struct v4l2_format format;
 
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//frame type, for video capture devices
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;//8 bit gray
    format.fmt.pix.width = IMAGE_WIDTH;//Resolution
    format.fmt.pix.height = IMAGE_HEIGHT;
    ret = ioctl(cam_fd, VIDIOC_TRY_FMT, &format);//try to set the desire format
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_TRY_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
 
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_S_FMT, &format);//if ok - set current format
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_S_FMT) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
 
    struct v4l2_requestbuffers req;
    req.count = BUFFER_COUNT;//number of buffered frames
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//Buffered frame data format
    req.memory = V4L2_MEMORY_MMAP;//memory mapping
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);//application buffer
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_REQBUFS) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    DBG("req.count: %d", req.count);
    if (req.count < BUFFER_COUNT) //the driver will attempt to allocate the requested number of 
      //buffers and it stores the actual number allocated in the count field. 
      //It can be smaller than the number requested, even zero, when the driver runs out of free memory
    {
        DBG("request buffer failed");
        return ret;
    }
 
    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof(buffer)); //Fill a region of memory started at &buffer with the "0" value
    buffer.type = req.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    for (i=0; i<req.count; i++)
    {
        buffer.index = i;
        ret = ioctl (cam_fd, VIDIOC_QUERYBUF, &buffer);//Get buffer frame address
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QUERYBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
        DBG("buffer.length: %d", buffer.length);
        DBG("buffer.m.offset: %d", buffer.m.offset);
        video_buffer_ptr[i] = (u8*) mmap(NULL, buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, cam_fd, buffer.m.offset);//Map the buffer to the any free memory addr in the application’s address space
        if (video_buffer_ptr[i] == MAP_FAILED)
        {
            DBG("mmap() failed %d(%s)", errno, strerror(errno));
            return -1;
        }
 
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);//put the buffered frame into the queue
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
    }
 
    int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(cam_fd, VIDIOC_STREAMON, &buffer_type);//start data flow
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_STREAMON) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
 
    DBG("cam init done.");
 
    return 0;
}
 
int cam_get_image(BUF* out_buffer, int out_buffer_size)
{
    int i, ret;
    struct v4l2_buffer buffer;
        
    memset(&buffer, 0, sizeof(buffer));
    
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = BUFFER_COUNT;
    
    for (i=0;i<BUFFER_COUNT;i++)
    {
        hrc::time_point begin = hrc::now();
        ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);//dequeue a filled (capturing) or displayed (output) buffer from the driver’s outgoing queue
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_DQBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
     
        if (buffer.index < 0 || buffer.index >= BUFFER_COUNT)
        {
            DBG("invalid buffer index: %d", buffer.index);
            return ret;
        }
     
        //DBG("dequeue done, index: %d", buffer.index);
        hrc::time_point end = hrc::now();
        std::cout << "GR: Dequeue elapsed time ("<< buffer.index <<")____________: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
        if (i == (BUFFER_COUNT-1))
        {
            //Buffered frame data video_buffer_ptr[buffer.index] is copied to out_buffer
            begin = hrc::now();
            memcpy(out_buffer, video_buffer_ptr[buffer.index], out_buffer_size);
            //DBG("copy done.");    
            end = hrc::now();
            std::cout << "GR: Buffer copy elapsed time ("<< buffer.index <<")________: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;  
        }
        begin = hrc::now();
        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);//enqueue an empty (capturing) or filled (output) buffer in the driver’s incoming queue
        if (ret != 0)
        {
            DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
            return ret;
        }
        //DBG("enqueue done.");
        end = hrc::now();
        std::cout << "GR: Enqueue elapsed time ("<< buffer.index <<")____________: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
    }
        
    return 0;
}

int cam_get_triggered_image(BUF* out_buffer, int out_buffer_size)
{
    int ret;
    struct v4l2_buffer buffer;
        
    memset(&buffer, 0, sizeof(buffer));
    
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = BUFFER_COUNT;
    
    hrc::time_point begin = hrc::now();
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);//dequeue a filled (capturing) or displayed (output) buffer from the driver’s outgoing queue
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_DQBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    
    if (buffer.index < 0 || buffer.index >= BUFFER_COUNT)
    {
    DBG("invalid buffer index: %d", buffer.index);
    return ret;
    }  
    
    //DBG("dequeue done, index: %d", buffer.index);
    hrc::time_point end = hrc::now();
    std::cout.width(40);
    std::cout << std::left <<"GR: Dequeue elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
    
    //Buffered frame data video_buffer_ptr[buffer.index] is copied to out_buffer
    begin = hrc::now();
    memcpy(out_buffer, video_buffer_ptr[buffer.index], out_buffer_size);
    //DBG("copy done.");    
    end = hrc::now();
    std::cout.width(40);
    std::cout << std::left << "GR: Buffer copy elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;  
    
    begin = hrc::now();
    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);//enqueue an empty (capturing) or filled (output) buffer in the driver’s incoming queue
    if (ret != 0)
    {
        DBG("ioctl(VIDIOC_QBUF) failed %d(%s)", errno, strerror(errno));
        return ret;
    }
    //DBG("enqueue done.");
    end = hrc::now();
    std::cout.width(40);
    std::cout << std::left << "GR: Enqueue elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
    

    return 0;
}
//drop semaphor to indicate the buffer readiness for reading
int drop_semaphore(const char *sem_name, std::string *descr)
{
  sem_t *sem;
  std::cout << "QL: Dropping semaphore " << *descr << " --> ";
  if ( (sem = sem_open(sem_name, 0)) == SEM_FAILED ) {
      perror("sem_open");
      return -1;
  }
  sem_post(sem);
  std::cout<<"Semaphore droped" << std::endl;
  return 0;
}

//to free memory
void memory_free(void)
{
  int i,ret;
  for (i=0; i<BUFFER_COUNT; i++)
  {
     ret=munmap(video_buffer_ptr[i],IMAGE_SIZE);
     if (ret != 0)
      { DBG("Munmap failed!!."); }
     else
      {  DBG("Munmap Success!!.");}
     //free();
  }
}

int  change_options_by_commandline(int argc, char *argv[], int *shutter, float *gain, int *hflip, int *vflip, int *capcnt, int *wm)
{
	int  opt;

	while((opt =  getopt(argc, argv, "g:s:h:v:c:w:")) != -1)
	{
		switch(opt)
		{
			default:
				printf("_______________________________________________________________________________\n");
				printf("                                                                               \n");
				printf("  %s v.%d.%d.%d.\n", DEMO_NAME, DEMO_MAINVERSION, DEMO_VERSION, DEMO_SUBVERSION);
				printf("  -----------------------------------------------------------------------------\n");
				printf("                                                                               \n");
				printf("  Usage: %s [-s sh] [-g gain] [-h f] [-v f] [-c cnt] [-w write]\n", argv[0]);
				printf("                                                                               \n");
				printf("  -s,  Shutter Time                                                            \n");
				printf("  -g,  Gain Value                                                              \n");
				printf("  -h,  horizen flip                                                            \n");
				printf("  -v,  vertical flip                                                           \n");
				printf("  -c,  capture count                                                           \n");
                printf("  -w,  write output images to .raw files (0 - no, 1 -yes)                      \n");
                printf("_______________________________________________________________________________\n");
				printf("                                                                               \n");
				return(+1);
			case 's':  *shutter    = atol(optarg);  printf("GR: Setting Shutter Value to %d.\n",*shutter);  break;
			case 'g':  *gain       = atof(optarg);  printf("GR: Setting Gain Value to %f.\n",   *gain   );  break;
			case 'h':  *hflip      = atol(optarg);  printf("GR: Horizen flip the captured image.\n");       break;
			case 'v':  *vflip      = atol(optarg);  printf("GR: Vertical flip the captured image.\n" );     break;
			case 'c':  *capcnt     = atol(optarg);  printf("GR: Capture %d frame.\n",*capcnt );             break;
            case 'w':  *wm         = atol(optarg);  printf("GR: Setting save mode to %d.\n", *wm);          break;
			
		}
	}

	if(argc<2)
	{
		printf("GR: Incorrect command line option (see:  %s -? )\n", argv[0]);
	}

	return(0);
}

int  sensor_set_parameters(int optGain, int optShutter,int opthflip,int optvflip)
{
  int    ee, rc, target;
  unsigned int    ctlID, val;
  char   a10cTarget[11];
  struct v4l2_control  ctl;


  for(target= 0; target< 4; target++)
  //for(target= 0; target< 2; target++)
  {
    switch(target)
    {
      case 0:  sprintf(a10cTarget,"Gain"    );  ctlID = V4L2_CID_GAIN;      val = optGain;     break;
      case 1:  sprintf(a10cTarget,"Exposure");  ctlID = V4L2_CID_EXPOSURE;  val = optShutter;  break;    
      case 2:  sprintf(a10cTarget,"Hflip");     ctlID = V4L2_CID_HFLIP;     val = opthflip;  break;    
      case 3:  sprintf(a10cTarget,"Vflip");     ctlID = V4L2_CID_VFLIP;     val = optvflip;  break;
    }

    // Only needed for debugging: Get old value.
    //if((target!=2)&&(target!=3))
    {
      memset(&ctl, 0, sizeof(ctl));
      ctl.id = ctlID;

      rc =  ioctl(cam_fd , VIDIOC_G_CTRL, &ctl);
      if(rc<0)
      {
        if(EINVAL!=errno){ee=-1; goto fail;} //general error.
        else             {ee=-2; goto fail;} //unsupported.
      }

      //syslog(LOG_DEBUG, "%s():  Old %s Value: %d.\n", __FUNCTION__, a10cTarget, ctl.value);
      printf( "GR: %s():  Old %s Value: %d.\n", __FUNCTION__, a10cTarget, ctl.value);
    }

    // Set new value.
    //if((target!=2)&&(target!=3))
    {
      memset(&ctl, 0, sizeof(ctl));
      ctl.id    = ctlID;
      ctl.value = val;
      rc = ioctl(cam_fd , VIDIOC_S_CTRL, &ctl);
      if(rc<0)
      {
        if((EINVAL!=errno)&&(ERANGE!=errno)){ee=-3; goto fail;} //general error.
        else                                {ee=-4; goto fail;} //Value out of Range Error.
      }

      //syslog(LOG_DEBUG, "%s():  Requested New %s Value: %d.\n", __FUNCTION__, a10cTarget, val);
      printf( "GR: %s():  Requested New %s Value: %d.\n", __FUNCTION__, a10cTarget, val);
    }

    // Only needed for debugging: Get new value.
    //if((target!=2)&&(target!=3))
    {
      memset(&ctl, 0, sizeof(ctl));
      ctl.id = ctlID;

      rc =  ioctl(cam_fd , VIDIOC_G_CTRL, &ctl);
      if(rc<0)
      {
        if(EINVAL!=errno){ee=-5; goto fail;} //general error.
        else             {ee=-6; goto fail;} //unsupported.
      }

      //syslog(LOG_DEBUG, "%s():  New %s Value: %d.\n", __FUNCTION__, a10cTarget, ctl.value);
      printf( "GR: %s():  New %s Value: %d.\n", __FUNCTION__, a10cTarget, ctl.value);
    }
  }


  ee = 0;
fail:
  switch(ee)
  {
    case 0:
      break;
    case -1:
    case -3:
    case -5:
      //syslog(LOG_ERR, "%s():  ioctl(%s) throws Error (%d(%s))!\n", __FUNCTION__, (-3==ee)?("VIDIOC_S_CTRL"):("VIDIOC_G_CTRL"), errno, strerror(errno));
      printf( "GR: %s():  ioctl(%s) throws Error (%d(%s))!\n", __FUNCTION__, (-3==ee)?("VIDIOC_S_CTRL"):("VIDIOC_G_CTRL"), errno, strerror(errno));
      break;
    case -2:
    case -6:
      //syslog(LOG_ERR, "%s():  V4L2_CID_.. is unsupported!\n", __FUNCTION__);
      printf( "GR: %s():  V4L2_CID_.. is unsupported!\n", __FUNCTION__);
      break;
    case -4:
      //syslog(LOG_ERR, "%s():  %s Value is out of range (or V4L2_CID_.. is invalid)!\n", __FUNCTION__, a10cTarget);
      printf( "GR: %s():  %s Value is out of range (or V4L2_CID_.. is invalid)!\n", __FUNCTION__, a10cTarget);
      break;
  }

  return(ee);
}


 

int  main(int argc, char *argv[])
{
    int    j = 0;
    int    ret = 0;
    int    optShutter = 8721;
    int    opthflip = 1;
    int    optvflip = 1;
    int    optcapcnt = 80000; // TODO: in production should be unlimited
    int    optSaveMode = 0; // 0 - no need to write to file, 1 - write to file
    float  optGain = 30;
       
    // ftok to generate unique key for shared memory
    key_t key = ftok("capmemory",67);
    // shmget returns an identifier in shmid
    //int shmid = shmget(key, IMAGE_SIZE,0666|IPC_CREAT);
    int shmid = shmget(key, IMAGE_SIZE,0666|IPC_EXCL);
    if(shmid == -1)
    {
         perror("GR: create the Shared Memory Segment");
         return 1;
    }
    if (sem_close(cap_sem)!=0)
    {
        perror("GR: semaphore is checked");        
    } 
    //create semaphore to catch signal from ql    
    if ( (cap_sem = sem_open(SEMAPHORE_CAP_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) 
    {
        perror("GR: semaphore for capturing open");
        return -1;
    } 
    // shmaid attach to shared memory -> new shared memory addres points to the new buf structure array [IMAGE_SIZE]
    BUF* buf = (BUF*)shmat(shmid, (void*)0, 0);    

    ret =  change_options_by_commandline(argc, argv, &optShutter, &optGain, &opthflip, &optvflip, &optcapcnt, &optSaveMode);
    ASSERT(ret==0);

    ret = cam_open();
    if(ret!=0){
        std::cout<< "GR:  [ERROR] Check camera connection" << std::endl;
    }
    ASSERT(ret==0);

    ret =  sensor_set_parameters( optGain, optShutter,opthflip, optvflip);
    ASSERT(ret==0);

    ret = cam_select(0);
    ASSERT(ret==0);
 
    ret = cam_init();
    ASSERT(ret==0); 

    int count = 0;
    for(j=0;j<optcapcnt;j++)
    {
        std::cout << "GR: |||||||||||||||--[" << j << "]--|||||||||||||||" << std::endl;        
        hrc::time_point begin = hrc::now();
        //ret = cam_get_image(buf, IMAGE_SIZE);       
        ret = cam_get_triggered_image(buf, IMAGE_SIZE);
        ASSERT(ret==0); 
        // send "image ready" IPC signal 
        sem_name = "IMAGE_READY";
        if(drop_semaphore(SEMAPHORE_READ_NAME, &sem_name)!=0){return -1;}       
        hrc::time_point end = hrc::now();
        std::cout.width(40);        
        std::cout << std::left << "GR: Get triggered image time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
                

        if ((optSaveMode>0)  && (j<500)) // limit the number of output file to 500
        {             
            begin = hrc::now();
            char filename[32];
            sprintf(filename, "./%05d.raw", count++);
            int fd = open(filename,O_WRONLY|O_CREAT,00700);//save image data
            if (fd >= 0)
            {
                write(fd, buf, IMAGE_SIZE);
                close(fd);
            }
            else
            {
                LOGD("GR: open() failed: %d(%s)", errno, strerror(errno));
            }
            end = hrc::now();
            std::cout << "GR: Write .raw file time____: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl << std::endl;
        }
        if (sem_wait(cap_sem)!=0){perror("cap_wait");}

    }
   
    memory_free();
    ret = cam_close();
    ASSERT(ret==0);
    
    if (sem_close(cap_sem)!=0)
    {
        perror("QL: cap_sem_close");
        return -1;
    }
    
    if (shmdt(buf)!=0)
    {
        perror("GR: buffer detach");
        return -1;
    }
    
    // destroy the shared memory
    if (shmctl(shmid,IPC_RMID,NULL)!=0)
    {
        perror("GR: destroy the shared memory");
        return -1;
    }
    
    return 0;
}
