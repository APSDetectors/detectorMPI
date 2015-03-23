
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



/**
  * Make this program separate from the QT project. in the command line do gcc -o
  *it will make n executable that streams images to a named pipe. the QT program will thejn rad the pipe.
  *it is fortewsting the pipe input of the qt program.
  * to compile and run:
  *
  *  g++ -o imgtest -DIMGGEN imageStreamTest.cpp
  *
  *imgtest 4> mypipename
  *imgtest 4>tfile.bin
  *imgtest 4 256 >
  *
  *geneerate  100 images, 1st 10 are darks.256 in size
  *imgtest 100 256 10 > file.bin
  */
#ifdef IMGGEN

// USE_MPI is defined only when we build the QT program. make sure it is NOT defined for gcc'ing the
// test parogram.


#include "stdlib.h"
#include "math.h"
#include "stdio.h"
#include "time.h"

void fourSquares(int size_x, int size_y, unsigned short *imgptr);
void writeToOutput(int size_x, int size_y,unsigned short *img);

void rings(int size_x, int size_y, unsigned short *imgptr);
void noiseImage(int size_x, int size_y,unsigned short *img);
void addImg(
        int size_x,
        int size_y,
        unsigned short *img1,
        unsigned short *img2,
        unsigned short *imgsum);

void ringsPhotons(int size_x, int size_y, double probab, unsigned short *imgptr);


char zzz[]="zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";


unsigned short imgdata[2000000];
unsigned short baseimage[2000000];
unsigned short backgndimage[2000000];

/*****************************************************************************
 *
 * write lots of test images to stdout
 *
 ****************************************************************************/


main(int argc, char *argv[])
{

    // arg0 is n images.
   // it streams to stdout.

int nimgs=1;
int size_x=1024;
int size_y=1024;
int nbackgnd = 0;
unsigned short *iptr = baseimage;

int imgtype =0;


    // seed the rand num gen
 srand (time(NULL));

    //
    //Deal with arguments
    //

    if (argc>=2)
    {
        nimgs = atoi(argv[1]);
    }

    if (argc>=3)
    {
        size_x=atoi(argv[2]);
        size_y=size_x;
    }

    if (argc>=4)
    {
        nbackgnd=atoi(argv[3]);

    }


    if (argc>=5)
    {
        imgtype=atoi(argv[4]);

    }

    //
    // make back ground and signal image
    //


    //make back gnd img. 4 squares
    fourSquares(size_x, size_y, backgndimage);

    if (imgtype ==0)
    {
        //make concentric rung image.
        rings(size_x,size_y,baseimage);
        //add back gnd to rings
        addImg(size_x,size_y,backgndimage,baseimage,baseimage);
    }

    //
    // calculate all images. add noise to base image
    //



    for (int k=0;k<nimgs;k++)
    {
        //
        //create new image
        //
        if (k<nbackgnd)
        {
            fprintf(stderr,"Creating dark image %d of %d \n",k,nbackgnd);


        }
        else
        {
           fprintf(stderr,"Creating signal image %d of %d \n",k,nimgs);

        }

        if (imgtype==0)
        {
            noiseImage(size_x,size_y, imgdata);
            if (k>nbackgnd)
                addImg(size_x,size_y, imgdata,baseimage,imgdata);
            else
                addImg(size_x,size_y, imgdata,backgndimage,imgdata);
        }

        if (imgtype==1)
        {
            noiseImage(size_x,size_y, imgdata);

            addImg(size_x,size_y, imgdata,backgndimage,imgdata);

            if (k>=nbackgnd)
            {
                ringsPhotons(size_x, size_y,.2,baseimage);
                addImg(size_x,size_y, imgdata,baseimage,imgdata);
            }


        }


        if (imgtype==2)
        {
            noiseImage(size_x,size_y, imgdata);

            addImg(size_x,size_y, imgdata,backgndimage,imgdata);

            if (k>=nbackgnd)
            {
                ringsPhotons(size_x, size_y,.1,baseimage);
                addImg(size_x,size_y, imgdata,baseimage,imgdata);
            }


        }

        if (imgtype==3)
        {
            noiseImage(size_x,size_y, imgdata);

            addImg(size_x,size_y, imgdata,backgndimage,imgdata);

            if (k>=nbackgnd)
            {
                ringsPhotons(size_x, size_y,.01,baseimage);
                addImg(size_x,size_y, imgdata,baseimage,imgdata);
            }


        }



        if (imgtype==4)
        {

            ringsPhotons(size_x, size_y,.2,imgdata);

        }

        if (imgtype==5)
        {

            ringsPhotons(size_x, size_y,.1,imgdata);

        }

        if (imgtype==6)
        {

            ringsPhotons(size_x, size_y,.01,imgdata);

        }










        writeToOutput(size_x, size_y,imgdata);

    }
}//main


