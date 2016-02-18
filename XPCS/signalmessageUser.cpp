
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

guiMessageFieldsUser::guiMessageFieldsUser()
{


    //calcs that mpi should do. any/all can be true/false
      is_acq_dark = false;
      num_dark=10;
      is_sub_dark=false;

      is_flipendian=false;

       is_save_mpi_rams=false;

       is_block_10s=false;
        ms_block_time=1000;


    display_time_ms=200;
    update_text_time_ms=1000;

     // true of we limit nuim procs
      is_limit_max_proc=false;
     // num of proc to use max
      max_num_procs=2;

      num_std_thresh=4.0;

      is_raw_imm=false;
      is_comp_imm=false;

      which_img_view=0;

      input_queue_size_mb=200;
      is_rst_in_queue=false;
        input_queue_len_RBV=200/2;

       output_queue_size_mb=200;
       is_rst_out_queue=false;
        output_queue_len_RBV=200/2;

        is_descramble=false;

        is_acq_dark_RBV=false;


}


guiMessageFieldsUser& guiMessageFieldsUser::operator=(const guiMessageFieldsUser& other)
{


   //calcs that mpi should do. any/all can be true/false
     is_acq_dark = other.is_acq_dark;
     num_dark=other.num_dark;
     is_sub_dark=other.is_sub_dark;


     is_save_mpi_rams=other.is_save_mpi_rams;
     is_block_10s=other.is_block_10s;
    ms_block_time=other.ms_block_time;

    is_descramble=other.is_descramble;

    is_limit_max_proc=other.is_limit_max_proc;


    // num of proc to use max
     max_num_procs=other.max_num_procs;

     num_std_thresh=other.num_std_thresh;
     is_comp_imm=other.is_comp_imm;
     is_raw_imm=other.is_raw_imm;

     which_img_view=other.which_img_view;

    is_flipendian=other.is_flipendian;

     input_queue_size_mb=other.input_queue_size_mb;
     is_rst_in_queue=other.is_rst_in_queue;
    input_queue_len_RBV=other.input_queue_len_RBV;

     output_queue_size_mb=other.input_queue_size_mb;
     is_rst_out_queue=other.is_rst_in_queue;
        output_queue_len_RBV=other.output_queue_len_RBV;
        display_time_ms=other.display_time_ms;
        update_text_time_ms=other.update_text_time_ms;

        is_acq_dark_RBV=other.is_acq_dark_RBV;



     return *this;
}


newImgMessageFieldsUser::newImgMessageFieldsUser()
{

}


newImgMessageFieldsUser& newImgMessageFieldsUser::operator=(const newImgMessageFieldsUser& other)
{
    return *this;
}

mpiBcastMessageUser::mpiBcastMessageUser()
{




        //Can have both false, or either bit not both true
         mpi_is_makeraw_imm=false;
         mpi_is_makecomp_imm=false;

         mpi_num_std_thresh=4.0;
         imm_bytes=0;

          mpi_is_flip_endian=true;

          mpi_is_descramble=true;



}


mpiBcastMessageUser& mpiBcastMessageUser::operator=(const mpiBcastMessageUser& other)
{

    //Can have both false, or either bit not both true
     mpi_is_makeraw_imm=other.mpi_is_makeraw_imm;
     mpi_is_makecomp_imm=other.mpi_is_makecomp_imm;

     mpi_num_std_thresh=other.mpi_num_std_thresh;
     imm_bytes=other.imm_bytes;

      mpi_is_flip_endian=other.mpi_is_flip_endian;

      mpi_is_descramble=other.mpi_is_descramble;
    return *this;
}
