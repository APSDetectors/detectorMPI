
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef mpiUser_H
#define mpiUser_H

#include "mpiengine.h"
#include "signalmessage.h"

#include "imm.h"

class mpiUser : public mpiEngine
{
public:
    mpiUser();
    ~mpiUser();

    //called in all ranks before we do cals on every iteration
    virtual void beforeCalcs(mpiBcastMessage &message);
    // onluy called in RANK1, when we get new gui message. called by mpiScatter
    virtual void beforeFirstCalc(mpiBcastMessage &message);
    // called all ranks, every iteration of img calcs.
    virtual void afterCalcs(mpiBcastMessage &message);


virtual int parseMessage(mpiBcastMessage &message);




  virtual  int doImgCalcs(void);


void flipEndian(int whichimg);


    void subDark(int whichimg);
    void calcThresh(void);
    void reCalcThresh(void);
    void clearThresh(void);


    virtual int setupMPI2(int argc, char *argv[]);


    virtual void shutdownMPI2();

    void tableDescramble2(
              int outimg,
              int inimg);



    void removeOverscan(quint16 *image_out, quint16 *image_in,int pix_x, int pix_y, quint64 image_size_pixels);

    void descramble_image_jtw1(quint16 *image_out, quint16 *image_in, int imgx, int imgy,quint64 image_size_pixels);


     unsigned short *sdark_image;
     unsigned short *sthresh_image;
    double *square_image;
    unsigned short *temp_image;

    bool is_made_desc_table;

    imm my_imm;


    //!!make a table to see how desc works.1152 x 1000 in soze
    int desc_table[3000000];
    int inv_desc_table[3000000];
};

#endif // mpiUser_H
