
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef MPIMESGRECVR_H
#define MPIMESGRECVR_H

#include "mpicalcrunner.h"

class mpiMesgRecvr : public mpiCalcRunner
{
    Q_OBJECT
public:
    explicit mpiMesgRecvr(QObject *parent = 0,mpiEngine* mpi=0);
protected:
    virtual void run();
signals:
    
public slots:
    
};

#endif // MPIMESGRECVR_H