/*****************************************************************************
 *
 * make an image with noise of rectang. dist.
 *
 ****************************************************************************/


void noiseImage(int size_x, int size_y,unsigned short *img)
{


    int pix = 0;
   for ( int y =0;y<size_y;y++)
       for (int  x =0;x<size_x;x++)
   {


       img[pix]=(rand()%8192) - 4096;

      pix++;
   }

}


/*****************************************************************************
 *
 * write img to stdout
 *
 ****************************************************************************/


void writeToOutput(int size_x, int size_y,unsigned short *img)
{
    //
    //write image to pipe
    //


    int numpix = size_x*size_y;


    //write zzz's
    fwrite(zzz,1,64,stdout);
    //write image size x,y
    fwrite(&size_x,4,1,stdout);
    fwrite(&size_y,4,1,stdout);
    //
    //write image data
   //

    int nwrite = 0;
    int total_write = 0;
    do
    {
        nwrite = fwrite(img+total_write, 2,1024,stdout);
        total_write += nwrite;
    }while(total_write<numpix);

    fflush(stdout);
}


/*****************************************************************************
 *
 * add two imgs
 *
 ****************************************************************************/



void addImg(
        int size_x,
        int size_y,
        unsigned short *img1,
        unsigned short *img2,
        unsigned short *imgsum)
{

    int pix = 0;
    int numpix = size_x*size_y;

    for (pix=0;pix<numpix;pix++)
        imgsum[pix]=img1[pix]+img2[pix];

}


/*****************************************************************************
 *
 * mak image with 4 squares, or quadrants
 *
 ****************************************************************************/


void fourSquares(int size_x, int size_y, unsigned short *imgptr)
{

    //
    //make an image with concentric rings lioke a powder diff.\
    // we call this base image. we add noise to it for each sent out image
    //
    int pval = 0;
    int  pix = 0;

    for ( int y =0;y<size_y;y++)
        for (int x =0;x<size_x;x++)
    {

            if (y<(size_y/2))
            {
                if (x<(size_x/2))
                {
                    pval=12000;
                }
                else
                {
                    pval=5000;
                }

            }
            else
            {
                if (x<(size_x/2))
                {
                    pval=18000;
                }
                else
                {
                    pval=9000;
                }

            }

            imgptr[pix]=(unsigned short)pval;

       pix++;
    }


}

/*****************************************************************************
 *
 * make img with concentric ruings
 *
 ****************************************************************************/


void rings(int size_x, int size_y, unsigned short *imgptr)
{
    int numpix = size_x*size_y;
    int k,x,y;
    double pval;
    unsigned short pvals;
    int pix;

    double dist;
    double center=size_x/2;





    //
    //make an image with concentric rings lioke a powder diff.\
    // we call this base image. we add noise to it for each sent out image
    //

     pix = 0;
    for ( y =0;y<size_y;y++)
        for ( x =0;x<size_x;x++)
    {
        //bar image
        //imgdata[pix]=(pix%32768) + (rand()%8192);

        //circle image
            dist = sqrt((x -center)*(x-center) + (y-center)*(y-center));
            pval = 1.0 + 0.5*cos(6.28 * dist/(center/8.0));

            pval = pval *8192;
            //unsigned short pvals = (unsigned short)pval +  (rand()%8192);

            imgptr[pix]=(unsigned short)pval;

       pix++;
    }



}



/*****************************************************************************
 *
 * make img with a few photons occurring on concentric ruings
 * same code as rings() above, but the rungs tell a prob. of a phton
 * probab is like xray flux, makes more photons for higher number.
 *from 0 to 1.0
 ****************************************************************************/


void ringsPhotons(int size_x, int size_y, double probab, unsigned short *imgptr)
{
    int numpix = size_x*size_y;
    int k,x,y;
    double pval;
    unsigned short pvals;
    int pix;

    double dist;
    double center=size_x/2;





    //
    //make an image with concentric rings lioke a powder diff.\
    // we call this base image. we add noise to it for each sent out image
    //

     pix = 0;
    for ( y =0;y<size_y;y++)
        for ( x =0;x<size_x;x++)
    {
        //bar image
        //imgdata[pix]=(pix%32768) + (rand()%8192);

        //circle image
            dist = sqrt((x -center)*(x-center) + (y-center)*(y-center));
            pval = 1.0 + 0.5*cos(6.28 * dist/(center/8.0));

            pval = pval *probab * 1000.0;
            int pvali=(int)pval;

            int random=rand()%1000;

            //unsigned short pvals = (unsigned short)pval +  (rand()%8192);

            if (pvali>random)
                imgptr[pix]= 8192;
            else
                imgptr[pix]= 0;

       pix++;
    }



}



#endif
