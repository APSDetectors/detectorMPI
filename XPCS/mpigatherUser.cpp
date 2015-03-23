
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpigatherUser.h"

mpiGatherUser::mpiGatherUser(mpiEngine *mpi,imageQueue &dq, imageQueue &fq) :
    mpiGather(mpi,dq,fq)
{



}


void mpiGatherUser::beforeGather(void)
{

}



void mpiGatherUser::readPendingDatagrams()
{

        //we cam put mmore stuff here...
    // bit parent class is good enouigh.

    mpiGather::readPendingDatagrams();

}
