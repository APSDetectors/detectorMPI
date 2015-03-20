
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "imm.h"

imm::imm(int mb)
{
    max_image_bytes=mb;

    imm_buffer=new unsigned char[max_image_bytes];

    imm_num_pixels=0;

    imm_num_bytes=0;
}

imm::~imm()
{
    delete[] imm_buffer;
}



/******************************************************************************
 *
 *
 *****************************************************************************/


void imm::rawToIMM(
    unsigned char* raw_image,
    int raw_bytes,// tottal bytes
    int raw_precision,//num bytes per pixel
    int raw_x_pixels,
    int raw_y_pixels,
    int threshold,
    int raw_timestamp,
    int img_index,
    unsigned short* IMM_image,
    int *IMM_bytes)
{

    int k;
    int num_pixels;
    int pix_rec_bytes;
    int num_raw_pixels;
    unsigned short* raw_image_us;
    unsigned char* raw_image_uc;

    unsigned int *IMM_data_int32_t;
    immHeader *IMM_header;
    unsigned short* IMM_data_short;
    unsigned char *IMMptr;


    // point header to top of new memory
    IMM_header=(immHeader*)IMM_image;

    IMMptr = (unsigned char*)IMM_image;
    for (k=0;k<immHeader::header_size;k++)
        IMMptr[k] = 0;


    fillHeaderZZFF(IMM_header);

    //
    // Put some magic numbers in the header
    //
    IMM_header->cols=raw_x_pixels;
    IMM_header->rows=raw_y_pixels;

            // bytes per pixel
    IMM_header->bytes = raw_precision;
        // magic numbers
        IMM_header->immversion=immHeader::immver;
        IMM_header->mode = 2;
        IMM_header->compression=0;


        IMM_header->col_beg=0;
        IMM_header->col_end=raw_x_pixels-1;
        IMM_header->corecotick=raw_timestamp;

     IMM_data_short = (unsigned short*)(IMM_image+immHeader::header_size);


    //
    // Fill the pixel location section
    //


    raw_image_us = (unsigned short*)raw_image;
    raw_image_uc = raw_image;
    //raw_image_ul = (uint32_t*)raw_image;

    num_raw_pixels = raw_bytes/raw_precision;
    num_pixels=num_raw_pixels;

    //no need to copy the image from point a to point b... we already have it
    //pub img 0. we are just making a header.
    //memcpy((void*)IMM_data_short,(void*)raw_image_us,raw_bytes);




    IMM_header->dlen = num_pixels;
    // calculate the size of the new compressde image in bytes
    *IMM_bytes = immHeader::header_size;
    *IMM_bytes = *IMM_bytes + raw_bytes;

}




/******************************************************************************
 *
 *
 *****************************************************************************/


qint32 imm::rawToCompIMM(
    unsigned char* raw_image,
    qint32 raw_bytes,// tottal bytes
    qint32 raw_precision,//num bytes per pixel
    qint32 raw_x_pixels,
    qint32 raw_y_pixels,
    qint32 raw_timestamp,
    qint32 img_index,
    unsigned char* IMM_image,
    qint32 *IMM_bytes)
{

    qint32 k;
    qint32 num_pixels;
    qint32 pix_rec_bytes;
    qint32 num_raw_pixels;
    unsigned short* raw_image_us;
    unsigned char* raw_image_uc;

    unsigned short* pix_val_s;
    quint32 *IMM_data_int;
    immHeader *IMM_header;
    unsigned short* IMM_data_short;
    unsigned char *IMMptr;
    qint32 location;


    // get memory for new compressed image
    //*IMM_image = this->scratch_memory;

    // point header to top of new memory
    IMM_header=(immHeader*)IMM_image;

    IMMptr = IMM_image;
    for (k=0;k<immHeader::header_size;k++)
        IMMptr[k] = 0;

    fillHeaderZZFF(IMM_header);


    //
    // Put some magic numbers in the header
    //
    IMM_header->cols=raw_x_pixels;
    IMM_header->rows=raw_y_pixels;

            // bytes per pixel
    IMM_header->bytes = raw_precision;
        // magic numbers
        IMM_header->immversion=immHeader::immver;
        IMM_header->mode = 2;
        IMM_header->compression=6;



        IMM_header->row_beg=0;
        IMM_header->row_end=raw_y_pixels;

        IMM_header->col_beg=0;
        IMM_header->col_end=raw_x_pixels;
        IMM_header->corecotick=raw_timestamp;

    // IMM_data_short = (unsigned short*)(*IMM_image+compressed_header::header_size);
     IMM_data_int = (unsigned int*)(IMM_image+immHeader::header_size);


    //
    // Fill the pixel location section
    //


    raw_image_us = (unsigned short*)raw_image;
    raw_image_uc = raw_image;
    //raw_image_ul = (uint32_t*)raw_image;

    num_raw_pixels = raw_bytes/raw_precision;

    num_pixels=0;
    location = 0;

    pix_val_s=(unsigned short*)this->imm_buffer;


    // we alrday subtracted the dark and thresholedd...
    // just include non zero pixels in the compressed image
   int threshold = 0;



    if (raw_precision==2)
    {
        for (k=0;k<num_raw_pixels; k++)
        {
            if (*raw_image_us > threshold)
            {
                *IMM_data_int = location;
                *pix_val_s=*raw_image_us;
                IMM_data_int++;
                pix_val_s++;
                num_pixels++;
            }
            raw_image_us++;
            location ++;

        }
    }
    else//for char data
    {
        printf("mpiengine::rawToIMM, ERROR- only ushort data supported for fccd imm\n");
        fflush(stdout);
    }



    // fill in the pix value section of the IMM image. just copy...
    memcpy((void*)IMM_data_int,(void*)this->imm_buffer,num_pixels*sizeof(short));




    IMM_header->dlen = num_pixels;


    // calculate the size of the new compressde image in bytes
    *IMM_bytes = immHeader::header_size;
    *IMM_bytes = *IMM_bytes + sizeof(quint32)*num_pixels;
    *IMM_bytes = *IMM_bytes + sizeof(unsigned short)*num_pixels;

    return(num_pixels);

}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/


void imm::fillHeaderZZFF(immHeader *h)
{
    int k;

    for(k=0;k<immHeader::z_len;k++)
    {
        h->ZZZZ[k] = 0;
    }

    for(k=0;k<immHeader::f_len;k++)
    {
        h->FFFF[k] = 255;
    }

}



