
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpiUser.h"
#include <QTime>

mpiUser::mpiUser():
    mpiEngine(),
    my_imm(3000000)
{

    //
    // set up the public MPI memory, images that are shared between ranks
    //

    // 4 short images.
    pub_sh_img_nimgs=4;

    // room for 1k image, room for image specs, and 128k extra to boot- room for imm header
    pub_sh_img_size_bytes=1152*1024*sizeof(short) + sizeof(imageSpecs) + 1024*1024;

    //if we pass several images to a rank, we line them up by img_spacing_shorts.
    //this is an offset from public_short_image[n] pointer  to an image.
     img_spacing_shorts=pub_sh_img_size_bytes/sizeof(short);


    // TWP double iamge to store the dark acc image.
     // 2nd image is for storing the acc noise image
      pub_db_img_nimgs=2;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
      pub_db_img_size_bytes=1152*1024*sizeof(double) + 1024*1024;
      img_spacing_doubles =  pub_db_img_size_bytes/sizeof(double);


      is_made_desc_table=false;
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
     printTrace("mpiUser::beforeCalcs");
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
 message.mpi_is_descramble=message.gui.is_descramble;
 message.mpi_is_flip_endian=message.gui.is_flipendian;

//is_made_desc_table=false;
 }


 //
 // called by mpiScatter AFTER a new iomage comes in from detector,  after we dequeue it, after images
 // are scattered to rank, but just BEFORE we kick off mpi calcs. We set up broadcast message here.
 //
  void mpiUser::beforeFirstCalc(mpiBcastMessage &message)
  {
      printTrace("mpiUser::beforeFirstCalc");
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

    is_made_desc_table=false;
    message.mpi_is_descramble=message.gui.is_descramble;

    message.mpi_is_flip_endian=message.gui.is_flipendian;

  }

 //
 // Called in mpiGather, after all ranks have processed images, but BEFORE have gathered images to final rank.
 //
 void  mpiUser::afterCalcs(mpiBcastMessage &message)
 {
     printTrace("mpiUser::afterCalcs");
 }


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

int mpiUser::setupMPI2(int argc, char *argv[])
{


    printTrace("mpiUser::setupMPI2");
    temp_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];

    sdark_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];

    sthresh_image= new unsigned short[pub_sh_img_size_bytes/sizeof(short)];


    //for xpcs
    square_image= new double[pub_db_img_size_bytes/sizeof(double)];
}




void mpiUser::shutdownMPI2()
{

    printTrace("mpiUser::shutdownMPI2");

    delete[] sdark_image;
    delete[] sthresh_image;
    delete[] temp_image;


    delete[] square_image;


}





void mpiUser::calcThresh(void)
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

