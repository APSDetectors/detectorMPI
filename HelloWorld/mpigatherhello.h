#ifndef mpiGatherHello_H
#define mpiGatherHello_H


#include "mpiGather.h"
class mpiGatherHello : public mpiGather
{
public:
    mpiGatherHello(mpiEngine *mpi,imageQueue &dq, imageQueue &fq);

public slots:
    virtual void readPendingDatagrams();




};

#endif // mpiGatherHello_H
