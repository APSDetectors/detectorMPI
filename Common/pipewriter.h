
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef PIPEWRITER_H
#define PIPEWRITER_H

#include <QObject>
#include <QDir>
#include "signalmessage.h"
#include "tinytiff.h"
#include "imagequeueitem.h"
#include "pipebinaryformat.h"
#include "stdio.h"
#include "imm_header.h"


class pipeWriter : public QObject
{
    Q_OBJECT
public:
    explicit pipeWriter(
            imageQueue &dq,
            imageQueue &fq,
            QObject *parent = 0);
    
signals:
    // after we are done with new imqage we signal
    void finishedImage(mpiSignalMessage mes);

public slots:

    // when gui is updated...
    virtual void gotGuiSetting(guiSignalMessage mes);
    // on new image to write to file or pipe
    virtual void newImage(mpiSignalMessage img);

    virtual void openPipe(void);
    virtual void closePipe(void);
protected:

    volatile int capture_counter;

    pipeBinaryFormat pwrite;
    // tiff writing object
    tinytiff tifWriter;

    FILE *myfile;
    bool is_file_open;

    // image data - copy from mpi image message
    mpiBcastMessage imgdata;

    // lastest gui message settings
    guiMessageFields guisettings;

    // image queues for data and free iamges
    imageQueue &data_queue;
    imageQueue &free_queue;


    bool is_capture_started;
    int file_number;
    int init_frame_number;

    //output pipe- named linux pipe.
    FILE *pipe;
    bool is_pipe_open;

    QString lastfilename;

    int last_imm_buffer_number;

    QDir directory;
    
};

#endif // PIPEWRITER_H
