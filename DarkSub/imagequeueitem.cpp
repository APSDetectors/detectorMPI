
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "imagequeueitem.h"

imageQueueItem::imageQueueItem(QObject *parent, quint64 imgsize_shorts):
    QObject(parent)
{


    //img_data = new unsigned short[imgsize_shorts];
    img_data = (unsigned short*)malloc(imgsize_shorts*sizeof(short));
    // assume we get x4 compression. if not we lose data.. who cares
    //comp_img_data=new unsigned char[imgsize_shorts/4];
    //!!comp_img_data= (unsigned char*)malloc((imgsize_shorts)*sizeof(short));

    specs=(imageSpecs*)malloc(sizeof(imageSpecs));

    clear();

}


imageQueueItem::~imageQueueItem()
{
    //delete(img_data);
    free(img_data);
    //delete(comp_img_data);
    //!!free(comp_img_data);

    free(specs);
}

void imageQueueItem::clear(void)
{
    //x,y size in pixels of image.asumme ushort data type
     specs->size_x=0;
     specs->size_y=0;

    //length of image in shorts, this is sizex*sizey if pixel is a short
     specs->img_len_shorts=0;
    //number of images stuck into the img_data, if processing several frames at once.
     specs->numframes=0;
    // true of compressed data present
    //!!specs->is_compressed=false;
    //true of non compressed img present
     specs->is_raw=false;

     //!!specs->is_raw_imm=false;

     specs->frame_number = 0;

     //!!specs->is_descrambled=false;

     //!!specs->img_cnt_udp=0;

     //!!specs->imm_num_pixels=0;

     specs->processed_by_rank=0;

}




//construct num_items imgeQueueItems and put into queue Q.
void imageQueueItem::fillQueue(QQueue<imageQueueItem*> *Q, int num_items,quint64 size_img_shorts)
{
    int k;
    imageQueueItem *item;

    for (k=0; k<num_items;k++)
    {
        item=new imageQueueItem(0,size_img_shorts);
        Q->enqueue(item);
    }
}

// dequeue all items from Q, and destruct them. Leave Q alone, it must be destructed later.
void imageQueueItem::emptyQueue(QQueue<imageQueueItem*> *Q)
{
    imageQueueItem *item;

    while(!Q->isEmpty())
    {
        item=Q->dequeue();
        delete(item);
    }
}


//////
//////
///////
///////
///////
///
///
///
///


imageQueue::imageQueue() :
    QQueue<imageQueueItem*>(),
    lock()
{

}


void imageQueue::enqueue(imageQueueItem* item)
{
    lock.lock();
    QQueue<imageQueueItem*>::enqueue(item);

    lock.unlock();
}

imageQueueItem* imageQueue::dequeue(void)
{
    imageQueueItem *item;
    lock.lock();

     item = QQueue<imageQueueItem*>::dequeue();
    lock.unlock();
    return(item);

}


bool imageQueue::dequeueIfOk(imageQueueItem **item)
{
    bool ans=false;
    *item=0;

    lock.lock();

    if (!isEmpty())
    {
        *item=QQueue<imageQueueItem*>::dequeue();
        ans=true;
    }


    lock.unlock();
    return(ans);
}

//construct num_items imgeQueueItems and put into queue Q.
void imageQueue::fillQueue(int num_items, quint64 size_img_shorts)
{
    int k;
    imageQueueItem *item;
    lock.lock();

    for (k=0; k<num_items;k++)
    {
        item=new imageQueueItem(0,size_img_shorts);
        QQueue<imageQueueItem*>::enqueue(item);
    }
    num_fill_items=num_items;

    lock.unlock();
}

// dequeue all items from Q, and destruct them. Leave Q alone, it must be destructed later.
int imageQueue::emptyQueue(void)
{
    imageQueueItem *item;
    int numdeleted=0;

    lock.lock();

    while(!isEmpty())
    {
        item=QQueue<imageQueueItem*>::dequeue();
        delete(item);
        numdeleted++;
    }
    lock.unlock();
    return(numdeleted);
}
int imageQueue::count()
{
    return(QQueue<imageQueueItem*>::count());
}

bool imageQueue::isEmpty()
{
    return(QQueue<imageQueueItem*>::isEmpty());
}
