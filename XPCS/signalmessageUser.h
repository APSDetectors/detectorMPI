
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef SIGNALMESSAGEUSER_H
#define SIGNALMESSAGEUSER_H

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
struct guiMessageFieldsUser
{

    guiMessageFieldsUser();
    guiMessageFieldsUser& operator=(const guiMessageFieldsUser& other);



//calcs that mpi should do. any/all can be true/false
    // gui, mpiuser, mpiscatter
    //mpi user, before 1st calc, message.mpi_accum_darknoise =message.gui.is_acq_dark
    // mpiscatter::gotMPIGuiSettings, calls stuff..
    // set in gui widget
bool is_acq_dark;

//gui, and mpiUser
// set in gui widget
// mpiUser::beforeFirstCalc, message.num_dark_images_to_accum=message.gui.num_dark
int num_dark;

//set in gui to sub dark.
// in mpiUser::beforeCalcs, and beforeFirstCalc,
// message.mpi_sub_dark=message.gui.is_sub_dark
// the issure here is this:
// when subbing dark, the dark should be cleared when button is hit. then left alone.
// dark counter shold be set to 0 on button hit, then be left alone.
// There are two flags: mpi_accum_specs, which resets the darks. this should be done on
// button hit only. should be set hi and sent to ranks, then set low and sent to ranks.
// or else we keep zeroing the img. over and over
// mpi_accum_darknoise- is high for doing actual accum. it can stay high, and once counter runs out
// mpi_accum_darknoise has no effect. shouod be set high on bitton hit, but stauys hi.
bool is_sub_dark;


// for debugging mpi, saving mpi rams to files.
bool is_save_mpi_rams;

// debugging for 10s block of mpi
bool is_block_10s;
int ms_block_time;


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

bool is_descramble;
bool is_flipendian;

int display_time_ms;
int update_text_time_ms;


bool is_acq_dark_RBV;



};


//
// When there is a new image gotten from detetcor or new image completed by MPI
// we put these spcs in there. put in anything about images that will be useful.
// put in anything about mpi statuis, calculations, rank etc.
//



struct newImgMessageFieldsUser
{

    newImgMessageFieldsUser();
    newImgMessageFieldsUser& operator=(const newImgMessageFieldsUser& other);


};

//
// This is the message that is broadcasted to all mpi calc proceses at betgionning and end of calc.
// There are twop bcasts, one at start of calc, and one at end. this is wheat is in message.
// a starting point is the gui settings, and image specs.
//

struct mpiBcastMessageUser
{
    mpiBcastMessageUser();
    mpiBcastMessageUser& operator=(const mpiBcastMessageUser& other);





    //!!
    //!! added for XPCS
    //!!
    //Can have both false, or either bit not both true
    bool mpi_is_makeraw_imm;
    bool mpi_is_makecomp_imm;

    bool mpi_is_flip_endian;

    bool mpi_is_descramble;
    double mpi_num_std_thresh;
    int imm_bytes;



};


#endif // SIGNALMESSAGE_H
