
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpicalcrunner.h"

mpiCalcRunner::mpiCalcRunner(QObject *parent, mpiEngine *mpi) :
    QThread(parent)
{
    my_mpi=mpi;
    is_running=true;

}
void mpiCalcRunner::run()
{

        my_mpi->mpiCalcLoop();
}
