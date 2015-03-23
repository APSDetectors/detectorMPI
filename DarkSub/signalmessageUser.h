
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




enum  {

update_thresh=100
};


//calcs that mpi should do. any/all can be true/false
bool is_acq_dark;
int num_dark;
bool is_sub_dark;


// for debugging mpi, saving mpi rams to files.
bool is_save_mpi_rams;

// debugging for 10s block of mpi
bool is_block_10s;


// true of we limit nuim procs
bool is_limit_max_proc;
// num of proc to use max
int max_num_procs;

};


//
// When there is a new image gotten from detetcor or new image completed by MPI
// we put these spcs in there. put in anything about images that will be useful.
// put in anything about mpi statuis, calculations, rank etc.
//



struct newImgMessageFieldsUser
{

    newImgMessageFieldsUser();



};

//
// This is the message that is broadcasted to all mpi calc proceses at betgionning and end of calc.
// There are twop bcasts, one at start of calc, and one at end. this is wheat is in message.
// a starting point is the gui settings, and image specs.
//

struct mpiBcastMessageUser
{
    mpiBcastMessageUser();



};


#endif // SIGNALMESSAGE_H
