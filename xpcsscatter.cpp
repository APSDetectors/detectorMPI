
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "xpcsscatter.h"

xpcsScatter::xpcsScatter(
        mpiEngine *mpi,
        imageQueue &free,
        imageQueue &data) :
    mpiScatter(mpi,free,data)
{

}

/*************************************************************************
 *
 *
 ************************************************************************/


xpcsScatter::~xpcsScatter()
{

}
/*************************************************************************
 *
 *
 ************************************************************************/
//
// called by newImage slot, just after aw take new image off data_queue.
// called just before we scatter to mpi ranks
//
void xpcsScatter::onDeFifo(imageQueueItem *item)
{

     mpi_message.imgspecs.inpt_img_cnt=item->specs->inpt_img_cnt;
}

/*************************************************************************
 *
 *
 ************************************************************************/

void xpcsScatter::beforeDeFifo(void)
{

    //
    //This is for debugging. we can block 10s so detector images can fill queue.
    // Tjhis allows testing the MPI calcs running really fast.
    //

    if (mpi_message.gui.is_block_10s)
    {
        mpi_message.gui.is_block_10s=false;

        printf("To block 10s\n");
        fflush(stdout);
        usleep(10000000);
        printf("Done blocking\n");
        fflush(stdout);
    }


    if (!mpi_message.mpi_is_limit_nproc)
        num2dequeue=my_mpi->numprocs*my_mpi->num_imgs_per_rank;
   else
      num2dequeue= mpi_message.mpi_max_num_proc;
}


/*************************************************************************
 *
 *
 ************************************************************************/

void xpcsScatter::afterDeFifo(void)
{

    mpi_message.imgs_in_infifo=data_queue.count();


    mpi_message.num_images_to_calc=img_calc_counter;

    my_mpi->beforeCalcs(mpi_message);
    mpi_message.mpi_image=true;


}


/*************************************************************************
 *
 *
 ************************************************************************/


void   xpcsScatter:: gotMPIGuiSettings(guiSignalMessage mes_)
{
    if (mes_.message.is_rst_in_queue)
    {
        printf("Resetting Input Queue\n");fflush(stdout);

        free_queue.emptyQueue();
        int imgsizeshort=mes_.message.size_x * mes_.message.size_y;
        int oneMB=1024*1024;
        int qlen = (mes_.message.input_queue_size_mb*oneMB)/(sizeof(short)*imgsizeshort);
        free_queue.fillQueue(qlen,imgsizeshort);

        mes_.message.input_queue_len_RBV=qlen;

        printf("Qlen %d Frames, Qsize %d MB, ImgSize %dMb",
               qlen,
               (qlen*imgsizeshort*sizeof(short))/oneMB,
               imgsizeshort*sizeof(short)/oneMB);
        fflush(stdout);

    }

    mpiScatter::gotMPIGuiSettings(mes_);


}

