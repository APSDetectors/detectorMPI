#include "mpigatherdark.h"

mpiGatherDark::mpiGatherDark(mpiEngine *mpi,imageQueue &dq, imageQueue &fq) :
    mpiGather(mpi,dq,fq)
{



}






void mpiGatherDark::readPendingDatagrams()
{

        //we cam put mmore stuff here...
    // bit parent class is good enouigh.

    mpiGather::readPendingDatagrams();

}
