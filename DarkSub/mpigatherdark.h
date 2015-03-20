#ifndef MPIGATHERDARK_H
#define MPIGATHERDARK_H


#include "mpiGather.h"
class mpiGatherDark : public mpiGather
{
public:
    mpiGatherDark(mpiEngine *mpi,imageQueue &dq, imageQueue &fq);

public slots:
    virtual void readPendingDatagrams();




};

#endif // MPIGATHERDARK_H
