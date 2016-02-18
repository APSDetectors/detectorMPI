
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef SIGNALMESSAGE_H
#define SIGNALMESSAGE_H

#include <QMetaType>

#include "signalmessageUser.h"

/***************************************************************************************
 *This is a message sent as a qt signal. the idea is to collect all of the settings of the gui
 *in this struct, then send as a signal argument. it can be used for all the signals in the system
 *to keep design simple. This info will be translated into MPI messages sent between processes.
 * Put whatever you want as fields.
/******************************************************************************************/



//
// Put all your custom stuff below. You can remove the current fields
// use all basic types so you don't have huge binary message
// this struct will be sent over mpi as a byte stream.
// also it is sent as a signal.slot etc.
// all this stuff reflects what is on the user interface. We collect all uiser interface
// settings, and put into a message that can be a signal.
//
struct guiMessageFields : public guiMessageFieldsUser
{

    guiMessageFields();
    guiMessageFields& operator=(const guiMessageFields& other);



enum iosource {
is_input_testimgs,
is_input_pipe,
is_output_tiff,
is_output_pipe,
is_inout_null,
is_output_imm
};

enum  mpi_command{
start_calcs,
stop_calcs,
conn_input,
disconn_input,
conn_output,
disconn_output,
nop,
update_thresh,
ave_darks_now};


//
// Commdns to turn MPI calcs on/off or conn/discon inputs outputs
//
// thgis is all over the place....
//mpiScatter:: gotMPIGuiSettings, stqat_calcs, sets up dark accum,
//              stop calcs, discon ninput pipe, clears data queue.
// pipeWriteer, on startcalcs, opens pipe. on stop calcs, closes pipe
// mpiUser::parseMessage- update_thresh, calcs thresh img, for new std.
// gui- sets the command based on buttons. received by other obhects.
// mpiscatterUser::gotGui--- mpi_message.gui.command==mpi_message.gui.ave_darks_now
//          for dark averging... then set to nop before setupMPI...
mpi_command command;

// use above enums for input source, out put source
// it is either pipes or files or test images...
iosource input_type;
iosource output_type;

bool is_set_outputtype;

bool is_print_trace;



//if tiff oputptu, here is info
char tiffpath[256];
char tiffbasename[80];
// user requested tiff number
int tiffnumber;
// tiff number after increments by file writer... read back
int tiffnumber_RBV;
char fullfilename_RBV[512];

//if we start capture, do we reset the file number to gui setting?
bool is_set_tiffnumber;

bool is_bigstreamfile;

// when hi, we reset file output capture, or pipe output capture counter.
// this should be HI only once. we set it hi, then we reset to 0 before next gui message..
// or else we alreayd reset the img counter on every gui update
bool start_capture;
// num to capture or pipe out.
int num_file_capture;
// true of we capture infiniate num of files, and not num_file_capture
bool is_infinite_capture;

// if we are saving files, htere is cap counter..
//set in pipewriter
int capture_counter;

//are we capturing... pipeWriter setws this
bool is_capture_RBV;
//set by gui.. to stop capturing in pipewriter
bool stop_capture;
//true for pipewriter to inc the file save number, as part of filenmawe.
bool is_file_num_inc;



// if linux opipes, here is info.
char inpipename[256];
char outpipename[256];

//image size
int size_x;
int size_y;

//num images- to inpput to sustem- num images or num imgs to read from input pipe
int num_images;
// true of we input infinitae images and not num_images
bool is_infinite_num_images;



//test image period ms
int test_img_period;

int is_acq_dark;

float brightness;
float contrast;


//true after start, false after stop
bool is_calcs_started;


};




//
// When there is a new image gotten from detetcor or new image completed by MPI
// we put these spcs in there. put in anything about images that will be useful.
// put in anything about mpi statuis, calculations, rank etc.
//



struct newImgMessageFields : public newImgMessageFieldsUser
{

    newImgMessageFields();
    newImgMessageFields& operator=(const newImgMessageFields& other);

    // size x of image- mpiengine, pipeReader, mpiUser
    int size_x;
    int size_y;
    // unused... only in signal messasge...
    int data_type;

