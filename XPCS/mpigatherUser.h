
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef mpiGatherUser_H
#define mpiGatherUser_H


#include "mpiGather.h"
class mpiGatherUser : public mpiGather
{
public:
    mpiGatherUser(mpiEngine *mpi,imageQueue &dq, imageQueue &fq);

    // call this after mpi gets BCast message, gui settings over bcast.
    // called on every image returned from mpi, just before we gather.
    //
    virtual void beforeGather();
public slots:
    virtual void readPendingDatagrams();





};

#endif // mpiGatherUser_H
