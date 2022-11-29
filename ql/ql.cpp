//============================================================================
// Name        : quick labeling toolchain
// Author      : dmitry moskalev
// Version     : 1.0.0
// Copyright   : ask me to get it
// Description : quick labeling on base of openCV find contours tool
//============================================================================

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <getopt.h>
#include <dirent.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#define HRC std::chrono::high_resolution_clock
#define coutf std::cout.width(40);std::cout << std::left
#define VP std::vector<std::vector<cv::Point>>
#define V4i std::vector<cv::Vec4i>
#define IMAGE_WIDTH 1280//sensor fixed output 1280*800 image
#define IMAGE_HEIGHT 800
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT)
#define BUF_SIZE 8 // buffer for coordinates 
#define u8 unsigned char
#define SEMAPHORE_READ_NAME "/read_semaphore"
#define SEMAPHORE_CAP_NAME "/cap_semaphore"
#define SEMAPHORE_TCP_REQ_NAME "/tcp_request_semaphore"
#define SEMAPHORE_TCP_READY_NAME "/tcp_answer_semaphore"
#define SEMAPHORE_ACQ_NAME "/acq_semaphore"

struct BUF {u8 array[IMAGE_SIZE];};
struct CRD {std::int16_t x; std::int16_t y; std::int32_t num;}; // buffer for coordinates
BUF* buf; 
CRD* crdbuf;
int shmid,crdshmid;
sem_t *read_sem;
sem_t *tcp_ready_sem;
std::string sem_name;
#define LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while (0)
#define DBG(fmt, args...) LOGD("QL: %s:%d, " fmt, __FUNCTION__, __LINE__, ##args);
#define ASSERT(b) \
do \
{ \
    if (!(b)) \
    { \
        LOGD("QL: error on %s:%d", __FUNCTION__, __LINE__); \
        return 0; \
    } \
} while (0)

#define  NAME          "quick labeling"
#define  MAINVERSION    (  0)  /**<  Main Version: X.-.-   */
#define  VERSION        (  1)  /**<       Version: -.X.-   */
#define  SUBVERSION     (  1)  /**<    Subversion: -.-.X   */

struct Coordinates {
    int latch_X;
    int latch_Y;
    int circle_X;
    int circle_Y;
    int label_X;
    int label_Y;
    int radius;
};
struct ContourArgs{
    VP mask_contours;
    int threshold_value;
    int min_area;
    int max_area;
    double scale;
    double dist_in_loop;
    double min_moment;
    double max_moment;
    int radius;
};
int OPT_GRAPH = 0;
static void get_circle_crds(Coordinates *crd,
                                   const cv::Mat &input,
                                   ContourArgs *cnt_args)// find circle shape to detect the center
{
    cv::Mat thresh;
    VP contours;
    V4i hierarchy;
    int contour_index = 0;
    //double _distance = cnt_args->distance;
    double _distance = 1.2;
    double _dist_in_loop = cnt_args->dist_in_loop;
    //HRC::time_point begin = HRC::now();
    cv::threshold(input, thresh, cnt_args->threshold_value, 255, cv::THRESH_BINARY);
    cv::findContours(thresh, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE); // attention: RETR_EXTERNAL - retrieves only the extreme outer contours to save time
    std::vector<cv::Moments> cmu(contours.size());
    /*
    imshow("Thresholded image with contours", thresh);
    cv::waitKey(0);
    cv::destroyAllWindows();
    */
    for (size_t i = 0; i < contours.size(); i++)
    {
        cmu[i] = moments(contours[i]);
        if (cnt_args->min_area < cmu[i].m00 && cmu[i].m00 < cnt_args->max_area)
        {
            // detect distance between shapes of mask and just detected using I2 method
            _dist_in_loop = matchShapes(cnt_args->mask_contours[0], contours[i], cv::CONTOURS_MATCH_I2, 0);
            std::cout << "QL: Distance in loop = " << _dist_in_loop << " index = " << i << std::endl;
            if (_dist_in_loop < _distance)
            {
                crd->circle_X = int(cmu[i].m10 / cmu[i].m00);
                crd->circle_Y = int(cmu[i].m01 / cmu[i].m00);
                crd->radius = sqrt(cmu[i].m00 *113/355);
                _distance = _dist_in_loop;
                contour_index = i;
                cnt_args->min_moment = cmu[i].m00 < cnt_args->min_moment? cmu[i].m00 : cnt_args->min_moment;
                cnt_args->max_moment = cmu[i].m00 > cnt_args->max_moment? cmu[i].m00 : cnt_args->max_moment;
                coutf <<  "QL: Circle moment = " << cmu[i].m00 << std::endl;
                coutf <<  "QL: Circle radius = " << crd->radius << std::endl;
                coutf <<  "QL: circle x = " << crd->circle_X << std::endl;
                coutf <<  "QL: circle y = " << crd->circle_Y << std::endl;
            }
        }
    }
    std::vector<cv::Moments>().swap(cmu);
    //HRC::time_point end = HRC::now();
    //std::cout << "QL: Circle center x, y find time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    if (OPT_GRAPH){
        cv::drawContours(input, contours, contour_index, cv::Scalar(0, 255, 0), 2);
        cv::circle(input, cv::Point(crd->circle_X, crd->circle_Y), 7, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
        

    }
    
}

