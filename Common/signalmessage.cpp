
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "signalmessage.h"

//
//This is a message sent as a qt signal. the idea is to collect all of the settings of the gui
//in this struct, then send as a signal argument. it can be used for all the signals in the system
//to keep design simple. This info will be translated into MPI messages sent between processes.
// Put whatever you want as fields.
//

guiMessageFields::guiMessageFields() : guiMessageFieldsUser()
{
     command =  nop;

    // use above enums for input source, out put source
    // it is either pipes or files or test images...
      input_type= is_inout_null;
      output_type= is_inout_null;

    //calcs that mpi should do. any/all can be true/false
      is_acq_dark = false;


      \
      is_print_trace=false;


    //if tiff oputptu, here is info
     strcpy( tiffpath,"/local");
     strcpy( tiffbasename,"testy");
      tiffnumber=100;

      tiffnumber_RBV=100;

      strcpy( fullfilename_RBV,"NULL");
      \


    // if linux opipes, here is info.
     strcpy( inpipename,"/local/madpipe");
     strcpy( outpipename,"NULL");





     size_x=1024;
     size_y=1024;

     num_images=1;

     test_img_period=1000;




}


guiMessageFields& guiMessageFields::operator=(const guiMessageFields& other)
{

    guiMessageFieldsUser::operator=(other);

    command =  other.command;

   // use above enums for input source, out put source
   // it is either pipes or files or test images...
     input_type= other.input_type;
     output_type= other.output_type;

   //calcs that mpi should do. any/all can be true/false
     is_acq_dark = other.is_acq_dark;


     is_print_trace=other.is_print_trace;


   //if tiff oputptu, here is info
    strcpy( tiffpath,other.tiffpath);
    strcpy( tiffbasename,other.tiffbasename);
     tiffnumber=other.tiffnumber;
    tiffnumber_RBV=other.tiffnumber_RBV;

   // if linux opipes, here is info.
    strcpy( inpipename,other.inpipename);
    strcpy( outpipename,other.outpipename);
    strcpy(fullfilename_RBV ,other.fullfilename_RBV);



    size_x=other.size_x;
    size_y=other.size_y;

    num_images=other.num_images;

    test_img_period=other.test_img_period;


     is_negative=other.is_negative;


             return *this;
}


newImgMessageFields::newImgMessageFields() :  newImgMessageFieldsUser()
{
     size_x=0;
     size_y=0;
     data_type=0;

      inpt_img_cnt=0;
      frame_number=0;
     //!!int data_block_size_pixels;
      size_pixels=0;
      is_lost_images=0;
}


mpiBcastMessage::mpiBcastMessage() :
        gui(),
        imgspecs(),
        mpiBcastMessageUser()
{



    mpi_stop_calc_loop=false;
    mpi_image=false;
    mpi_save_mem_file=false;
    mpi_accum_darknoise=false;
    mpi_sub_dark=false;
    mpi_accum_specs=false;
    mpi_flip_endian=false;
    mpi_nop=false;
    mpi_is_print_trace=false;


      num_images_to_calc=0;

    // 1 to tell mpi to calc on an image
   // is_calc_image=0;
    //counts how manuy images was caled on ONE run of mpeOneLoop. this is a count all
    // images proc'ed by all ranks in ONE iteration of mpi calcs.
    img_calc_counter=0;




    // images in in fifl
     imgs_in_infifo=0;
    //input fifo len
     in_fifo_len=0;

    //how many images proc on rank0
     image_counter_rank0=0;

    // num darks currently integrated...
     dark_integrate_counter=0;


     //tells how many iamges we should avarage on dark accum.
      num_dark_images_to_accum=0;


      // each rank calc how manuy images on one turn
       num_imgs_per_rank=1;


       data_block_size_pixels_mpi=0;
       frame_number_mpi=0;
      //fifl len of output fifo
       images_in_fifo_mpi=0;
      //not sure??
       image_in_ptr_mpi=0;

       fifo_len_mpi=0;
      //frames lost at mpi dequeue
       is_lost_frames_mpi=0;

       //true if we limit num processes.
        mpi_is_limit_nproc=false;
       // number of procs to limit calc to
        mpi_max_num_proc=2;



}


signalMessage::signalMessage()
{
}


mpiSignalMessage::mpiSignalMessage() :
    signalMessage(),
    message()
{

}

guiSignalMessage::guiSignalMessage() :
    signalMessage(),
    message()
{

}

imageSignalMessage::imageSignalMessage() :
    signalMessage(),
    message()
{

}
