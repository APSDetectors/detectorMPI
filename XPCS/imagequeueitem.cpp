
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/




#include <stdio.h>

#include "imagequeueitem.h"

imageQueueItem::imageQueueItem(QObject *parent, quint64 imgsize_shorts):
    QObject(parent)
{
    //add an extra 64k of space to each image- room for imm headers or whatever...


    //img_data = new unsigned short[imgsize_shorts];
    img_data = (unsigned short*)malloc((imgsize_shorts+extra_shorts)*sizeof(short));
    // assume we get x4 compression. if not we lose data.. who cares
    //comp_img_data=new unsigned char[imgsize_shorts/4];
    //!!comp_img_data= (unsigned char*)malloc((imgsize_shorts)*sizeof(short));

    specs=(imageSpecs*)malloc(sizeof(imageSpecs));

    clear();

    specs->extra_space_shorts=extra_shorts;
    specs->img_len_shorts=imgsize_shorts+extra_shorts;
    specs->mem_len_shorts=specs->img_len_shorts;



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

     specs->extra_space_shorts=0;

    //length of image in shorts, this is sizex*sizey if pixel is a short
     specs->img_len_shorts=0;
    //number of images stuck into the img_data, if processing several frames at once.
     specs->numframes=0;
    // true of compressed data present
    specs->is_compressed=false;
    //true of non compressed img present
     specs->is_raw=false;

     specs->is_raw_imm=false;

     specs->frame_number = 0;

     specs->is_descrambled=false;

     specs->img_cnt_udp=0;

     specs->imm_num_pixels=0;

     specs->processed_by_rank=0;

     specs->capture_counter=0;

     specs->num_pixels=0;

     specs->dark_accum_tot=0;
     specs->rank_dark_accum_cnt=0;

     specs->error_code=0;

     specs->system_clock=0.0;
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
    lock()
{
    queue = 0;
    queue = (imageQueueItem**)malloc(max_queue_mem_size * sizeof(imageQueueItem*));
    imglist=0;
    imglist = (imageQueueItem**)malloc(max_queue_mem_size * sizeof(imageQueueItem*));


    max_queue_length=max_queue_mem_size;
    max_queue_length2=max_queue_length*2;

    read_location_extra=0;
    read_location=0;
    write_location=0;
    write_location_extra=0;
    queue_length=0;
    calcEmptyFull();
num_fill_items=0;


}


void imageQueue::enqueue(imageQueueItem* item)
{

    if (!is_full)
    {


        queue[write_location]=item;
        write_location_extra = (write_location_extra+1)% max_queue_length2;
        write_location= (write_location+1)%max_queue_length;

        calcEmptyFull();


    }

}

void imageQueue::calcEmptyFull(void)
{
    queue_length=(write_location-read_location)%max_queue_length;
    //while (queue_length<0)
    //    queue_length+=max_queue_length;


    if (write_location_extra==read_location_extra)
    {
        is_empty=true;
        is_full = false;
    }
    else
    {
        is_empty=false;
        if (write_location==read_location)
            is_full=true;
        else
            is_full = false;
    }

}

imageQueueItem* imageQueue::dequeue(void)
{
    imageQueueItem *item=0;

       if (!is_empty)
       {
           item = queue[read_location];

           read_location_extra = (read_location_extra+1)% max_queue_length2;
           read_location= (read_location+1)%max_queue_length;

           calcEmptyFull();


       }

       return(item);

}


bool imageQueue::dequeueIfOk(imageQueueItem **item)
{
    bool ans = false;
    *item=0;



    if (!isEmpty())
    {
        *item=dequeue();
        ans=true;
    }



    return(ans);
}

//construct num_items imgeQueueItems and put into queue Q.
void imageQueue::fillQueue(int num_items, quint64 size_img_shorts)
{
    num_fill_items = 1;
    while (num_fill_items<num_items)
        num_fill_items = num_fill_items<<1;

    if (num_fill_items > max_queue_mem_size)
        num_fill_items= max_queue_mem_size;



    printf("imageQueue::fillQueue,num_fill_items=%d \n",num_fill_items);




    int k;
    imageQueueItem *item;
    lock.lock();

    for (k=0; k<num_fill_items;k++)
    {
        item=new imageQueueItem(0,size_img_shorts);
        enqueue(item);
        imglist[k] = item;
    }


    lock.unlock();
    printf("imageQueue::fillQueue  Created %d items\n",num_fill_items);

    checkQueue();

}


void imageQueue::checkQueue(void)
{
    // update counters
    calcEmptyFull();

    //print counters
    printf("-------------imageQueue::checkQueue----------------\n");

    printf("%s %d \n","num_fill_items" ,num_fill_items );
    printf("%s %d \n","queue_length" , queue_length);
    printf("%s %d \n","max_queue_length" , max_queue_length);
    printf("%s %d \n","max_queue_length2" ,max_queue_length2 );
    printf("%s %d \n","write_location_extra" ,write_location_extra );
    printf("%s %d \n","read_location_extra" ,read_location_extra );

    printf("%s %d \n","write_location" ,write_location );
    printf("%s %d \n","read_location" ,read_location );
     printf("%s %d \n","is_empty" , is_empty);
      printf("%s %d \n","is_full" , is_full);


      printf("\nRun DEQUEUE/ENQUEUE Test\n");
    // check to see if pointers match the imglist, find any bad images in the queue
      imageQueueItem *item;
      int k,n;
      bool is_found;
      bool is_error = false;
      int ntest;
      //go thru queue 2 times
      for (ntest=0;ntest<2;ntest++)
      {
        for ( k=0; k<num_fill_items;k++)
          {
              item = dequeue();

              is_found=false;
              for (n=0;n<num_fill_items;n++)
              {
                  if (imglist[n]==item)
                  {
                     is_found=true;
                  }
               }
             // printf("k %d item %d is_found %d\n",k,item,is_found);
              if (!is_found)
              {
                  is_error=true;
              }

              enqueue(item);
          }
        }

      //print counters
      printf("-------------imageQueue::checkQueue----------------\n");

      printf("%s %d \n","num_fill_items" ,num_fill_items );
      printf("%s %d \n","queue_length" , queue_length);
      printf("%s %d \n","max_queue_length" , max_queue_length);
      printf("%s %d \n","max_queue_length2" ,max_queue_length2 );
      printf("%s %d \n","write_location_extra" ,write_location_extra );
      printf("%s %d \n","read_location_extra" ,read_location_extra );

      printf("%s %d \n","write_location" ,write_location );
      printf("%s %d \n","read_location" ,read_location );
       printf("%s %d \n","is_empty" , is_empty);
        printf("%s %d \n","is_full" , is_full);

      printf("\nQueueError? %d\n",is_error);
}



void imageQueue::resizeQueue(quint64 size_img_shorts)
{   int k;
    imageQueueItem *item;
    for (k=0;k<num_fill_items;k++)
    {

        item =imglist[k];


        item->specs->img_len_shorts= size_img_shorts + item->specs->extra_space_shorts;

        if (item->specs->img_len_shorts > item->specs->mem_len_shorts)
            item->specs->img_len_shorts = item->specs->mem_len_shorts;

    }
}

// dequeue all items from Q, and destruct them. Leave Q alone, it must be destructed later.
int imageQueue::emptyQueue(void)
{


    imageQueueItem *item;

    int k;


    checkQueue();

    lock.lock();

    printf("qlen %d \n",queue_length );
    for (k=0;k<num_fill_items;k++)
    {

        item =imglist[k];

            delete(item);


    }
    lock.unlock();

    printf("imageQueue::emptyQueue Deleted num_fill_items=%d\n",num_fill_items);

    read_location_extra=0;
    read_location=0;
    write_location=0;
    write_location_extra=0;
    queue_length=0;
    calcEmptyFull();

    return(num_fill_items);
}
int imageQueue::count()
{
    return(queue_length);
}

bool imageQueue::isEmpty()
{
    calcEmptyFull();
    return(is_empty);
}