static void get_latch_crds(Coordinates *crd,
                                   const cv::Mat &input,
                                   ContourArgs *cnt_args)
{
    cv::Mat thresh;
    VP latch_contours;
    V4i latch_hierarchy;
    int contour_index = 0;
    //double _distance = cnt_args->distance;
    double _distance = 1.2;
    double _dist_in_loop = cnt_args->dist_in_loop;
    double angle = 80 * CV_PI / 180;
    double scale = cnt_args->scale;
    double alfa = scale * cos(angle);
    double betta = scale * sin(angle);

    //HRC::time_point begin = HRC::now();

    threshold(input, thresh, cnt_args->threshold_value, 255, cv::THRESH_BINARY);
    cv::findContours(thresh, latch_contours, latch_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
/*
    //tedst purpose only
    cv::imshow("Output image with contours", thresh);
    cv::waitKey(0);
    cv::destroyAllWindows();
*/
    std::vector<cv::Moments> mu(latch_contours.size());
    for (size_t i = 0; i < latch_contours.size(); i++)
    {
        mu[i] = moments(latch_contours[i]);
        if (cnt_args->min_area < mu[i].m00 && mu[i].m00 < cnt_args->max_area)
        {
            // detect distance between shapes of mask and just detected using I2 method
            _dist_in_loop = matchShapes(cnt_args->mask_contours[0], latch_contours[i], cv::CONTOURS_MATCH_I2, 0);
            std::cout << "QL: Distance in loop = " << _dist_in_loop << " index = " << i << std::endl;
            if (_dist_in_loop < _distance)
            {
                crd->latch_X = int(mu[i].m10 / mu[i].m00);
                crd->latch_Y = int(mu[i].m01 / mu[i].m00);
                _distance = _dist_in_loop;
                contour_index = i;
                crd->label_X = crd->latch_X * alfa + crd->latch_Y * betta + (1 - alfa) * crd->circle_X - betta * crd->circle_Y;
                crd->label_Y = crd->latch_Y * alfa - crd->latch_X * betta + betta * crd->circle_X + (1 - alfa) * crd->circle_Y;
                cnt_args->min_moment = mu[i].m00 < cnt_args->min_moment? mu[i].m00 : cnt_args->min_moment;
                cnt_args->max_moment = mu[i].m00 > cnt_args->max_moment? mu[i].m00 : cnt_args->max_moment;
                std::cout << "QL: Latch moment = " << mu[i].m00 << std::endl;
                std::cout << "QL: latch x = " << crd->latch_X << ", latch y = " << crd->latch_Y << std::endl;
                std::cout << "QL: label x = " << crd->label_X << ", label y = " << crd->label_Y << std::endl;


            }
        }
    }

    //HRC::time_point end = HRC::now();
    //std::cout << "QL: Latch crds find time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    if (OPT_GRAPH){
        cv::drawContours(input, latch_contours, contour_index, cv::Scalar(0, 255, 0), 2);
        cv::circle(input, cv::Point(crd->latch_X, crd->latch_Y), 7, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
        /*cv::putText(input, std::to_string(mu[contour_index].m00), cv::Point(crd->latch_X - 10, crd->latch_X - 10), cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(0, 0, 0), 2);*/

        cv::circle(input, cv::Point(crd->label_X, crd->label_Y), 7, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
        cv::putText(input, "[*]label", cv::Point(crd->label_X - 15, crd->label_Y - 15), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(0, 0, 0), 2);
    }
    std::vector<cv::Moments>().swap(mu);
   

}

int read_config(std::string *configfile)
{
    // std::ifstream is RAII, i.e. no need to call close
    std::ifstream cFile (const_cast<char*>(configfile->c_str()));
    if (cFile.is_open())
    {
        std::string line;
        while(getline(cFile, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
            if(line[0] == '#' || line.empty())
                continue;
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            std::cout << name << " " << value << '\n';
        }
        
    }
    else {
        std::cerr << "QL: Couldn't open config file for reading.\n";
        return -1;
    }
    return 0;
}

size_t files_in_directory (std::string *dir)
{
    DIR *dp;
    size_t i = 0;
    struct dirent *ep; 
    char* dirname = const_cast<char*>(dir->c_str());     
    dp = opendir (dirname);
    if (dp != NULL)
    {
        while ((ep = readdir (dp))){ i++;   }     
        (void) closedir (dp);    
    }
    else
    {
        std::cout << "QL: ERROR: Couldn't open the directory " << dirname << std::endl;
        return 0;
    }
    return i-2;
}

int  change_options_by_commandline(int argc, char *argv[], int *mode, int *graph,  
            size_t *loopn, int *period, std::string *dir, int *ct, int *cr, int *cmin, 
            int *cmax,int *lt, int *lmin, int *lmax, double *ls)
{
    int  opt;
    const char    * short_opt = "m:g:n:p:d:c:r:i:u:l:a:x:s:h";
    struct option   long_opt[] = 
    {
        {"mode",          required_argument, NULL, 'm'},
        {"gain",          required_argument, NULL, 'g'},
        {"loopnumber",    required_argument, NULL, 'n'},
        {"period",        required_argument, NULL, 'p'},
        {"dir",           required_argument, NULL, 'd'},
        {"ct",            required_argument, NULL, 'c'},
        {"cr",            required_argument, NULL, 'r'},
        {"cmin",          required_argument, NULL, 'i'},
        {"cmax",          required_argument, NULL, 'u'},
        {"lt",            required_argument, NULL, 'l'},
        {"lmin",          required_argument, NULL, 'a'},
        {"lmax",          required_argument, NULL, 'x'},
        {"ls  ",          required_argument, NULL, 's'},
        {"help",          no_argument,       NULL, 'h'},        
        {NULL,            0,                 NULL, 0  }
    };

    while((opt =  getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch(opt)
        {
            default:
                printf("_______________________________________________________________________________\n");
                printf("                                                                               \n");
                printf("  %s v.%d.%d.%d.\n", NAME, MAINVERSION, VERSION, SUBVERSION);
                printf("  -----------------------------------------------------------------------------\n");
                printf("                                                                               \n");
                printf("  Usage: %s [-m mode] [-g graph] [-n loop number]\n", argv[0]);
                printf("                                                                               \n");
                printf("  -m, --mode,  Mode (0- cane gallery, 1- cane live, 2- bottle gallery, 3- bottle live)  \n");
                printf("  -g, --gain,  Graphical output (0-no, 1-yes)                                   \n");
                printf("  -n, --loopnumber, Loop number                                                 \n");
                printf("  -p, --period, payload emulator delay in millisecond                           \n");
                printf("  -d, --dir,  directory with images files                                       \n");
                printf("  --ct,  circle threshold level                                                \n");
                printf("  --cr,  circle radius in micrometers                                          \n");
                printf("  --cmin,  circle minimum area level                                           \n");
                printf("  --cmax,  circle maximum area level                                           \n");
                printf("  --lt,  latch threshold level                                                 \n");
                printf("  --lmin,  latch minimum area level                                            \n");
                printf("  --lmax,  latch maximum area level                                            \n");
                printf("_______________________________________________________________________________\n");
                printf("                                                                               \n");
                return(+1);
            case 'm':  *mode    = atol(optarg);  printf("QL: Setting Mode Value to %d.\n",*mode);  break;
            case 'g':  *graph   = atol(optarg);  printf("QL: Setting Graphical output to %d.\n", *graph  );  break;
            case 'n':  *loopn   = atol(optarg);  printf("QL: Setting Loop number to %zu.\n", *loopn  );  break;
            case 'p':  *period  = atol(optarg);  printf("QL: Setting period delay to %d milliseconds\n", *period  );  break;
            case 'd':  *dir     = std::string(optarg);  std::cout << "QL: Images directory: " << *dir << std::endl;  break;
            case 'c':  *ct      = atol(optarg);  printf("QL: Setting circle threshold level to %d.\n", *ct  );  break;
            case 'r':  *cr      = atol(optarg);  printf("QL: Setting circle raduis to %dμm.\n", *cr  );  break;
            case 'i':  *cmin    = atol(optarg);  printf("QL: Setting circle minimum area level to %d.\n", *cmin  );  break;
            case 'u':  *cmax    = atol(optarg);  printf("QL: Setting circle maximum area level to %d.\n", *cmax  );  break;
            case 'l':  *lt      = atol(optarg);  printf("QL: Setting latch threshold level to %d.\n", *lt  );  break;
            case 'a':  *lmin    = atol(optarg);  printf("QL: Setting latch minimum area level to %d.\n", *lmin  );  break;
            case 'x':  *lmax    = atol(optarg);  printf("QL: Setting latch maximum area level to %d.\n", *lmax  );  break;
            case 's':  *ls      = atof(optarg);  printf("QL: Setting scale to %2f.\n", *ls  );  break;
            case 'h':
            printf("QL: Usage: %s [OPTIONS]\n", argv[0]);
            printf("  -d, --dir                  file\n");
            printf("  -h, --help                print this help and exit\n");
            printf("\n");
            return(0);

        }
    }

    if(argc<2)
    {
        printf("QL: Incorrect command line option (see:  %s -? )\n", argv[0]);
    }

    return(0);
    
}

const cv::Mat read_raw_image (std::string *file_path)
{
    
    //const char *file_path = filename;
    std::ifstream fin;
    // specify binary reading mode
    fin.open(*file_path, std::ios_base::binary);
    if (!fin) 
    {
        std::cerr << "QL: open failed: " << &file_path << std::endl;
        // load buffer
        char* buffer = new char[0];
        cv::Mat img_output(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_8UC1, buffer);    
        return img_output; // return empty mat
    }
    // seek function moves the tag to the end of the input stream
    fin.seekg(0, fin.end);
    // TELL will inform the entire input stream (from the beginning to the mark) number of bytes
    int length = fin.tellg();
    // move the mark to the start position of the stream
    fin.seekg(0, fin.beg);
    std::cout << *file_path << "QL: opened, length: " << length << std::endl;

    // load buffer
    char* buffer = new char[length];
    // read function Read (copy) LENGTH in the stream to buffer
    fin.read(buffer, length);

    // construct opencv mat
    cv::Mat img_output(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_8UC1, buffer);
    
    return img_output;
}

int gallery_mode(std::string *base, size_t *loopn, int *period, int *mode, std::string *dir, 
                ContourArgs *circle_args, ContourArgs *latch_args)
{
    int drops = 0;
    char fileindex[6];
    double totaltime = 0;
    double meantime = 0;
    double min_total_time = 100000;
    double max_total_time = 0;
    std::string problem_loop;
    cv::Mat blurred;
    Coordinates crd = {0,0,0,0,0,0,0};
    if(*mode==0){ // mode=0 gallery mode for aluminium cans, mode=2 -gallery mode for bottles
    //VP mask_latch_contours;
    V4i mask_latch_hierarchy;
    const std::string latchmaskfile = *base + "/images_masks/mask.bmp";
    cv::Mat mask = cv::imread(latchmaskfile, cv::IMREAD_GRAYSCALE);
    cv::findContours(mask, latch_args->mask_contours, mask_latch_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    }
    //VP mask_circle_contours;
    V4i mask_circle_hierarchy; 
    const std::string circlemaskfile = *base + "/images_masks/round_mask.bmp";
    cv::Mat mask_circle = cv::imread(circlemaskfile, cv::IMREAD_GRAYSCALE);
    cv::findContours(mask_circle, circle_args->mask_contours, mask_circle_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    
    std::cout << "QL: Performing " << *loopn << " iterations...\n" << std::flush;
    for (size_t k = 0; k < *loopn; k++)
    {
        std::cout <<"QL: |||||||||||||||| -> " << k << " <-|||||||||||||||" << std::endl;
        crd={0,0,0,0,0,0};
        HRC::time_point begin = HRC::now();

        sprintf(fileindex,"%05zu",k);
        std::string filename = *base + std::string("/") + *dir + std::string("/") + fileindex + std::string(".raw");         
        cv::Mat img_gray = read_raw_image(&filename);
        cv::GaussianBlur(img_gray, blurred, cv::Size(5, 5), 0, 0, cv::BORDER_DEFAULT);

        get_circle_crds(&crd,blurred,circle_args);
        if(*mode==0){ 
            if ((crd.circle_X > 0) & (crd.circle_Y > 0)){
                get_latch_crds(&crd,blurred,latch_args);
            }        
        }
        HRC::time_point end = HRC::now();
        std::cout << "QL: loop time___________:" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                << "[ms]" << std::endl << std::endl;
        totaltime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        if((totaltime>*period) | (crd.circle_X == 0) | (crd.circle_Y == 0) | (crd.label_X == 0) | (crd.label_Y == 0))
        {
            drops+=1;
            problem_loop += "loop #" + std::to_string(k) + " -> " + std::to_string(totaltime) + "ms\n";
        }
        else
        {
            meantime+=totaltime;
            min_total_time = totaltime < min_total_time? totaltime:min_total_time;
            usleep((*period-int(totaltime))*1000);
        }
        max_total_time = totaltime > max_total_time ? totaltime : max_total_time;
        if (OPT_GRAPH)
        {
            cv::putText(blurred, std::to_string(k), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX,
                    1.5, cv::Scalar(255, 255, 255), 3);
            imshow("Output image with contours", blurred);
            //cv::waitKey(*period);
            cv::waitKey(0);
        }        
    }

    meantime/=(*loopn-drops);
    std::cout << "QL: Done!" << std::endl;
    std::cout << "QL: Average for " << *loopn << " CPU runs: " << meantime << "ms" << std::endl;
    std::cout << "QL: Minimum CPU loop time_: " << min_total_time      << "ms" << std::endl;
    std::cout << "QL: Maximum CPU loop time_: " << max_total_time      << "ms" << std::endl;
    std::cout << "QL: Maximum circle area___: " << circle_args->max_moment      << std::endl;
    std::cout << "QL: Minimum circle area___: " << circle_args->min_moment      << std::endl;
    if(*mode==0){ 
    std::cout << "QL: Maximum latch area____: " << latch_args->max_moment      << std::endl;
    std::cout << "QL: Minimum latch area____: " << latch_args->min_moment      << std::endl;
    }
    std::cout << "QL: Drops_________________: "                 << drops       << std::endl;
    if (problem_loop.length()>3)
    {
        std::cout << "QL: Problem loops are____: "     << std::endl <<problem_loop.c_str() << std::endl;
    }
    cv::destroyAllWindows();
    return drops;
}


cv::Mat get_triggered_image(BUF* out_buffer)
{    
    // wait for semaphore signal from v4l2 grabber
    coutf << "QL: Active wait trigger event" << std::endl;
    
    if (sem_wait(read_sem)!=0)
    {
        perror("sem_wait");
        cv::Mat img_buf(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_8UC1, 0); 
        return img_buf; // return empty mat
    } 
    
    // construct opencv mat
    cv::Mat img_buf(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_8UC1, out_buffer);  
    return img_buf;
}

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

int mshared_init(){
    // create shared memory segment, ftok to generate unique key
    key_t key = ftok("capmemory",67);
    // shmget returns an identifier in shmid, the shm segment with same name was created in v4l2 process
    shmid = shmget(key, IMAGE_SIZE,0666|IPC_CREAT);
    if(shmid == -1)
    {
        printf("QL: Unable to Connect with the shared memory segment.\n");
        return -1;
    }
   
    // create shared memory to send coordinates to printer via tcp_client
    key_t crdkey = ftok("coordinatesmemory",67);
    //int crdshmid = shmget(crdkey, BUF_SIZE,0666|IPC_CREAT);
    crdshmid = shmget(crdkey, BUF_SIZE,0666|IPC_EXCL);
    if(crdshmid == -1)
    {
         perror("QL: unable to create the TCP client Shared Memory Segment.\n");
         return -1;
    }
    //define buffer address as shmat to attach to shared memory
    buf = (BUF*) shmat(shmid, (void*)0, 0); 
    crdbuf = (CRD*) shmat(crdshmid, (void*)0, 0); 
    
    //create semaphore to catch signal from grabber  
    if (sem_close(read_sem)!=0) perror("QL: search for previously opened READ semaphore");     
    if ( (read_sem = sem_open(SEMAPHORE_READ_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) 
    {
        perror("QL: semaphore for reading open");
        return -1;
    } 

    //create semaphore to catch signal from tcp_client (connect to printer) 
    if (sem_close(tcp_ready_sem)!=0) perror("QL: search for previously opened TCP semaphore");   
    if ( (tcp_ready_sem = sem_open(SEMAPHORE_TCP_READY_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) 
    {
        perror("QL: tcp ready semaphore open");
        return -1;
    } 

    return 0;
}

int mshared_remove(){
    
    // close semaphore
    if (sem_close(read_sem)!=0)
    {
        perror("QL: read_sem close");
        return -1;
    }
    if (sem_close(tcp_ready_sem)!=0)
    {
        perror("QL: tcp_ready_sem close");
        return -1;
    }
     // detach from shared memory
    if (shmdt(buf)!=0)
    {
        perror("QL: Image buffer detach");
        return -1;
    }    
    if (shmdt(crdbuf)!=0)
    {
        perror("QL: Coordinates buffer detach");
        return -1;
    }  
    // destroy the shared memory
    shmctl(shmid,IPC_RMID,NULL);
    shmctl(crdshmid,IPC_RMID,NULL);
    return 0;
}

int conveyor_mode(std::string *base, size_t *loopn, int *period, int *mode, ContourArgs *circle_args, ContourArgs *latch_args)
{
    int drops = 0;
    double totaltime = 0;
    double meantime = 0;
    double min_total_time = 100000;
    double max_total_time = 0;
    std::string problem_loop;   
    cv::Mat blurred;   
    Coordinates crd = {0,0,0,0,0,0,0};
    
    if(*mode==1) // mode=1 - aluminium can detection, mode=3 - bottle cap detection
    {
    // find latch mask sample contours
    V4i mask_latch_hierarchy;
    const std::string latchmaskfile = *base + "/images_masks/mask.bmp";
    cv::Mat mask = cv::imread(latchmaskfile, cv::IMREAD_GRAYSCALE);
    cv::findContours(mask, latch_args->mask_contours, mask_latch_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    }
    // find circle mask sample contours
    V4i mask_circle_hierarchy;
    const std::string circlemaskfile = *base + "/images_masks/round_mask.bmp";
    cv::Mat mask_circle = cv::imread(circlemaskfile, cv::IMREAD_GRAYSCALE);
    cv::findContours(mask_circle, circle_args->mask_contours, mask_circle_hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
     
    if(mshared_init()<0) {
        perror("QL: shared memory and semaphores registration");
        return -1;
    } 

    HRC::time_point begin;
    HRC::time_point end;
    HRC::time_point tbegin;
    HRC::time_point tend;
    double cft = 0.0;
    
    for(size_t j=0;j<*loopn;j++)
    {
        std::cout << "QL: |||||||||||||||--[" << j << "]--|||||||||||||||" << std::endl;
        crd={0,0,0,0,0,0};        
        // read image from grabber
        
        begin = HRC::now();
        cv::Mat img_gray = get_triggered_image(buf);
        end = HRC::now();
        coutf << "QL: Camera frame processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[μs]" << std::endl;
        
        tbegin = HRC::now();
        begin = HRC::now();        
        cv::GaussianBlur(img_gray, blurred, cv::Size(5, 5), 0, 0, cv::BORDER_DEFAULT);

        get_circle_crds(&crd,blurred,circle_args);
        if(*mode==1) // mode=1 in case of aluminium can detection, for bottle mode=3
        {
            if ((crd.circle_X > 0) & (crd.circle_Y > 0)){
            get_latch_crds(&crd,blurred,latch_args);
            }
            if(crd.radius > 0){
                crdbuf->y= (crd.label_Y - (crd.circle_Y - crd.radius)) * circle_args->radius / crd.radius;
            }
            else 
                crdbuf->y=circle_args->radius;
        }
        else
        {
            crdbuf->y=circle_args->radius;
        }
        
        if(crd.radius > 0){
            crdbuf->x=(crd.circle_X - IMAGE_WIDTH/2) * circle_args->radius /crd.radius;                        
            }           
        else 
            crdbuf->x=0;
        
        crdbuf->num=j;
        
        coutf << "QL: crdbuf->x = " << crdbuf->x << std::endl;
        coutf << "QL: crdbuf->y = " << crdbuf->y << std::endl;
        coutf << "QL: crdbuf->num = " << crdbuf->num << std::endl;
        // send IPC signal: "the coordinates are ready to be sent to printer over tcp" 
        sem_name = "TCP_REQ";
        if(drop_semaphore(SEMAPHORE_TCP_REQ_NAME, &sem_name)!=0){return -1;} 
        sem_name = "ACQ";
        if(drop_semaphore(SEMAPHORE_ACQ_NAME, &sem_name)!=0){return -1;} 
        
        end = HRC::now();
        if(*mode==1) {coutf << "QL: Find label x,y time: " 
                    << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                    << "[ms]" << std::endl;}
        else {coutf << "QL: Find circle center x,y time: " 
                    << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                    << "[ms]" << std::endl;}
        
        cft = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        //active wait answer IPC signal from printer server via semaphore
        if (sem_wait(tcp_ready_sem)!=0){perror("QL: tcp_ready_sem wait");}
        
        begin = HRC::now();
        if (OPT_GRAPH && !(j%5)){ // show only every 5th image to save memory
            cv::putText(blurred, std::to_string(j), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX,
                    1.5, cv::Scalar(255, 255, 255), 3);
            imshow("Output image with contours", blurred);
            cv::waitKey(1);
            //cv::destroyAllWindows(); 
        }
        else
        {        
            usleep((*period - cft -10)*1000);
            
        }
        end = HRC::now();
        
        if (OPT_GRAPH && !(j%5))
        {
            coutf << "QL: Window show time: " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                << "[ms]" << std::endl;
        }
        else
        {
            coutf << "QL: Idling time: " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                << "[ms]" << std::endl;
        }
        tend = HRC::now();
        coutf << "QL: Full loop time: " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count() 
                << "[ms]" << std::endl << std::endl;
        totaltime = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count();
        if((totaltime>*period))
        {
            drops+=1;
            problem_loop += "loop #" + std::to_string(j) + " overtime-> " + std::to_string(totaltime) + "[ms]\n";
        }
        else if((crd.circle_X == 0) | (crd.circle_Y == 0))
        {
            drops+=1;
            problem_loop += "loop #" + std::to_string(j) + " wrong coordinates\n";
        }   
        else if((*mode==1) && ((crd.label_X == 0) | (crd.label_Y == 0)))
        {
            drops+=1;
            problem_loop += "loop #" + std::to_string(j) + " wrong coordinates\n";
        }  
        else
        {
            meantime+=totaltime;
            min_total_time = totaltime < min_total_time? totaltime:min_total_time;            
        }
        max_total_time = totaltime > max_total_time ? totaltime : max_total_time;
         // send "ready for next capturing" IPC signal 
        sem_name= "CAP";
        if(drop_semaphore(SEMAPHORE_CAP_NAME, &sem_name)!=0){return -1;} 
        
    }
    meantime = (*loopn-drops)>0? meantime/(*loopn-drops):0;
    coutf << "QL: Done!" << std::endl;
    coutf << "QL: Average for " << *loopn << " CPU runs: " << meantime << "[ms]" << std::endl;
    coutf << "QL: Minimum CPU loop time: " << min_total_time      << "[ms]" << std::endl;
    coutf << "QL: Maximum CPU loop time: " << max_total_time      << "[ms]" << std::endl;
    coutf << "QL: Maximum circle area: " << circle_args->max_moment      << std::endl;
    coutf << "QL: Minimum circle area: " << circle_args->min_moment      << std::endl;
    if(*mode==1) {
    coutf << "QL: Maximum latch area: " << latch_args->max_moment      << std::endl;
    coutf << "QL: Minimum latch area: " << latch_args->min_moment      << std::endl;
    }
    coutf << "QL: Drops: "                 << drops       << std::endl;
    if (problem_loop.length()>3)
    {
        coutf << "QL: Problem loops are: "     << std::endl <<problem_loop.c_str() << std::endl;
    }
    cv::destroyAllWindows();
   
    if(mshared_remove()<0){
        perror("QL: shared memory and semaphores remove");
        return -1;
    } 
    
    return drops;
}


int main(int argc, char **argv) {
    //global variables
    int optMode = 0;
    int optGraph = 0;
    int optPeriod = 40; // loop period in milliseconds for gallery mode
    size_t optLoopN = 80000; // loop number set to top for live test, but will be limited in gallery mode
    std::string optDirectory = "cane_moving";
    int circle_threshold=35;
    int circle_radius=30;
    int circle_min_area=150000;
    int circle_max_area=300000;
    int latch_threshold=85;
    int latch_min_area=2000;
    int latch_max_area=3200;
    double scale=1.7;
    VP mask_circle_contours;
    VP mask_latch_contours;
    size_t files_number = 0;
    int ret=0;
    ret =  change_options_by_commandline(argc, argv, &optMode, &optGraph, &optLoopN, &optPeriod, &optDirectory,
            &circle_threshold, &circle_radius, &circle_min_area, &circle_max_area, &latch_threshold, &latch_min_area, &latch_max_area, &scale);
    ASSERT(ret==0);

    ContourArgs circle_args = {
            mask_circle_contours,
            circle_threshold,
            circle_min_area,
            circle_max_area,
            0,
            10,
            300000.0,
            0.0,
            circle_radius
    };
    ContourArgs latch_args = {
            mask_latch_contours,
            latch_threshold,
            latch_min_area,
            latch_max_area,
            scale,
            10,
            3200.0,
            0.0,
            0
    };
    
    OPT_GRAPH = optGraph;
    std::string argv_str(argv[0]);
    std::string base = argv_str.substr(0, argv_str.find_last_of("/"));
    std::string directory_full_name = base + std::string("/") + optDirectory + std::string("/");
    
    switch(optMode)
    {
        default:
        case 0:// gallery mode for aluminium can             
        case 2:// gallery mode for bottle 
            files_number = files_in_directory(&directory_full_name);
            optLoopN = optLoopN < files_number? optLoopN : files_number;
            ret = gallery_mode(&base, &optLoopN, &optPeriod, &optMode, &optDirectory, &circle_args, &latch_args);
            ASSERT(ret>=0);
            break;
        case 1:// conveyor mode for aluminium can
        case 3:// conveyor mode for bottle 
            ret = conveyor_mode(&base, &optLoopN, &optPeriod, &optMode, &circle_args, &latch_args);
            ASSERT(ret>=0);
            break;        

    }
    return 0;
}




