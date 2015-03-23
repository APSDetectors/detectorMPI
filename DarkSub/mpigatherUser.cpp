#include "mpigatherUser.h"

mpiGatherUser::mpiGatherUser(mpiEngine *mpi,imageQueue &dq, imageQueue &fq) :
    mpiGather(mpi,dq,fq)
{



}






void mpiGatherUser::readPendingDatagrams()
{

        //we cam put mmore stuff here...
    // bit parent class is good enouigh.

    mpiGather::readPendingDatagrams();

}
