
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

      is_set_outputtype=false;
    //calcs that mpi should do. any/all can be true/false
      is_acq_dark = false;


      \
      is_print_trace=false;


    //if tiff oputptu, here is info
     strcpy( tiffpath,"/local");
     strcpy( tiffbasename,"testy");
      tiffnumber=100;

      tiffnumber_RBV=100;

      is_capture_RBV=false;
      strcpy( fullfilename_RBV,"NULL");
      \

      is_bigstreamfile=false;

      is_set_tiffnumber=false;

    // if linux opipes, here is info.
     strcpy( inpipename,"/local/madpipe");
     strcpy( outpipename,"NULL");


     // when hi, we reset file output capture, or pipe output capture counter.
     // this should be HI only once. we set it hi, then we reset to 0 before next gui message..
     // or else we alreayd reset the img counter on every gui update
      start_capture=false;
     // num to capture or pipe out.
      num_file_capture=100;
     // true of we capture infiniate num of files, and not num_file_capture
    is_infinite_capture=false;


    stop_capture=false;
    is_file_num_inc=false;
    capture_counter=0;


     size_x=1024;
     size_y=1024;

     num_images=1;
     is_infinite_num_images=false;

     test_img_period=1000;



      brightness=0.0;
      contrast=1.0;

      is_calcs_started=false;


}


guiMessageFields& guiMessageFields::operator=(const guiMessageFields& other)
{

    guiMessageFieldsUser::operator=(other);

    command =  other.command;

   // use above enums for input source, out put source
   // it is either pipes or files or test images...
     input_type= other.input_type;
     output_type= other.output_type;

     is_set_outputtype=other.is_set_outputtype;

   //calcs that mpi should do. any/all can be true/false
     is_acq_dark = other.is_acq_dark;

is_bigstreamfile=other.is_bigstreamfile;
     is_print_trace=other.is_print_trace;

     capture_counter=other.capture_counter;

   //if tiff oputptu, here is info
    strcpy( tiffpath,other.tiffpath);
    strcpy( tiffbasename,other.tiffbasename);
     tiffnumber=other.tiffnumber;
    tiffnumber_RBV=other.tiffnumber_RBV;

    is_capture_RBV=other.is_capture_RBV;
    is_set_tiffnumber=other.is_set_tiffnumber;


    stop_capture=other.stop_capture;
    is_file_num_inc=other.is_file_num_inc;

   // if linux opipes, here is info.
    strcpy( inpipename,other.inpipename);
    strcpy( outpipename,other.outpipename);
    strcpy(fullfilename_RBV ,other.fullfilename_RBV);



    size_x=other.size_x;
    size_y=other.size_y;

    num_images=other.num_images;

    test_img_period=other.test_img_period;


    // is_negative=other.is_negative;

    start_capture=other.start_capture;
   // num to capture or pipe out.
    num_file_capture=other.num_file_capture;
   // true of we capture infiniate num of files, and not num_file_capture
  is_infinite_capture=other.is_infinite_capture;

  is_infinite_num_images=other.is_infinite_num_images;




  brightness=other.brightness;
  contrast=other.contrast;

  is_calcs_started=other.is_calcs_started;


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

      error_code=0;


}

newImgMessageFields& newImgMessageFields::operator=(const newImgMessageFields& other)
{
    newImgMessageFieldsUser::operator=(other);
    size_x=other.size_x;
    size_y=other.size_y;
    data_type=other.data_type;

     inpt_img_cnt=other.inpt_img_cnt;
     frame_number=other.frame_number;
    //!!int data_block_size_pixels;
     size_pixels=other.size_pixels;
     is_lost_images=other.is_lost_images;

     error_code=other.error_code;
    return *this;
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



    mpi_image_spec_offset_shorts=0;
}

mpiBcastMessage& mpiBcastMessage::operator=(const mpiBcastMessage& other)
{
    mpiBcastMessageUser::operator=(other);


    gui=other.gui;
    imgspecs=other.imgspecs;

    mpi_stop_calc_loop=other.mpi_stop_calc_loop;
    mpi_image=other.mpi_image;
    mpi_save_mem_file=other.mpi_save_mem_file;
    mpi_accum_darknoise=other.mpi_accum_darknoise;
    mpi_sub_dark=other.mpi_sub_dark;
    mpi_accum_specs=other.mpi_accum_specs;
    mpi_flip_endian=other.mpi_flip_endian;
    mpi_nop=other.mpi_nop;
    mpi_is_print_trace=other.mpi_is_print_trace;


      num_images_to_calc=other.num_images_to_calc;

    // 1 to tell mpi to calc on an image
   // is_calc_image=other.;
    //counts how manuy images was caled on ONE run of mpeOneLoop. this is a count all
    // images proc'ed by all ranks in ONE iteration of mpi calcs.
    img_calc_counter=other.img_calc_counter;




    // images in in fifl
     imgs_in_infifo=other.imgs_in_infifo;
    //input fifo len
     in_fifo_len=other.in_fifo_len;

    //how many images proc on rank0
     image_counter_rank0=other.image_counter_rank0;

    // num darks currently integrated...
     dark_integrate_counter=other.dark_integrate_counter;


     //tells how many iamges we should avarage on dark accum.
      num_dark_images_to_accum=other.num_dark_images_to_accum;


      // each rank calc how manuy images on one turn
       num_imgs_per_rank=other.num_imgs_per_rank;


       data_block_size_pixels_mpi=other.data_block_size_pixels_mpi;
       frame_number_mpi=other.frame_number_mpi;
      //fifl len of output fifo
       images_in_fifo_mpi=other.images_in_fifo_mpi;
      //not sure??
       image_in_ptr_mpi=other.image_in_ptr_mpi;

       fifo_len_mpi=other.fifo_len_mpi;
      //frames lost at mpi dequeue
       is_lost_frames_mpi=other.is_lost_frames_mpi;

       //true if we limit num processes.
        mpi_is_limit_nproc=other.mpi_is_limit_nproc;
       // number of procs to limit calc to
        mpi_max_num_proc=other.mpi_max_num_proc;



    mpi_image_spec_offset_shorts=other.mpi_image_spec_offset_shorts;

    return *this;
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
