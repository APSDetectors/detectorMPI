
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "xpcsgather.h"

xpcsGather::xpcsGather(mpiEngine *mpi,imageQueue &dq, imageQueue &fq) :
    mpiGather(mpi,dq,fq)
{



}


void xpcsGather::beforeGather(void)
{

}



void xpcsGather::readPendingDatagrams()
{

        //we cam put mmore stuff here...
    // bit parent class is good enouigh.

    mpiGather::readPendingDatagrams();

}
