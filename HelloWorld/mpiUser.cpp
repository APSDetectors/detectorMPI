

#include "mpiUser.h"

mpiUser::mpiUser():
    mpiEngine()
{


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


     message.img_calc_counter+=message.num_images_to_calc;



 }


 //
 // called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
 // are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
 //
  void mpiUser::beforeFirstCalc(mpiBcastMessage &message)
  {
      message.mpi_image=false;

      is_print_trace=message.mpi_is_print_trace;


      message.imgspecs.size_x=message.gui.size_x;
      message.imgspecs.size_y=message.gui.size_y;
     message.imgspecs.size_pixels = message.imgspecs.size_x * message.imgspecs.size_y;
     message.data_block_size_pixels_mpi=message.imgspecs.size_pixels;
     //message.img_spacing_shorts=message.data_block_size_pixels_mpi;



     message.img_calc_counter=0;


  }









  int mpiUser::doImgCalcs(void)
   {


        printTrace("mpiUser::doImgCalcs");


                //get img specs for public  iamge 0

        if (is_calc_image)
        {
            //get specs for pub img 0
            //image_specs will have our data.
            getImageSpecs(0);


            //take the negative of the img.
            if (my_message.gui.is_negative)
                for (int k=0;k<img_num_pixels;k++)
                    public_short_image[0][k] = 65536-public_short_image[0][k];

    }



return(1);
  }





