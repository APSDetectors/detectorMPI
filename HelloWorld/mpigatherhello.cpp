#include "mpigatherhello.h"

mpiGatherHello::mpiGatherHello(mpiEngine *mpi,imageQueue &dq, imageQueue &fq) :
    mpiGather(mpi,dq,fq)
{



}






void mpiGatherHello::readPendingDatagrams()
{

        //we cam put mmore stuff here...
    // bit parent class is good enouigh.

    mpiGather::readPendingDatagrams();

}
