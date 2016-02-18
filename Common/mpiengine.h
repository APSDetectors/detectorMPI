

/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/







#ifndef MPIENGINE_H
#define MPIENGINE_H

#include <QObject>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <math.h>


#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // prerequisite typedef
#include <sys/socket.h> // struct sockaddr; system prototypes and constants
#include <netdb.h>    // network info lookup prototypes and structures
#include <netinet/in.h> // struct sockaddr_in; byte ordering macros
#include <arpa/inet.h>  // utility function prototypes


#include "mpi.h"


//#include <QHostAddress>


#include "signalmessage.h"

#include "imagequeueitem.h"

#define BILLION 1000000000L;

#define MAX_TIF			1000	// max tif file number

#define MAX_BLAEDS_NUM		2	// maxium number of blades
#define MAX_THREAD              100	// maxium threads number, defined by the hardware
#define BIT_PER_SAMPLE          16	// defined by camera property

#define alpha			3	// between [0, 4]
#define beta			15	// between [0, 20]

#define PORT			2348



// define if you  want whole image MPI, in stdead of splitting imgs into chunks
#define MPI_WHOLE_IMGS 1


class mpiEngine : public QObject
{
    Q_OBJECT
public:
    explicit mpiEngine(QObject *parent = 0);
    ~mpiEngine();


    \
    mpiBcastMessage my_message;

   //!! char *ipaddr;
      int CPU_THRS_NUM;

      int ttlprocs;				// total physical process number
      int compprocs;				// computation proces in total
      int numprocs;				// process number
      int numthrds;				// threads number in each process

     //!! unsigned short *strip_buff;     // parallel process image buffer
     //!! unsigned short *image_buff;	// input image, remote mem acces enterance


    //____________________________________________________________

      char *output_filename[];



      int rank;
      int i;

    //!!  int *comp_rank, *forward_rank, *backward_rank;


      MPI_Status status;
      // public window for rma
      MPI_Win win;

      MPI_Win windouble;



//!!      MPI_Group global_group, fgroup, bgroup, cgroup;     // forward group, backward group, computation group
      MPI_Comm MPI_FOR_WORLD, MPI_BACK_WORLD, MPI_COMP_WORLD;        // forward, backward, computation communicator
      // build MPI struct for image_info
      int count;
     //!! MPI_Datatype MPI_Imageinfo;

      //!1static  MPI_Datatype def_types[7];
     //!!static  int def_len[7];

      //!! MPI_Datatype types[7];
       //!!int len[7];


    //!! MPI_Aint disp[7];
     //!! long int base;



      //
      // setup for short image window
      //

      int pub_sh_img_nimgs;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
      int pub_sh_img_size_bytes;//def img size- large image.
      //bytes in rma memory for all proc...units are in shorts for puib_img_size2
       enum {

             pub_img_max_nimgs=32 //num if imgs ptrs into img mem.-

           };


        // where we put our large img. memopry
       unsigned short *public_short_image_memory;
       //pointers into the large img memopry
       unsigned short *public_short_image[pub_img_max_nimgs];

       //if we pass several images to a rank, we line them up by img_spacing_shorts.
       //this is an offset from public_short_image[n] pointer  to an image.
       int img_spacing_shorts;



       //
       // Setuyp for double image window
       //

       int pub_db_img_nimgs;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
       int pub_db_img_size_bytes;//def img size- large image.
       //bytes in rma memory for all proc...units are in shorts for puib_img_size2



         // where we put our large img. memopry
        double *public_double_image_memory;
        //pointers into the large img memopry
        double *public_double_image[pub_img_max_nimgs];

        //if we pass several images to a rank, we line them up by img_spacing_shorts.
        //this is an offset from public_short_image[n] pointer  to an image.
        int img_spacing_doubles;

        //eacg rank count num darks he accums
        int rank_dark_accum_cnt;

       //
        //
        //

        //total num images to calc this tuyrn on all ranks.
        int num_images_to_calc;

       //if we have small images we will want one rank to do several iamges.
       int num_imgs_per_rank;


        // number of imgs this rank has to calc. if num_imgs_per_rank=1, and we have 4 processes,
       // and only 3 imgs to calc, then rank 0,1,2 this is set to 1. rank3 it is 0.
       int num_imgs2calc_thisrank;

       //true if this rank has an img to calc, ie. num_imgs2calc_thisrank>0
       bool is_calc_image;
       //as we scatter images to ranks, we count them, to figure out which rank gets the image.
       // if num_imgs_per_rank=1, then this is trivial...
       int scatgath_counter;

    bool   is_print_trace;
    int trace_count;

    int err_stat;

    void hw_info();

  //!!  void debug();


    imageSpecs* getImageSpecs(int whichimg);

    //parse mpi message, and set class variables accordingly.
    // can be overridden by subclass
    // called by all ranks after Bcast, so all ranks match in their class\
    //vars.
    virtual int parseMessage(mpiBcastMessage &message);


    // edits message based on rank0 class vars, so all ranks reflect rank0 for certain
    // data. can be subclassed/ called before BCast to all ranks
    virtual  int mpiEngineRank0EditMessage(mpiBcastMessage &message);

    // print debug info if is_print_trace true
    void printTrace(char *m);

    // clear public double and short iamges to 0s
    void clearImages(int rr);
    void clearImages(void);

