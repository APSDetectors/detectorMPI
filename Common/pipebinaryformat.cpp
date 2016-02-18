
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include <stdio.h>
#include "pipebinaryformat.h"



char pipeBinaryFormat::zzz[]="zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";



pipeBinaryFormat::pipeBinaryFormat()
{
    message_code=0;
}


int pipeBinaryFormat::setMessage(int code)
{
    message_code = code;
}


int pipeBinaryFormat::checkErrors(int nread, FILE *f)
{
    //return 1 for break out of here...
   // printf("nread %d\n",nread);

    int err=ferror(f);
    if (err)
        printf("err = %d\n",err);

    if (message_code)
        return(message_code);


    return(err);
}


 int pipeBinaryFormat::readDataBlock(FILE *fp,imageQueueItem *item)
 {

       // read an image of this format:
     // unsigned shorts litte endian
     // start with 64 'Z' bytes
     // then have x size as a int, num of x puixels
     // y size as a int. num of y pixels
     //then bunch of shorts for the imate x*y pixels


     char zzz[64];
     int nread;
    int k;
    int stat = 0;


    //
    // Seawrch for zzzzzzz's , 64 zs in a row. we read 64 bytes, see if we have any z's.
    // if all z's we have found the beginning of image.
    // of some zs we read the rest of the 64 z's and hope we get them
   // if we dont find z's we seawrch until we find them
    //


    bool all_zzzs=false;

     bool is_found_zz=false;

     while(!is_found_zz)
     {
         //read in 64 bytes, search for 1 z.
         nread = fread(zzz,1,64,fp);





        if (checkErrors(nread,fp)!=0)
                 return(-1);


         //
         // Search for 64 z's
         //
         int first_z = -1;
         int N_zs=0;

         all_zzzs=true;
         for (k=0;k<64;k++)
         {
             if (first_z == -1 && zzz[k]=='z')
                     first_z=k;

             if (zzz[k]!='z')
                 all_zzzs=false;

             if (zzz[k]=='z')
                 N_zs++;

         }

         // we found 64 z's in a row so we are lined iup w/ data. now read the header and image data
         if (all_zzzs)
             is_found_zz=true;
         // we found some number of Z's, N_zs. Find the rest of them
         else if (first_z>0)
         {
             // num z's we found.
             int N_rest_of_zs=64-N_zs;

             nread = fread(zzz,1,N_rest_of_zs,fp);


                 if (checkErrors(nread,fp)!=0)
                     return(-1);



             all_zzzs=true;
             for (k=0;k<N_rest_of_zs;k++)
             {


                 if (zzz[k]!='z')
                     all_zzzs=false;

             }
             is_found_zz=all_zzzs;

         }


     }


     //
     // If we are here we have found the beginning of an image.
     // read x and y size as ints, then read in the image.
     //


     nread=fread(&item->specs->size_x,4,1,fp);


     if (checkErrors(nread,fp)!=0)
         return(-1);;


     nread=fread(&item->specs->size_y,4,1,fp);


     if (checkErrors(nread,fp)!=0)
         return(-1);

     nread=fread(&item->specs->num_pixels,4,1,fp);


     if (checkErrors(nread,fp)!=0)
         return(-1);






     nread=fread(&item->specs->frame_number,4,1,fp);


     if (checkErrors(nread,fp)!=0)
         return(-1);



     nread=fread(&item->specs->inpt_img_cnt,4,1,fp);


     if (checkErrors(nread,fp)!=0)
         return(-1);



     nread=fread(&item->specs->error_code,4,1,fp);


         if (checkErrors(nread,fp)!=0)
             return(-1);


     int n_s_pixels = item->specs->num_pixels;
     item->specs->img_len_shorts=n_s_pixels;

     if (n_s_pixels > item->specs->mem_len_shorts)
     {
         // make sure there is some head room.. that is why we have 1024, because fread is 1k
         item->specs->num_pixels = item->specs->mem_len_shorts-1024;
         item->specs->img_len_shorts = item->specs->mem_len_shorts-1024;
         n_s_pixels = item->specs->mem_len_shorts-1024;
     }

     int total_read=0;
     unsigned short *ptr = item->img_data;
     int num_to_read = 1024;
     do{
         if((n_s_pixels-total_read)> 1024) {
             num_to_read = 1024;
         }
         else {
             num_to_read = n_s_pixels - total_read;
         }
         nread= fread(ptr,sizeof(short),num_to_read,fp);


         if (checkErrors(nread,fp)!=0)
             return(-1);

         ptr+=nread;
         total_read+=nread;

     }while(total_read<n_s_pixels);




         return(0);

 }

 int pipeBinaryFormat::writeDataBlock(FILE *fp,imageQueueItem *item)
 {
     //
     //write image to pipe
     //

    unsigned short *imgdata = item->img_data;
    int size_x = item->specs->size_x;
    int size_y = item->specs->size_y;
    int num_shorts = item->specs->num_pixels;

    int cam_frame_number = item->specs->frame_number;
    int sw_frame_number = item->specs->inpt_img_cnt;
    int error_code = item->specs->error_code;

     //write zzz's
     fwrite(pipeBinaryFormat::zzz,1,64,fp);
     //write image size x,y
     fwrite(&size_x,4,1,fp);
     fwrite(&size_y,4,1,fp);
     fwrite(&num_shorts,4,1,fp);

     fwrite(&cam_frame_number,4,1,fp);
     fwrite(&sw_frame_number,4,1,fp);
     fwrite(&error_code,4,1,fp);


     //
     //write image data
    //

     int numpix = num_shorts;
     int nwrite = 0;
     int total_write = 0;
     do
     {
         nwrite = fwrite(imgdata+total_write, 2,1024,fp);
         total_write += nwrite;
     }while(total_write<numpix);

     fflush(fp);

 }
