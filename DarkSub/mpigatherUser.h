#ifndef MPIGATHERUser_H
#define MPIGATHERUser_H


#include "mpiGather.h"
class mpiGatherUser : public mpiGather
{
public:
    mpiGatherUser(mpiEngine *mpi,imageQueue &dq, imageQueue &fq);

public slots:
    virtual void readPendingDatagrams();




};

#endif // MPIGATHERUser_H
