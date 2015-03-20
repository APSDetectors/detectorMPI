
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpixpcs.h"

mpiXpcs::mpiXpcs():
    mpiEngine(),
    my_imm(3000000)
{

    //
    // set up the public MPI memory, images that are shared between ranks
    //

    // ONE short images.
    pub_sh_img_nimgs=1;

    // room for 1k image, room for image specs, and 1k extra to boot.
    pub_sh_img_size_bytes=1024*1024*sizeof(short) + sizeof(imageSpecs) + 1024;

    //if we pass several images to a rank, we line them up by img_spacing_shorts.
    //this is an offset from public_short_image[n] pointer  to an image.
     img_spacing_shorts=pub_sh_img_size_bytes/sizeof(short);


    // TWP double iamge to store the dark acc image.
     // 2nd image is for storing the acc noise image
      pub_db_img_nimgs=2;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
      pub_db_img_size_bytes=1024*1024*sizeof(double);
      img_spacing_doubles =  pub_db_img_size_bytes/sizeof(double);

}

mpiXpcs::~mpiXpcs()
{

}


//
// called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
// are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
//
 void mpiXpcs::beforeCalcs(mpiBcastMessage &message)
 {
     message.mpi_image=false;

    message.mpi_is_print_trace=message.gui.is_print_trace;
     is_print_trace=message.mpi_is_print_trace;

     message.mpi_save_mem_file=message.gui.is_save_mpi_rams;

     message.frame_number_mpi++;
     message.img_calc_counter+=message.num_images_to_calc;

     if (rank==0)
         message.image_counter_rank0++;


     //!! fopr xpcs
     message.mpi_num_std_thresh=message.gui.num_std_thresh;

     message.mpi_is_makecomp_imm=message.gui.is_comp_imm;
     message.mpi_is_makeraw_imm=message.gui.is_raw_imm;

     message.mpi_sub_dark=message.gui.is_sub_dark;


 }


 //
 // called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
 // are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
 //
  void mpiXpcs::beforeFirstCalc(mpiBcastMessage &message)
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


     //!! for XPCS below
     message.mpi_is_makecomp_imm=message.gui.is_comp_imm;
    message.mpi_is_makeraw_imm=message.gui.is_raw_imm;

    message.mpi_num_std_thresh=message.gui.num_std_thresh;


  }

 //
 // Called in mpiGather, after all ranks have processed images, but BEFORE have gathered images to final rank.
 //
 void  mpiXpcs::afterCalcs(mpiBcastMessage &message)
 {

 }


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

int mpiXpcs::setupMPI2(int argc, char *argv[])
{


    temp_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];

    sdark_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];

    sthresh_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];


    //for xpcs
    square_image= new double[pub_db_img_size_bytes/sizeof(double)];
}




void mpiXpcs::shutdownMPI2()
{

    printTrace("mpiXpcs::shutdownMPI2");

    delete[] sdark_image;
    delete[] sthresh_image;
    delete[] temp_image;


    delete[] square_image;


}





void mpiXpcs::calcThresh(void)
{




    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("calcThresh");

    int numpix = imgx*imgy;

    double std;
    double ndi=(double)num_dark_integrate;

    double multfact = 1.0 / (ndi);
    double thresh;
    unsigned short sthresh;

    for (int m=0;m<numpix;m++)
    {
        //E[x]
        public_double_image[0][m]=public_double_image[0][m]*multfact;
        //E[x*x]
        public_double_image[1][m]=public_double_image[1][m]*multfact;
        double mu=public_double_image[0][m];
        double mu2=public_double_image[1][m];
        double var=mu2 - mu*mu;
        square_image[m]=var;
        std=sqrt(var);
        public_double_image[1][m]=std;

        thresh = my_message.mpi_num_std_thresh * std + mu;
        sthresh = (unsigned short)thresh;

         sdark_image[m]=(unsigned short)mu;


         sthresh_image[m]= sthresh;

    }

}

