#ifndef MPISCATTERDARK_H
#define MPISCATTERDARK_H

#include "mpiScatter.h"

class mpiScatterDark : public mpiScatter
{
public:
    mpiScatterDark(
            mpiEngine *mpi,
            imageQueue &free,
            imageQueue &data);

    ~mpiScatterDark();

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

#endif // MPISCATTERDARK_H