void mpiUser::reCalcThresh(void)
{




    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("mpiUser::reCalcThresh");

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




void mpiUser::clearThresh(void)
{




    int imgx=my_message.imgspecs.size_x;
        int imgy=my_message.imgspecs.size_y;
    printTrace("mpiUser::clearThresh");

    int numpix = imgx*imgy;

    for (int m=0;m<numpix;m++)
    {

         sdark_image[m]=0;
         sthresh_image[m]=0;

    }

}



//called by all ranks after Bcast, so local class vars match message.
// this gets called buy ALL ranks after gui is updated, and when we have new iamges.
int mpiUser::parseMessage(mpiBcastMessage &message)
{

    printTrace("mpiUser::parseMessage");
    // my_message gets updated from message  in mpiEngine::parseMessage


    // we could add stuff here, but we ar eOK. see the source in mpiEngine to make sure
    // called when we update gui, and also on new images...
    // this gets called buy ALL ranks after gui is updated, and when we have new iamges.
    mpiEngine::parseMessage(message);

    // to allow instant update of thresholds
    if (my_message.gui.command==message.gui.update_thresh)
    {
        printTrace("mpiUser::parseMessage- updated thresholds");
        reCalcThresh();
    }

    // if we send new dark accum specs, clear the short dark iamge.
    // doub dark is cleared in parent class in parseMessage
    if (my_message.mpi_accum_specs)
    {
        printTrace("mpiUser::parseMessage- clearing thresholds, setup for accum new darks");

        // clear local mem for threshold and dark iage. both short data
        clearThresh();

        //Clear both public double iamges when we start new accum of darks.
        clearDarks(0);
        clearDarks(1);

        is_made_desc_table=false;
        rank_dark_accum_cnt=0;
    }



}



//input is public_short_image[0]
// descrrambles into public_short_image[1]
//dark sub into public_short_image[1]
// imm compress into public_short_image[2]

  int mpiUser::doImgCalcs(void)
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

        printTrace("mpiUser::doImgCalcs================================================");


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


            image_specs->is_descrambled=false;

            //we normally uyse img 0,. if desc is on, we outptu to img 1, so we use that one...



            if (my_message.mpi_is_descramble)
            {
                //take pub image putput to private image
                //partialDescramble(imgx, imgy,0, count,rank,numprocs);

                //we have a bunch of public_image[], all mapped to one contiguous mem
                //starting at public_iamge[0][0]. hwere is the map
                // public_image[0] - raw iamge for all ranks
                // publc_image[1] -descramled image for ll ranks



                image_specs->is_descrambled=true;

                printTrace("doImgCalcs,mpi_b_image_desc2 ");

                if (is_made_desc_table)
                    this->tableDescramble2(1,0);
                else
                {
                    //make desc table and inverse desc table
                    //also desc the image to pub image 1. we will end up desc. again after
                    //making table, so the desc operation here is ONLTY for makin gtable.
                    // desc image is not used.
                    this->descramble_image_jtw1(public_short_image[1],
                                            public_short_image[0],
                                            img_size_x,
                                            img_size_y,
                                            img_num_pixels);

                    //alter desc table to remove overscan lines
                    //!!if (message&mpi_b_remove_overscan)
                    this->removeOverscan(public_short_image[1],
                                             public_short_image[1],
                                             img_size_x,
                                             img_size_y,
                                             img_num_pixels);

                    //we will calc new desc table as we desc image
                    is_made_desc_table=true;

                    //now clear the image and do a table descranble. this is needed if overscan
                    //removal is on, so extra puixels at end of images will be zero.
                    //extra pixels happen becasue we remove overscan pixels, and shift whole image\
                    // to top left corner. so we end up w/ extra pixels on right and bottom.
                    clearImages(1);

                    //now do the actual desc, amd rem overscan
                    this->tableDescramble2(1,0);


                }
                //do this calc
                memcpy(public_short_image[0],public_short_image[1],img_num_pixels*sizeof(short));

            }


            if (my_message.mpi_is_flip_endian)
            {
                flipEndian(0);
            }

            my_message.gui.is_acq_dark_RBV=is_acc_darks;

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

            image_specs->dark_accum_tot=dark_integrate_counter;
            image_specs->rank_dark_accum_cnt=rank_dark_accum_cnt;

            if (my_message.mpi_is_makeraw_imm )
            {
                printTrace("mpiUser- making raw IMM\n");

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

                    //imm_bytes is img siize plus imm header size
                image_specs->is_raw_imm=true;
                image_specs->is_raw=false;
                image_specs->imm_num_pixels=img_num_pixels;
                image_specs->is_compressed=false;
                image_specs->num_pixels=my_message.imm_bytes/2;

                immHeader *immh = (immHeader*)temp_image;
                immh->systick=image_specs->system_clock;
                immh->elapsed=(double)(immh->systick) / 1000.0;
                immh->corecotick=image_specs->inpt_img_cnt;
                memcpy(public_short_image[0],temp_image,my_message.imm_bytes);
            }

            if (my_message.mpi_is_makecomp_imm )
            {
                printTrace("mpiUser- making comp IMM\n");
                my_imm.rawToCompIMM(
                        (unsigned char*)public_short_image[0],
                        img_num_pixels*sizeof(short),
                        2,
                        img_size_x,
                        img_size_y,
                        0,
                        image_specs->img_len_shorts * 2,//max size of space for data
                        (unsigned char*)temp_image,
                        &my_message.imm_bytes);


                image_specs->is_raw_imm=false;
                image_specs->is_raw=false;
                immHeader *imh = (immHeader*)((unsigned char*)temp_image);
                image_specs->imm_num_pixels=imh->dlen;
                image_specs->is_compressed=true;
                image_specs->num_pixels=my_message.imm_bytes/2;

                immHeader *immh = (immHeader*)temp_image;
                immh->systick=image_specs->system_clock;
                immh->elapsed=(double)(immh->systick) / 1000.0;
                immh->corecotick=image_specs->inpt_img_cnt;


                memcpy(public_short_image[0],temp_image,my_message.imm_bytes);

            }


            if (my_message.gui.which_img_view==1)
            {
                //adrk img
                //std img
                image_specs->is_raw_imm=false;
                image_specs->is_raw=true;

                image_specs->imm_num_pixels=0;
                image_specs->is_compressed=false;
                image_specs->num_pixels=img_num_pixels;


                for (int k=0;k<img_num_pixels;k++)
                {
                    public_short_image[0][k]=(unsigned short)(public_double_image[0][k]);
                }

            }

            if (my_message.gui.which_img_view==2)
            {
                //std img
                image_specs->is_raw_imm=false;
                image_specs->is_raw=true;

                image_specs->imm_num_pixels=0;
                image_specs->is_compressed=false;
                image_specs->num_pixels=img_num_pixels;


                for (int k=0;k<img_num_pixels;k++)
                {
                    public_short_image[0][k]=(unsigned short)(public_double_image[1][k]);
                }

            }

            if (my_message.gui.which_img_view==3)
            {
                //thresh img:

                image_specs->is_raw_imm=false;
                image_specs->is_raw=true;

                image_specs->imm_num_pixels=0;
                image_specs->is_compressed=false;
                image_specs->num_pixels=img_num_pixels;

                memcpy(public_short_image[0],sthresh_image,img_num_pixels*sizeof(short));
            }

            image_specs->processed_by_rank=rank;
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

            //writeImageRaw(public_double_image[0],img_num_pixels*sizeof(double),"AA_DsumImage",0);
            //writeImageRaw(public_double_image[1],img_num_pixels*sizeof(double),"AA_DsumSqImage",0);
            // now calc the std image and thresh hold iamges
            calcThresh();
           // writeImageRaw(public_double_image[0],img_num_pixels*sizeof(double),"AA_DAveImage",0);
            //writeImageRaw(public_double_image[1],img_num_pixels*sizeof(double),"AA_DStdImage",0);

            //writeImageRaw(sdark_image,img_num_pixels*sizeof(short),"AA_UDarkImage",0);
            //writeImageRaw(sthresh_image,img_num_pixels*sizeof(short),"AA_UThreshImage",0);

            //writeImageRaw(square_image,img_num_pixels*sizeof(double),"AA_DVarianceImg",0);

            //is_finish_darks=false;
        }





        printTrace("mpiUser::doImgCalcs_______________________________________________");