    // set in pipereader, readback in gui,
    // in mpiscatterUser- on dequeue- mpi_message.imgspecs.inpt_img_cnt=item->specs->inpt_img_cnt;
    // not sure if inpt_img_cnt=item->specs->inpt_img_cnt is ever set...
    // in pipeReader.. imgspecs.message.inpt_img_cnt=test_frame_number; for pipes and test images.
    int inpt_img_cnt;

    // set in pipeReader, but never read amnywuere... imgspecs.message.frame_number= test_frame_number

    int frame_number;
    //!!int data_block_size_pixels;

    //used in mpiengine, mpigather, pipereader, mpiuser
    int size_pixels;

    //in pipeReader set to false. in mpiGather set to true if we cannot dequeue on freequeue
    // never set in scatter... this is bad
    int is_lost_images;

    int error_code;

};

//
// This is the message that is broadcasted to all mpi calc proceses at betgionning and end of calc.
// There are twop bcasts, one at start of calc, and one at end. this is wheat is in message.
// a starting point is the gui settings, and image specs.
//

struct mpiBcastMessage : public mpiBcastMessageUser
{
    mpiBcastMessage();

    mpiBcastMessage& operator=(const mpiBcastMessage& other);

    //orig gui settings
    guiMessageFields gui;

    //image specs, size etc.
    //
    newImgMessageFields imgspecs;


    //1 if we are supposed to calc an image.
    // bits we send to all mpi procs that are commands.

//
    //
    // commands for each mpi calc
    //

    // tell mpi to stop- used in mpiGather. Never is set anywhere!!!
      bool   mpi_stop_calc_loop;
      // tell mpi we have new frame or frames. used in mpiGether,
      //set in mpiGatherUser, used in mpiUser, mpiengine
      //set to true if we defifo new image in mpiGather. means there are new fraems to calc.
      bool   mpi_image;
    // save ram from different procs. for debugging image calcs.
     //set in mpiUser, used in mpiUser. if gui ssays save dbg files we set true.
      bool   mpi_save_mem_file;
      // accum darks- will accum until we reach num darks to accum.
      // mpiengine uses it, mpiuser sets , mpiscatteruser sets
      // it is set to true if user wishes to accum dark images. it sets var
      // in mpiengine, is_acc_darks, if mpi_accum_darknoise, and counter<num dark to ave.
      //set in beforeFirst calc... it stays high after darks are done averaging.
      //mpiEngine::parseMessage, if (message.mpi_accum_darknoise &&dark_integrate_counter<num_dark_integrate)
      // mpiEngine::parseMessage   if (!message.mpi_accum_darknoise)
      //mpiUser::beforeFirstCalc, message.mpi_accum_darknoise =message.gui.is_acq_dark

      //mpiScatterUser:: gotMPIGuiSettings, if (mpi_message.gui.command==mpi_message.gui.ave_darks_now, to true
     bool    mpi_accum_darknoise;



     // subtract darks. set in mpiUser based on gui. used in mpiUser to turn on dark sub
     bool    mpi_sub_dark;
     // reset dark iamges to 0's and zero dark accum counter
     // in mpiScatter:getGui... if (mpi_message.gui.command==mpi_message.gui.start_calcs), it sets hi and lo
     // and calls setupMPI for each.
     // rad back in mpiengine::parseMessage, to set dark_integrate_counter, and zero dark img.
     // this has to be in all ranks.
     //mpiUser::parseMessage, to clear darks, and claer std image.
     //mpiScatteruser::got gui, if (mpi_message.gui.command==mpi_message.gui.ave_darks_now)...
     // calls setupMPI twice to zero stuff on the gui button
      bool   mpi_accum_specs;
      // flip endian of image
     bool    mpi_flip_endian;
    // do nothing...
     bool    mpi_nop;
// debugging print messages from all ranks
      bool    mpi_is_print_trace;



    // this is important to make calc work correctly, dont mess with it.
     // when call mpi calcs, this is how many ioamges we have gotten from detector,
     // and we send this number of images to the set of ranks on oneLoopMpi
      // in mpiscatterUser, mpi_message.num_images_to_calc=img_calc_counter;
      // this is number of images we defifoed and scattered to all ranks. if we defifo one img,
      // it is set to 1. if we defof 10 imgs, and scatter to 10 ranks, it is 10.
      // if num_images_to_calc=5 and nranks=10, then rank 6-10 know they have no image to deal with.
      // this keeps track of things like dark accum over many ranks.
     int num_images_to_calc;



