

#include "mpiUser.h"

mpiUser::mpiUser():
    mpiEngine()
{

    //
    // set up the public MPI memory, images that are shared between ranks
    //

    // one short image.
    pub_sh_img_nimgs=1;

    // room for 1k image, room for image specs, and 1k extra to boot.
    pub_sh_img_size_bytes=1024*1024*sizeof(short) + sizeof(imageSpecs) + 1024;

    //if we pass several images to a rank, we line them up by img_spacing_shorts.
    //this is an offset from public_short_image[n] pointer  to an image.
     img_spacing_shorts=pub_sh_img_size_bytes/sizeof(short);


    // one double iamge to store the dark acc image.
      pub_db_img_nimgs=1;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
      pub_db_img_size_bytes=1024*1024*sizeof(double);
      img_spacing_doubles =  pub_db_img_size_bytes/sizeof(double);

}

mpiUser::~mpiUser()
{

}


//
// called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
// are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
//
 void mpiUser::beforeCalcs(mpiBcastMessage &message)
 {
     message.mpi_image=false;

    message.mpi_is_print_trace=message.gui.is_print_trace;
     is_print_trace=message.mpi_is_print_trace;

     message.mpi_save_mem_file=message.gui.is_save_mpi_rams;

     message.frame_number_mpi++;
     message.img_calc_counter+=message.num_images_to_calc;

     if (rank==0)
         message.image_counter_rank0++;



 }


 //
 // called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
 // are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
 //
  void mpiUser::beforeFirstCalc(mpiBcastMessage &message)
  {
      message.mpi_image=false;
      message.mpi_accum_darknoise =message.gui.is_acq_dark;
      message.mpi_sub_dark=message.gui.is_sub_dark;
     message.mpi_is_print_trace=message.gui.is_print_trace;
      is_print_trace=message.mpi_is_print_trace;

      message.mpi_save_mem_file=message.gui.is_save_mpi_rams;

      message.imgspecs.size_x=message.gui.size_x;
      message.imgspecs.size_y=message.gui.size_y;
     message.imgspecs.size_pixels = message.imgspecs.size_x * message.imgspecs.size_y;
     message.data_block_size_pixels_mpi=message.imgspecs.size_pixels;
     //message.img_spacing_shorts=message.data_block_size_pixels_mpi;

     message.num_dark_images_to_accum=message.gui.num_dark;

     message.frame_number_mpi=0;

     message.img_calc_counter=0;
     message.image_counter_rank0=0;


     message.mpi_is_limit_nproc=message.gui.is_limit_max_proc;
     message.mpi_max_num_proc=message.gui.max_num_procs;



  }

 //
 // Called in mpiGather, after all ranks have processed images, but BEFORE have gathered images to final rank.
 //
 void  mpiUser::afterCalcs(mpiBcastMessage &message)
 {

 }


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

int mpiUser::setupMPI2(int argc, char *argv[])
{



    sdark_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];

}




void mpiUser::shutdownMPI2()
{

    printTrace("mpiUser::shutdownMPI2");

    delete[] sdark_image;



}





void mpiUser::calcThresh(void)
{


    double thresh;
    unsigned short sthresh;
    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("calcThresh");

    int numpix = imgx*imgy;

    for (int m=0;m<numpix;m++)
    {

         sdark_image[m]=(unsigned short)public_double_image[0][m];

    }

}





//input is public_short_image[0]
// descrrambles into public_short_image[1]
//dark sub into public_short_image[1]
// imm compress into public_short_image[2]

  int mpiUser::doImgCalcs(void)
   {


        printTrace("mpiUser::doImgCalcs");

        // if we send new dark accum specs, clear the short dark iamge.
        // doub dark is cleared in parent class in parseMessage
        if (my_message.mpi_accum_specs)
        {
            calcThresh();
        }

                //get img specs for public  iamge 0

        if (is_calc_image)
        {
            //get specs for pub img 0
            getImageSpecs(0);


            if (is_print_trace)
            {
                printf("Rank %d got Img num %d\n",rank,image_specs->inpt_img_cnt);
            }



            if (my_message.mpi_save_mem_file && is_calc_image)
            {
                writeImageRaw(public_short_image[0],pub_sh_img_size_bytes*sizeof(short) ,"PubImg",0);
            }




            //accum pub img 0 into dark accum... of needed.
            accumDarkStd(0,0);

            if (my_message.mpi_sub_dark && is_calc_image)
            {


                //0 is offset from top of image, 1 is publc_image[1]
                printTrace("doImgCalcs, mpi_b_sub_dark");
                subDark(0);


            }

    }

        //must be outside of the if_calc_iamge
        // this must execute on every rank regardless if we have img.
        // has barriers and fences etc.

        if (is_finish_darks)
        {

            combineDarkStd(0);

            calcThresh();
            is_finish_darks=false;
        }



return(1);
  }






   /*****************************************************************************************************
    *
    *
    *
    **************************************************************************************************/

  //in private, output piblic
  void mpiUser::subDark(int whichimg)
  {
      printTrace("subDark");



       int endcnt=img_num_pixels;
       int m;
       for (m=0;m<endcnt;m++)
       {
           if (public_short_image[whichimg][m]>sdark_image[m])
               public_short_image[whichimg][m]=public_short_image[whichimg][m]-sdark_image[m];
           else
               public_short_image[whichimg][m]=0;

       }
  }