    // for debugging- write image in ptr to a file.
    // ptr is data, lenbutes is numbutes.
    //fbase is base file anme, fnum is file number added to name.
    // rank is auytomatically incl in the name.
    void writeImageRaw(void *ptr, int lenbytes,char* fnbase,int fnum);

    // flip endianess of the iamge.
    void flipEndian(int pix_x, int pix_y,int offset, int count);


    // set up mpiengine memory etc.
 int setupMPI(int argc, char *argv[]);
 // callback so child classes can do something in setup
    virtual int setupMPI2(int argc, char *argv[]);

// shitdown mpi free windows, mem etc.
void  shutdownMPI();
// callback for child classes
    virtual void shutdownMPI2();

//call fence twice, do nothing in between
void fenceMPIData(void);
//call fence once. on win, or public_iomage
void fenceMPIData2(void);

//get from target public iamge to local public image space
void getMPIData(int target,int count,int tar_offset,int src_offset);
//get from target public_short_image+tar_offset to local memory. count in shorts
//must void fenceMPIData2(void);
void getMPIData2(int target,int count,int tar_offset, unsigned short *local_memory);

//assign public_iamge[x] based on image size, and number of images o process per rank
void assignPubImgPtrs(void);


// broadcase to all ranks, from root, to all ranks.
//message is the message, count is num bytes in message.
void* Bcast(void *message,int count, int root);

//put from local this->public_short_image to other ranks
void putMPIData(int target,int count,int tar_offset,int src_offset);
//put from some local mem ptr to ranks. if rank=target it is a memcpuy to public)image
//,ust call w/ ffenceMPIData2
void putMPIData2(int target,int count,int tar_offset,unsigned short *local_memory);


//!!int Bcast(int message,int *imgx, int *imgy, int root);

// send/receive between ranks. count is num bytes in message.
int Recv(void *message, int count, int fromwho);
void Send(void *message, int count, int towho);


// for rank 2,3,4. this is a loop that does calcs over an over.
void mpiCalcLoop(void);

// this is the routine that is executed each time we do a calc on all ranks
// put your stuff in here.
virtual int mpiOneLoop(mpiBcastMessage &message);

//virtual int doImgCalcs(int message,int imgx, int imgy,int nimgs,bool is_calc,bool isaccdark);

virtual int doImgCalcs(void);

// calls RCV and waits, then sends out QT signals.
void waitSentMessages(void);
//!!void *attached_mem;






// given tot num images to proc by all ranks, figure out how many
// iamges this rank must calc.
int getNumImgs2CalcThisRank(int nimgs);

//
// vars for calc'ing dark accum, partial sums
//

// true if image accum is done amd we meed to combine partial sums across ranks
bool is_finish_darks;
// true of we have already finished darks
bool is_finish_darks_done;

// true if we need  to accum images.
bool is_acc_darks;
//total num of images to accum, desired num iamges to accum
int num_dark_integrate;
// current number of images we have integrated, sum of all ranks.
int dark_integrate_counter;
// number of images this rank must accum, if any, for dark accum, or any partial sum
int num_images_to_acc_this_rank;

//some important img specs
int img_size_x;
int img_size_y;
int img_num_pixels;



// if accums are done, combine partial sums.all ranks get tiotal sum, and averated, into
// pub doub img which_doub_image only done if needed. return true of a combine was in fact done
// if called by one rank it must be called by all, as it has fences.
bool combineDarkStd(int which_doub_image);

// combine whichdoube image partial sums across all ranks. mult by mult_factor, like
// 1/N or 1/(N-1). mult_factor is 1 for just a sum
bool combineDarkStd(int which_doub_image,double mult_factor);

//clears pub img double img which_doub_image to 0.0's

void clearDarks(int which_doub_image);

// incs pub short image[whichimg] into pub img doble[which_doub_image] if we need to.
void accumDarkStd(int which_sh_img, int which_doub_image);

// incs pub short image[whichimg] into pub img doble[which_doub_image] if we need to.
void accumDarkStd(double *doubimg, int which_doub_image);



// calcs if we need to integrate an image this rank
int calcNumDarksAcc(int nimgs);


//figure out if this rank has to calc on any images.
//pass in total num images that need to be processed byh all ranks
bool getIsCalc(int nimgs);


void resetScatGathCounter(void);
void incScatGathCounter(void);

void scatterImage(int count,unsigned short *local_memory);
void scatterImage(imageQueueItem *item);

void gatherImage(int count,unsigned short *local_memory,int which_image);
void gatherImage(int count, imageQueueItem *item,int which_image);

virtual void beforeCalcs(mpiBcastMessage &message){};
virtual void afterCalcs(mpiBcastMessage &message){};
 virtual void beforeFirstCalc(mpiBcastMessage &message){};


imageSpecs *image_specs;

// image number, this is lowest frame number for all ranks. it is frame number of rank 0.
//we assume image numbers for other ranks is frame_number + rank.
// we set this number  on new images  then call mpiOneLoop, which will bcast this number
// to all the ranks. each rank will in turn set this field so all ranks match.
// number starts at zero, and incs every time a new image comes in to the udp port.
quint32 image_counter_rank0;




public slots:






virtual void    sendMPIGuiSettings(guiSignalMessage message);


signals:




virtual void signalGuiSettings(guiSignalMessage message);

};

#endif // MPIENGINE_H
