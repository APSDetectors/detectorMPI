
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef pipeReader_H
#define pipeReader_H


#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QUdpSocket>
#include <QFile>
#include <QDir>

#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <stdio.h>
#include "pipebinaryformat.h"

#include "imagequeueitem.h"

#include "signalmessage.h"

//! [0]
//!
//!
//!
//!


class pipeReader : public QObject
{
    Q_OBJECT


   public:

    imageQueue &free_queue;
    imageQueue &data_queue;

    FILE *in_pipe;


    volatile quint32 inpt_img_cnt;


    pipeReader(
            imageQueue &free,
            imageQueue &data);
    ~pipeReader();


     void makeTestImage(void);


     static void onSignal(int param);

  public slots:
    virtual void openPipe(guiSignalMessage data);
    virtual void closePipe(void);
    virtual void streamTestImages(void);
     virtual void getPipeImages(void);
virtual void debugStreamImages(int numimages);

signals:
    void newImageReady(imageSignalMessage data);


protected:
    //counter of test images
    int test_frame_number;




    bool is_got_close_message;
    bool is_pipe_open;

    guiMessageFields guisettings;

    //tue if we lose frames
    bool is_lost_frames;


    //
    // current queue item, where we put data from udp
    //

    // extra buffer if we run out of queue space...
    imageQueueItem *bit_bucket;


};
#endif // pipeReader_H
