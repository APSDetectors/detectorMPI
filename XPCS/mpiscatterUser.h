
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef mpiScatterUser_H
#define mpiScatterUser_H

#include "mpiScatter.h"

class mpiScatterUser : public mpiScatter
{
public:
    mpiScatterUser(
            mpiEngine *mpi,
            imageQueue &free,
            imageQueue &data);

    ~mpiScatterUser();

public slots:
    virtual void    gotMPIGuiSettings(guiSignalMessage mes_);

protected:
    // called each time we defifo image from data queie on new image slot
    virtual void onDeFifo(imageQueueItem *item);
    // called just before we start defifo images on new image slot.
    virtual void beforeDeFifo(void);
    //after defifo and scatter on new image, just before mpi calcs are starrted,
    virtual void afterDeFifo(void);

};

#endif // mpiScatterUser_H
