#ifndef IMAGEQUEUEITEM_H
#define IMAGEQUEUEITEM_H

#include <QObject>
 #include <QQueue>
#include <QMutex>

/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/


/**
 * @brief The imageQueueItem class
 *
 *This class represents an image on a queue. Image data is stored. also we can
 *have compressed version of image and several fields. the idea is to have a
 *free queue of empty images, and data queue of images to be processed.
 */

struct imageSpecs
{

    //x,y size in pixels of image.asumme ushort data type
    int size_x;
    int size_y;



    //length of image in shorts, this is sizex*sizey if pixel is a short
    int img_len_shorts;
    //number of images stuck into the img_data, if processing several frames at once.
    int numframes;

    //true of non compressed img present
    bool is_raw;



    //frame number from CIN board
   int frame_number;

    //software image input counter
    int inpt_img_cnt;


    //tell which rank processed image
    int processed_by_rank;

    //this is data size we xfer to get specs. it is larger than above fields, but a nice
    //even number of shorts. for mpi xfers of the specs.
    enum {spec_len_short=32};

    //extra space so we dont have mem overruns w/ mpi xfers of spec data
    char zzz[128];

};


class imageQueueItem : public QObject
{
    Q_OBJECT
public:
    explicit imageQueueItem(QObject *parent = 0, quint64 imgsize_shorts=3000000);

    ~imageQueueItem();

    unsigned short *img_data;
   //!! unsigned char *comp_img_data;


    imageSpecs *specs;

    //claer fields for empty image
    void clear(void);

    //construct num_items imgeQueueItems and put into queue Q.
    static void fillQueue(QQueue<imageQueueItem*> *Q, int num_items, quint64 size_img_shorts);
    // dequeue all items from Q, and destruct them. Leave Q alone, it must be destructed later.
    static void emptyQueue(QQueue<imageQueueItem*> *Q);




    
signals:
    
public slots:
    
};



class imageQueue : public QObject, QQueue<imageQueueItem*>
{
    Q_OBJECT
public:
    imageQueue();

    QMutex lock;

    void enqueue(imageQueueItem*);
    imageQueueItem* dequeue(void);
    bool dequeueIfOk(imageQueueItem **item);
    //construct num_items imgeQueueItems and put into queue Q.
    void fillQueue(int num_items, quint64 size_img_shorts);
    // dequeue all items from Q, and destruct them. Leave Q alone, it must be destructed later.
    int emptyQueue(void);
    int count();
    bool isEmpty();

    //when we call fill queue, record how many items
    int num_fill_items;
signals:

public slots:


};

#endif // IMAGEQUEUEITEM_H
