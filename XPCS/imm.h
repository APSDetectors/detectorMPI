
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/




#ifndef IMM_H
#define IMM_H
#include <QObject>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <math.h>


#include <string.h>
#include <unistd.h>


#include "imm_header.h"


class imm
{
public:
    imm(int mb);
    ~imm();

    void fillHeaderZZFF(immHeader *h);


    qint32 rawToCompIMM(
        unsigned char* raw_image,
        qint32 raw_bytes,// tottal bytes
        qint32 raw_precision,//num bytes per pixel
        qint32 raw_x_pixels,
        qint32 raw_y_pixels,
        qint32 raw_timestamp,
        qint32 max_bytes,
        unsigned char* IMM_image,
        qint32 *IMM_bytes);



    void rawToIMM(
        unsigned char* raw_image,
        int raw_bytes,// tottal bytes
        int raw_precision,//num bytes per pixel
        int raw_x_pixels,
        int raw_y_pixels,
        int threshold,
        int raw_timestamp,
        int img_index,
        unsigned short* IMM_image,
        int *IMM_bytes);


    void IMMtoRaw(
            unsigned char* imm_image,
            int max_outshorts,
            unsigned short* outraw_image,
            int *size_x,
            int *size_y,
            int *num_nonzr_pix,
            int *corecotick);

   int max_image_bytes;
   unsigned char *imm_buffer;

   //num bytes in imm iamge we just compressed...
   int  imm_num_bytes;
   int imm_num_pixels;
};

#endif // IMM_H
