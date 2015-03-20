
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
struct guiMessageFields
{

    guiMessageFields();
    guiMessageFields& operator=(const guiMessageFields& other);



enum iosource {
is_input_testimgs,
is_input_pipe,
is_output_tiff,
is_output_pipe,
is_inout_null
};

enum mpi_command {
start_calcs,
stop_calcs,
conn_input,
disconn_input,
conn_output,
disconn_output,
nop,
update_thresh
};


//
// Commdns to turn MPI calcs on/off or conn/discon inputs outputs
//
mpi_command command;

// use above enums for input source, out put source
// it is either pipes or files or test images...
iosource input_type;
iosource output_type;

//calcs that mpi should do. any/all can be true/false
bool is_acq_dark;
int num_dark;
bool is_sub_dark;


bool is_print_trace;

// for debugging mpi, saving mpi rams to files.
bool is_save_mpi_rams;

// debugging for 10s block of mpi
bool is_block_10s;

//if tiff oputptu, here is info
char tiffpath[256];
char tiffbasename[80];
// user requested tiff number
int tiffnumber;
// tiff number after increments by file writer... read back
int tiffnumber_RBV;
char fullfilename_RBV[512];

// if linux opipes, here is info.
char inpipename[256];
char outpipename[256];

//image size
int size_x;
int size_y;

//num images
int num_images;

//test image period ms
int test_img_period;

// true of we limit nuim procs
bool is_limit_max_proc;
// num of proc to use max
int max_num_procs;

//!!
//!! add for xpcs
//!!
// num of stds to set thresh for noise meas
double num_std_thresh;
// below for imm, can have both false, or one or the other true, not both true
bool is_comp_imm;
bool is_raw_imm;


// wjhich item to view
int which_img_view;


int input_queue_size_mb;
bool is_rst_in_queue;
// act q len, after setting up to size in mb
int input_queue_len_RBV;

int output_queue_size_mb;
bool is_rst_out_queue;
int output_queue_len_RBV;
};


//
// When there is a new image gotten from detetcor or new image completed by MPI
// we put these spcs in there. put in anything about images that will be useful.
// put in anything about mpi statuis, calculations, rank etc.
//



struct newImgMessageFields
{

    newImgMessageFields();

    int size_x;
    int size_y;
    int data_type;

    int inpt_img_cnt;
    int frame_number;
    //!!int data_block_size_pixels;
    int size_pixels;
    int is_lost_images;





};

//
// This is the message that is broadcasted to all mpi calc proceses at betgionning and end of calc.
// There are twop bcasts, one at start of calc, and one at end. this is wheat is in message.
// a starting point is the gui settings, and image specs.
//

struct mpiBcastMessage
{
    mpiBcastMessage();

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

    // tell mpi to stop
      bool   mpi_stop_calc_loop;
      // tell mpi we have new frame or frames
      bool   mpi_image;
    // save ram from different procs. for debugging image calcs.
      bool   mpi_save_mem_file;
      // accum darks- will accum until we reach num darks to accum.
     bool    mpi_accum_darknoise;
     // subtract darks
     bool    mpi_sub_dark;
     // reset dark iamges to 0's and zero dark accum counter
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
     int num_images_to_calc;



     //
     // info about mpi calcs
     //

    // num times we started mpi calcs
    int img_calc_counter;

    // images in in fifl
    int imgs_in_infifo;
    //input fifo len
    int in_fifo_len;

    //how many images proc on rank0
    int image_counter_rank0;

    // num darks currently integrated... set by MPI ranks...
    int dark_integrate_counter;

    //tells how many iamges we should avarage on dark accum.
    int num_dark_images_to_accum;


    // each rank calc how manuy images on one turn
    int num_imgs_per_rank;



    int data_block_size_pixels_mpi;
    int frame_number_mpi;
    //fifl len of output fifo
    int images_in_fifo_mpi;
    //not sure??
    int image_in_ptr_mpi;

    int fifo_len_mpi;
    //frames lost at mpi dequeue
    int is_lost_frames_mpi;


    //true if we limit num processes.
    bool mpi_is_limit_nproc;
    // number of procs to limit calc to
    int mpi_max_num_proc;




    //!!
    //!! added for XPCS
    //!!
    //Can have both false, or either bit not both true
    bool mpi_is_makeraw_imm;
    bool mpi_is_makecomp_imm;

    double mpi_num_std_thresh;
    int imm_bytes;



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