void mpiXpcs::reCalcThresh(void)
{




    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("calcThresh");

    int numpix = imgx*imgy;

    double std;
    double ndi=(double)num_dark_integrate;

    double multfact = 1.0 / (ndi);
    double thresh;
    unsigned short sthresh;

    for (int m=0;m<numpix;m++)
    {
        double mu=public_double_image[0][m];
        double std=public_double_image[1][m];


        thresh = my_message.mpi_num_std_thresh * std + mu;
        sthresh = (unsigned short)thresh;

         sdark_image[m]=(unsigned short)mu;


         sthresh_image[m]= sthresh;

    }

}




void mpiXpcs::clearThresh(void)
{




    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("calcThresh");

    int numpix = imgx*imgy;

    for (int m=0;m<numpix;m++)
    {

         sdark_image[m]=0;
         sthresh_image[m]=0;

    }

}



//called by all ranks after Bcast, so local class vars match message.
// this gets called buy ALL ranks after gui is updated, and when we have new iamges.
int mpiXpcs::parseMessage(mpiBcastMessage &message)
{

    // my_message gets updated from message  in mpiEngine::parseMessage


    // we could add stuff here, but we ar eOK. see the source in mpiEngine to make sure
    // called when we update gui, and also on new images...
    // this gets called buy ALL ranks after gui is updated, and when we have new iamges.
    mpiEngine::parseMessage(message);

    // to allow instant update of thresholds
    if (my_message.gui.command==message.gui.update_thresh)
    {
        printTrace("mpiXpcs::parseMessage- updated thresholds");
        reCalcThresh();
    }

    // if we send new dark accum specs, clear the short dark iamge.
    // doub dark is cleared in parent class in parseMessage
    if (my_message.mpi_accum_specs)
    {
        printTrace("mpiXpcs::parseMessage- clearing thresholds, setup for accum new darks");

        // clear local mem for threshold and dark iage. both short data
        clearThresh();

        //Clear both public double iamges when we start new accum of darks.
        clearDarks(0);
        clearDarks(1);
    }



}



