
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpimesgrecvr.h"

mpiMesgRecvr::mpiMesgRecvr(QObject *parent,mpiEngine *mpi) :
    mpiCalcRunner(parent,mpi)
{
}


void mpiMesgRecvr::run()
{

    printf("Waiting for messages- Rank = %d\n",my_mpi->rank);
    fflush(stdout);
        my_mpi->waitSentMessages();
}