     //
     // info about mpi calcs
     //

    // num times we started mpi calcs. this counts number of images scattered to all ranks.
     // on one iteration, we defifo 2 imgs, we inc this by 2. it is calc'ed in rank 0.
    int img_calc_counter;

    // images in in fifl- mpiscatterUser sets this on dequing new images. it is num images in inut data Q
    int imgs_in_infifo;
    //input fifo len- seems unused.
    int in_fifo_len;

    //how many images proc on rank0
    // set in mpiengine, and mpiUser. also there is member of mpiengine image_counter_rank0.
    int image_counter_rank0;

    // num darks currently integrated... set by MPI ranks...
    //seems un used. prob does nothing...
    int dark_integrate_counter;

    //tells how many iamges we should avarage on dark accum.
    //mpi engine...mpiuser . gets val from gui in mpiUser. tells how many to accum.
    //
    int num_dark_images_to_accum;


    // each rank calc how manuy images on one turn
    //used in mpiengine. tells how many imgs to scatter to each rank
    // inluy tested with 1. don;t know if even owrk with >1
    int num_imgs_per_rank;


    // in mpiUser vefore 1st calc, rank0 onluy, message.data_block_size_pixels_mpi=message.imgspecs.size_pixels
    // it is num of puxels in an image. sent to all ranks
    int data_block_size_pixels_mpi;

    //reset to -0 on before 1st calc, incremeted buy all ranks locally in each frame calced. mpiUser, beforeCalcs
    int frame_number_mpi;

    //fifl len of output fifo
    // seems unused.. read by gui, but never set to anything...
    int images_in_fifo_mpi;
    //not sure?? - unused
    int image_in_ptr_mpi;

    // unused- never set anywhere...
    int fifo_len_mpi;
    //frames lost at mpi dequeue
    // seems unused
    int is_lost_frames_mpi;


    //true if we limit num processes.
    //set in gui on mpiUser vefore 1st calc. used in mpiscatteruser as debuggin tool
    // to limit num ranks to scatter to. of false, scatter ao all ranks. if fase scatter to
    // rank 0 to mpi_max_num_proc-1
    bool mpi_is_limit_nproc;
    // number of procs to limit calc to- mpiUser, mpiscatteruser
    // num of ranks to scatter to.
    int mpi_max_num_proc;



    // in pub images, we store 1st the iamge data, then copy in the
    // image specs fields. this number will be the same as the
    // size of malloc'ed mem for each iamgequeueitem->img_data.
    // it is set in defifoScatter in mpiScatter:: class. the val
    // is then sent to all ranks. it is in unites of shorts not butes
    int mpi_image_spec_offset_shorts;


};
//Make all your messages signalMessages, so you can hae more than one type. Inherit from this.
//
class signalMessage
{
public:
    signalMessage();


};


Q_DECLARE_METATYPE(signalMessage)

class guiSignalMessage : signalMessage
{
public:
    guiSignalMessage();

    guiMessageFields message;

};


//
// Leave the below alone- QT MOC needs it to create a signal message.
//

Q_DECLARE_METATYPE(guiSignalMessage)
// in main_mpi, main function we must call int id = qRegisterMetaType<MyStruct>();




class imageSignalMessage : signalMessage
{
public:
    imageSignalMessage();

    newImgMessageFields message;

};


//
// Leave the below alone- QT MOC needs it to create a signal message.
//

Q_DECLARE_METATYPE(imageSignalMessage)
// in main_mpi, main function we must call int id = qRegisterMetaType<MyStruct>();





class mpiSignalMessage : signalMessage
{
public:
    mpiSignalMessage();

    mpiBcastMessage message;

};


//
// Leave the below alone- QT MOC needs it to create a signal message.
//

Q_DECLARE_METATYPE(mpiSignalMessage)
// in main_mpi, main function we must call int id = qRegisterMetaType<MyStruct>();





#endif // SIGNALMESSAGE_H
