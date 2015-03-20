
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/




#ifndef MPICALCRUNNER_H
#define MPICALCRUNNER_H

#include <QThread>
#include "mpiengine.h"

class mpiCalcRunner : public QThread
{
    Q_OBJECT
public:
    explicit mpiCalcRunner(QObject *parent = 0,mpiEngine* mpi=0);
    
    volatile bool is_running;

    mpiEngine* my_mpi;
protected:
    virtual void run();

signals:
    
public slots:
    
};

#endif // MPICALCRUNNER_H
