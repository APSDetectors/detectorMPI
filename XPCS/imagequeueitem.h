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


    // extra room in img data over x*y size shorts
    int extra_space_shorts;

    //length of image in shorts, this is sizex*sizey if pixel is a short
    int img_len_shorts;

    //size of malloced mem in shorts
    int mem_len_shorts;

    //number of images stuck into the img_data, if processing several frames at once.



    int numframes;
    // true of compressed data present
    bool is_compressed;
    //true of non compressed img present
    bool is_raw;

    //true uf there is a raw imm header present in the compressed img mem area
    bool is_raw_imm;

    //true if image is descrambled
    bool is_descrambled;

    //frame number from CIN board
   int frame_number;

    //software image input counter
    int inpt_img_cnt;

    //software image counter, set from the udp input counter. this is
    // gotten from the udp inpImgcounter on rank0, and bcast'ed to other ranks
    int img_cnt_udp;

    //approx num pixels in imm file... not calc for every image, but only in rank0
    //for exact num, look at imm header
    int imm_num_pixels;

    int num_pixels;

    //tell which rank processed image
    int processed_by_rank;

    //number of darks accum on that rank.. saveed until next accumm
    int rank_dark_accum_cnt;

    // num dark accum total, set in rank 0
    int dark_accum_tot;


    //when capturing filesl. here is the file capture counter
    int capture_counter;


    //error messages
    int error_code;
double system_clock;
    //this is data size we xfer to get specs. it is larger than above fields, but a nice
    //even number of shorts. for mpi xfers of the specs.
    enum {spec_len_short=128};

    //extra space so we dont have mem overruns w/ mpi xfers of spec data
    int zzz[128];



};


class imageQueueItem : public QObject
{
    Q_OBJECT
public:
    explicit imageQueueItem(QObject *parent = 0, quint64 imgsize_shorts=3000000);

    ~imageQueueItem();

    unsigned short *img_data;
   //!! unsigned char *comp_img_data;


    enum{extra_shorts = 65536};

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



class imageQueue : public QObject
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

    void resizeQueue(quint64 size_img_shorts);

    void checkQueue(void);
    //when we call fill queue, record how many items
    int num_fill_items;
signals:

public slots:

protected:
    //this is fifo, a curc buffer of len max_queue_mem_size.
     //we have read and write ptr, and calcs for empty and full
    imageQueueItem **queue;
    // if we call fill queue, it puts list of all img ojgs here, as well as in
    //queue. whem we empty queue, we dont dequeue, as imgs may be somewhere else
    // off queue, and there may be bad ptrs somewhere, where img is queued twice
    //by mistake. so on empty qieie, we delete all in imglist. and just set
    // read/write ptrs to 0.
    imageQueueItem **imglist;

    //num items in queue
    volatile int queue_length;
    //max num items in q- must be power of 2
    volatile int max_queue_length;
    volatile int max_queue_length2;

    // read /write counters extra bits
    volatile int write_location_extra;
    volatile int read_location_extra;
    //read write counters
    volatile int write_location;
    volatile int read_location;
    // full empty flags
    volatile bool is_empty;
    volatile bool is_full;


    enum {     max_queue_mem_size=4096};

void calcEmptyFull(void);

};

#endif // IMAGEQUEUEITEM_H