return(1);
  }



  /*****************************************************************************************************
   *
   *
   *
   **************************************************************************************************/


  //in private, output piblic
  void mpiUser::flipEndian(int whichimg)
  {

      printTrace("flipEndian");

        int offset = 0;
        int count = img_num_pixels;

       int endcnt=offset+count;
       int m;
       unsigned short a,b;
       for (m=offset;m<endcnt;m++)
       {

           a=public_short_image[whichimg][m];
           a=a>>8;
           b=public_short_image[whichimg][m];
           b=(b&255)<<8;
           public_short_image[whichimg][m]=a|b;

       }
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
           if (public_short_image[whichimg][m]> sthresh_image[m] )
               public_short_image[whichimg][m]= public_short_image[whichimg][m]-sdark_image[m];
           else
               public_short_image[whichimg][m]=0;

       }
  }





  /*****************************************************************************************************
   *
   *
   *
   **************************************************************************************************/



  void mpiUser::tableDescramble2(
          int outimg,
          int inimg)
  {


  printTrace("tableDescramble2");
  quint16 *image_out = public_short_image[outimg];
  quint16 *image_in=public_short_image[inimg];
  int imgx = img_size_x;
  int imgy = img_size_y;
  quint64 image_size_pixels=img_num_pixels;



      int k;

  int mm,nn;

  //!!
  //printf("x %d y %d o %d c %d\n",pix_x, pix_y, offset,count);
      for (k=0;k<image_size_pixels;k++)
      {

          mm=desc_table[k];




          if (mm>=0 && mm<image_size_pixels)
              image_out[mm]=image_in[k];



      }
  }





  /*****************************************************************************************************
   *
   *
   *
   **************************************************************************************************/

  void mpiUser::removeOverscan(quint16 *image_out, quint16 *image_in,int pix_x, int pix_y, quint64 image_size_pixels)
  {
      printTrace("removeOverscan");

      int x,y,in_index,out_index;

      //
      //set up default values for overscan remove
      //


      // number of overscan columns in the top and bottom
      int os_colwidth=0;
      //columns on top/bottom half that have pixels we want to keep
      int data_colwidth=10;
      //rows in middle of image that are overscan, that we want to remove.
      int middle_height=2;


      //
      //here we have some rules as to how to set up the rem over
      // we can comment these out, and change them based on how we are using CCD
      //perhaps they should be PVs?
      //

      // ofor 960 mode we take out no middle lines.
      // for small mode we take out 2 middle lines
      if (pix_y>950)
         middle_height=2;
      else
          middle_height=2;



      int panel_width=data_colwidth+os_colwidth;
      int y_top_st=0;
      int y_top_end= (pix_y/2) - (middle_height/2);
      int y_bot_st=(pix_y/2) + (middle_height/2);
      int y_bot_end=pix_y;

      int y_st,y_end;

      bool is_copy;

      int y_out_offset=0;

      int m;

      int pub_img_size2=img_num_pixels-1;

      //clear desc table. we still inv desc rtable which we need here.
      for (m=0;m<pub_img_size2;m++)
          desc_table[m]=pub_img_size2-1;

      for (int topbottom=0;topbottom<2;topbottom++)
      {
          if (topbottom==0)
          {
              y_st=y_top_st;
                  y_end=y_top_end;
                  y_out_offset=0;
           }
            else
          {
                y_st=y_bot_st;
                y_end=y_bot_end;
                y_out_offset=0-middle_height;
           }


          for (y=y_st;y<y_end;y++)
          {
              out_index= (y+y_out_offset)*pix_x;
              in_index=y*pix_x;
              is_copy=true;
              for (x=0;x<pix_x;x++)
              {
                 if (is_copy)
                 {
                     //if here we do NOT have an overscan pixel.

                      image_out[out_index]=image_in[in_index];
                      //find the original pix location from raw image from fccd.
                      //we are dealing w/ a descrambled image, so use inverse desc table
                      int  m=inv_desc_table[in_index];

                      //we reqrite the descrambling table, so we can go from a raw image
                      // to a completely descrambled and overscan-removed image in one step
                      desc_table[m]=out_index;

                 }
                 else
                 {
                     //if here we DO have overscan pixel
                 }

                  in_index++;

                  if (in_index%panel_width <data_colwidth)
                  {
                      out_index++;
                      is_copy=true;



                  }
                  else
                      is_copy=false;

              }//for x
          }//for y
      }//for topbomomon


  }





  void mpiUser::descramble_image_jtw1(quint16 *image_out, quint16 *image_in, int imgx, int imgy,quint64 image_size_pixels)
  {

      printTrace("descramble_image_jtw1");

      if(!image_out || !image_in) return;


  //!!TJM
     // if (is_made_desc_table==false)
      //{
       //   descramble_image_maddog1(image_out, image_in,  image_size_pixels);
        //  return;
     // }

  //    if (is_made_desc_table==true)
    //  {
     //     descramble_image_maddog2(image_out, image_in,  image_size_pixels);
     //     return;
     // }


      quint64 inOffsetQuad0;
      quint64 inOffsetQuad1;
      quint64 inOffsetQuad2;
      quint64 inOffsetQuad3;
      quint64 outOffsetQuad0;
      quint64 outOffsetQuad1;
      quint64 outOffsetQuad2;
      quint64 outOffsetQuad3;

      //quint64 outOffset;
      //quint64 inOffset;

      quint64 image_height = imgy; // Total number of rows in image
      // Canonical Example: 2000
      quint64 image_width = imgx; // Total number of subcolumns = (Total number of fcrics on one side) x
      // (Total number of columns in fcric) x (number of real and vitural pixels in a column)
      // Canonical Example: 8 x 12 x 12 = 1152
      quint64 RealColumnPos=0,RealRowPos=0,posInBlock=0,posInMux=0,posInCol=0;
      //quint64 QuadPixelPos = 0;
      quint64 eof_word_cnt = 0; // end of fcric word count: i.e. number of times we see '0xFIFO' in the byte stream
      quint64 start_eof_counter = 0;
      quint64 quad = 0; //Where is each quadrant located in the tiff? It would help to have a visual sense.
      //!!TJM added numcolumns w/ JTW for other img sizes w/out overscan
      //!!TJM
      quint64 num_columns=image_width/96;
      quint64 num_columns_m1=num_columns-1;
      quint64 num_columns_x_twelve = num_columns * 12;

          for (quint64 m = 0; m < image_height; m++ )
          {
              for(quint64 n = 0; n < image_width - 1; n++ )
              {
                  // pixel index
                  quint64 index = n + (m*image_width);

                  if (eof_word_cnt < 4) //If eof_word_count 0,1,2,3
                      // WHY are we casting a fraction to an INT to truncate? There has to be a better way.
                  {
                      quad = (quint64)(( (index - eof_word_cnt)%16)/4); //How do we know that index > eof_word_count and we are not implicitly ?
                  }
                  else //If eof_word_count >=4, then we are past the 4 words we don't want to copy so the index is off by the 4 words we deleted.
                  {
                      quad = (quint64)(( (index - 4)%16)/4); //If eof_word_cnt > 4, pretend it is 4?
                  }

                  if(quad==0)     // when is zero for first 4 words, then 1 for next 4 words
                  {
                      /*
                                  for index = 0: RealColumPos = ((3-0)*144) + (0*12) + (11-0) = 443
                                  What is the block and what is the mutex? The example numbers I tried in
                                  this code are not giving any hints as to the abstraction.

                                  Can we calculate real column pos based on index? or n and m?
                                  */
                      // JTW change 144 to num_columns * 12
                      RealColumnPos = ((3-posInBlock) * (num_columns_x_twelve) ) + (posInMux*num_columns) + (num_columns_m1 - posInCol);

                      // When we are reading out the overscan pixels swap first and last column and move all others back of one position
                      //if( RealColumnPos%num_columns == 0 ){ //If RealColumPos = 0, 12, 24, 36, etc...
                      if(RealColumnPos%num_columns == 0 && num_columns == 12){
                          RealColumnPos = RealColumnPos + num_columns_m1;
                      }else{
                          RealColumnPos = RealColumnPos - 1;
                      }

                      if (eof_word_cnt < 4) //If eof_word_count 0,1,2,3
                      {
                          RealRowPos = (quint64)( (index - eof_word_cnt) / (2*image_width));
                      }
                      else //If eof_word_count >=4
                      {
                          RealRowPos = (quint64)( (index - 4) / (2*image_width));
                      }



                      //ImageColumPos = RealColumnPos;
                      //RealRowPos = RealRowPos;

                      // JTW for no OS mode add -1 to Q0 and Q1 and add +1 to Q2 and Q3

                      // perform quad0, quad1, quad2, and quad3 operation all here
                      inOffsetQuad0 = index;
                      if (num_columns == 12)
                          outOffsetQuad0 = (RealRowPos*image_width) + image_width - 1 - RealColumnPos;
                      else
                          outOffsetQuad0 = (RealRowPos*image_width) + image_width - 1 - RealColumnPos - 1;


                      inOffsetQuad1 = index + 4;
                      if (num_columns == 12)
                          outOffsetQuad1 = (RealRowPos*image_width) + image_width/2 - 1 - RealColumnPos;
                      else
                          outOffsetQuad1 = (RealRowPos*image_width) + image_width/2 - 1 - RealColumnPos - 1;

                      inOffsetQuad2 = index + 8;
                      if (num_columns == 12)
                          outOffsetQuad2 =  ((image_height - 1 - RealRowPos )*image_width) + RealColumnPos;
                      else
                          outOffsetQuad2 =  ((image_height - 1 - RealRowPos )*image_width) + RealColumnPos + 1;

                      inOffsetQuad3 = index + 12;
                      if (num_columns == 12)
                          outOffsetQuad3 = ((image_height - 1 - RealRowPos )*image_width) + RealColumnPos + image_width/2;
                      else
                          outOffsetQuad3 = ((image_height - 1 - RealRowPos )*image_width) + RealColumnPos + image_width/2 + 1;

                      // jtw - look for 0xF1F0 instead of 0xf0f1, because the bytes get swapped later in imageprocessor.cpp
                      // do not write the 4 eof words when you get to 0xf1f0,  These are not at the end of the data buffer.
                      if ( *(image_in + inOffsetQuad0) == 0xF1F0 )
                      {
                          start_eof_counter = 1;
                      }

                      if (start_eof_counter == 0)
                      {
                          // inOffset >= 0 && outOffset >= 0 has to be true because they are unsigned ints!
                          // If they are not supposed to be uints, we need to STOP implicit type casting.
                          if(inOffsetQuad0 < image_size_pixels && outOffsetQuad0 < image_size_pixels)
                          {
                              *(image_out + outOffsetQuad0) = *(image_in + inOffsetQuad0);
                              desc_table[inOffsetQuad0]=outOffsetQuad0;
                              inv_desc_table[outOffsetQuad0]=inOffsetQuad0;
                          }
                      }
                      else if ( (start_eof_counter == 1) && (eof_word_cnt < 4) )    // do not transfer 4 words, throw away f0f1 f2de adf0 0d01.
                      {
                          eof_word_cnt += 1;
                      }
                      else if ( (start_eof_counter == 1) && (eof_word_cnt = 4) )    // do not transfer 4 words, throw away f0f1 f2de adf0 0d01.
                      {
                          eof_word_cnt += 1;
                          if(inOffsetQuad0 < image_size_pixels && outOffsetQuad0 < image_size_pixels)
                          {
                              *(image_out + outOffsetQuad0) = *(image_in + inOffsetQuad0);
                              desc_table[inOffsetQuad0]=outOffsetQuad0;
                              inv_desc_table[outOffsetQuad0]=inOffsetQuad0;
                          }
                      }
                      else if ( (start_eof_counter == 1) && (eof_word_cnt > 4) )
                      {
                          if(inOffsetQuad0 < image_size_pixels && outOffsetQuad0 < image_size_pixels)
                          {
                              *(image_out + outOffsetQuad0) = *(image_in + inOffsetQuad0);
                              desc_table[inOffsetQuad0]=outOffsetQuad0;
                              inv_desc_table[outOffsetQuad0]=inOffsetQuad0;
                          }
                      }

                      if(inOffsetQuad0 < image_size_pixels && outOffsetQuad0 < image_size_pixels)
                      {
                          *(image_out + outOffsetQuad1) = *(image_in + inOffsetQuad1);
                          *(image_out + outOffsetQuad2) = *(image_in + inOffsetQuad2);
                          *(image_out + outOffsetQuad3) = *(image_in + inOffsetQuad3);

                          desc_table[inOffsetQuad1]=outOffsetQuad1;
                          desc_table[inOffsetQuad2]=outOffsetQuad2;
                          desc_table[inOffsetQuad3]=outOffsetQuad3;

                          inv_desc_table[outOffsetQuad1]=inOffsetQuad1;
                          inv_desc_table[outOffsetQuad2]=inOffsetQuad2;
                          inv_desc_table[outOffsetQuad3]=inOffsetQuad3;

                      }

                      // can we do better??
                      if ( (eof_word_cnt < 1) || (eof_word_cnt > 4) )     // do not move output pointer during 4 eof words
                      {
                          if (posInBlock == 3)
                          {
                              posInBlock = 0;
                              if (posInMux == 11)
                              {
                                  posInMux = 0;
                                  if (posInCol == num_columns_m1) posInCol = 0;
                                  else                posInCol = posInCol + 1;

                              }
                              else posInMux = posInMux + 1;
                          }
                          else posInBlock = posInBlock + 1;
                      }
                  }   // quad == 0
              }       // n or colunm
          }           // m or row





  }

