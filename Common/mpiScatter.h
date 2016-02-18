
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef mpiScatter_H
#define mpiScatter_H


#include <QTime>

#include "mpiengine.h"
#include "imagequeueitem.h"
#include "signalmessage.h"



class mpiScatter : public QObject
{
    Q_OBJECT
    
public:
    explicit mpiScatter(
            mpiEngine *mpi,
            imageQueue &free,
            imageQueue &data);
    ~mpiScatter();



 public:

    //our image specs and gui settings stored here.


    mpiBcastMessage mpi_message;


    //--------------< Image Display >

    mpiEngine *my_mpi;

    imageQueue &free_queue;
    imageQueue &data_queue;

    //store fifolen to be sent to the streaminport
    quint32 fifolen;

    QTime disp_timer;
//temp storage of imges
       imageQueue temp_queue;

public slots:



    virtual void    gotMPIGuiSettings(guiSignalMessage mes_);

       virtual void newImage(imageSignalMessage mes_);
       virtual void lostImage();

 signals:


    virtual void connectInput(guiSignalMessage mes_);
       virtual void disconnectInput();

protected:

int signals_no_frames;
int num_queued_img_signals;
int num_imgs_de_fifoed;
int num_slots_called;

//limit scattering to x ranks... for debugging
//!!int rank_dbg_limit;
//!!bool is_rank_dbg_limit;
// num images to dequeue on new image based on num ranks.
 int num2dequeue;

 // num iamges to calc on ONE mpi kickoff, on ONE new iamge slot call.
 // usually set to 1 for slow images, for fast images, set to num ranks, for
 // one image per rank
 int img_calc_counter;

void   sendMPISetup(void);

virtual void onDeFifo(imageQueueItem *item);
virtual void beforeDeFifo(void);
virtual void afterDeFifo(void);

// because we dequeue images in groups at a time, but qt signal each new images,
// we keep track of QT signals so they don't pile up.
bool deQueueSignals(void);

//take frames off fifo and scatter to mpi ranks
// return number of iamges defifoed
int deFifoScatterImages(void);


int num_imgs_to_stream;

};

#endif // mpiScatter_H
