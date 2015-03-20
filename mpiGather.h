
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/


#ifndef mpiGather_H
#define mpiGather_H

#include "pipeReader.h"
#include "mpiengine.h"
#include "imagequeueitem.h"
class mpiGather: public QObject
{
    Q_OBJECT
public:
    mpiGather(mpiEngine *mpi,imageQueue &dq, imageQueue &fq);


    imageQueue &free_queue;
    imageQueue &data_queue;

    mpiEngine *my_mpi;
 mpiBcastMessage my_message;
public slots:
    virtual void getGuiSpecs(guiSignalMessage message);



// void Set_sw_trigger(enum TriggerMode trigger_mode, quint64 param1, quint64 param2, quint64 param3);
// void Stop_capture();

virtual void readPendingDatagrams();





   signals:
    void signaldataready_mpi(mpiSignalMessage message);


    /**imageQueue *data_queue,
           imageQueue *free_queue,
            bool is_lost_frames,
            quint64 data_block_size_pixels_mpi,
            quint16 frame_number_mpi,
            quint16 images_in_fifo_mpi,
            quint16 image_in_ptr_mpi,
            quint16 fifo_len_mpi,
            bool is_lost_frames_mpi,
            bool is_acc_darks
            );*/


public:
    virtual void callMPIStuff(void);
    virtual void beforeGather(void);

};

#endif // mpiGather_H