//input is public_short_image[0]
// descrrambles into public_short_image[1]
//dark sub into public_short_image[1]
// imm compress into public_short_image[2]

  int mpiXpcs::doImgCalcs(void)
   {

      //
      // we hav these variables on each new image at our disposal
      //

      //public_double_image[0] integrating dark, [1] is the integrating std iamge.
       //num_dark_integrate is how many darks we must integrate total.
         //num_images_to_calc is total images to calc across all ranks
     // img_num_pixels is number if pixels in the iamge.
     //img_size_x, img_size_y is size of iamge in pixels
     // public_short_image are new image, [0] is new iamge

   // my_message is the gui settings, and all mpi iamges. specs from image from detector is there as well.
    //        detector image sopecs also appear at end of piblic_short_image[0], after pidxels. getImageSpecs gets this into
   //  image_specs    - the specs of the image from the detector. also copied into my_message in super class.
    //is_calc_image - if true that means we have an image to calc in this rank.

        printTrace("mpiXpcs::doImgCalcs================================================");


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


            if (is_acc_darks)
            {
                //accum pub img 0 into dark accum... of needed.
                // 1st 0 is input imate or puclc_sh_image 0.
                //2nd arg0 is puiblic double image 0.

                accumDarkStd(0,0);


                //
                // now we must make a square image, and accum into the 2nd pub image.
                //

                // make a sqire image
                double dval;
                for (int p=0; p<img_num_pixels;p++)
                {
                    dval =(double) public_short_image[0][p];
                    dval = dval*dval;
                    square_image[p] = dval;
                }

                //accum square intop public_double_iamge[1]
                accumDarkStd(square_image,1);

            }

            if (my_message.mpi_sub_dark && is_calc_image)
            {


                //0 is offset from top of image, 1 is publc_image[1]
                printTrace("doImgCalcs, mpi_b_sub_dark");
                subDark(0);


            }



            if (my_message.mpi_is_makeraw_imm)
            {
                printTrace("mpiXpcs- making raw IMM\n");

                my_imm.rawToIMM(
                        (unsigned char*)public_short_image[0],
                        img_num_pixels*sizeof(short),
                        2,
                        img_size_x,
                        img_size_y,
                        0,
                        0,
                        image_specs->inpt_img_cnt,
                        temp_image,
                        &my_message.imm_bytes);

                memcpy(public_short_image[0],temp_image,img_num_pixels*sizeof(short));
            }

            if (my_message.mpi_is_makecomp_imm)
            {
                printTrace("mpiXpcs- making comp IMM\n");
                my_imm.rawToCompIMM(
                        (unsigned char*)public_short_image[0],
                        img_num_pixels*sizeof(short),
                        2,
                        img_size_x,
                        img_size_y,
                        0,
                        image_specs->inpt_img_cnt,
                        (unsigned char*)temp_image,
                        &my_message.imm_bytes);

                memcpy(public_short_image[0],temp_image,img_num_pixels*sizeof(short));

            }


            if (my_message.gui.which_img_view==1)
            {
                //adrk img
                //std img
                for (int k=0;k<img_num_pixels;k++)
                {
                    public_short_image[0][k]=(unsigned short)(public_double_image[0][k]);
                }

            }

            if (my_message.gui.which_img_view==2)
            {
                //std img
                for (int k=0;k<img_num_pixels;k++)
                {
                    public_short_image[0][k]=(unsigned short)(public_double_image[1][k]);
                }

            }

            if (my_message.gui.which_img_view==3)
            {
                //thresh img:


                memcpy(public_short_image[0],sthresh_image,img_num_pixels*sizeof(short));
            }


    }

        //must be outside of the if_calc_iamge
        // this must execute on every rank regardless if we have img.
        // has barriers and fences etc.

        //we get total average of all darks into pub dooub img 0.
        //is_finish_darks goes true when it is time to compute total sum,
        // that is, when accum is done. we must set to false ourselves.
        if (is_finish_darks)
        {
            combineDarkStd(0,1.0);
            // here we will get basic sum of all squared images into pub doub img 1.
            double noise_mult_factor =1.0;
            combineDarkStd(1,noise_mult_factor);

            writeImageRaw(public_double_image[0],img_num_pixels*sizeof(double),"AA_DsumImage",0);
            writeImageRaw(public_double_image[1],img_num_pixels*sizeof(double),"AA_DsumSqImage",0);
            // now calc the std image and thresh hold iamges
            calcThresh();
            writeImageRaw(public_double_image[0],img_num_pixels*sizeof(double),"AA_DAveImage",0);
            writeImageRaw(public_double_image[1],img_num_pixels*sizeof(double),"AA_DStdImage",0);

            writeImageRaw(sdark_image,img_num_pixels*sizeof(short),"AA_UDarkImage",0);
            writeImageRaw(sthresh_image,img_num_pixels*sizeof(short),"AA_UThreshImage",0);

            writeImageRaw(square_image,img_num_pixels*sizeof(double),"AA_DVarianceImg",0);

            is_finish_darks=false;
        }

        printTrace("mpiXpcs::doImgCalcs_______________________________________________");


return(1);
  }


   /*****************************************************************************************************
    *
    *
    *
    **************************************************************************************************/

  //in private, output piblic
  void mpiXpcs::subDark(int whichimg)
  {
      printTrace("subDark");



       int endcnt=img_num_pixels;
       int m;
       for (m=0;m<endcnt;m++)
       {
           if (public_short_image[whichimg][m]> sthresh_image[m] )
               public_short_image[whichimg][m]=1024 + public_short_image[whichimg][m]-sdark_image[m];
           else
               public_short_image[whichimg][m]=0;

       }
  }

