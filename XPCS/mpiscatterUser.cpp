
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpiscatterUser.h"

mpiScatterUser::mpiScatterUser(
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


mpiScatterUser::~mpiScatterUser()
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
void mpiScatterUser::onDeFifo(imageQueueItem *item)
{

     mpi_message.imgspecs.inpt_img_cnt=item->specs->inpt_img_cnt;
}

/*************************************************************************
 *
 *
 ************************************************************************/

void mpiScatterUser::beforeDeFifo(void)
{

    //
    //This is for debugging. we can block 10s so detector images can fill queue.
    // Tjhis allows testing the MPI calcs running really fast.
    //

    if (mpi_message.gui.is_block_10s)
    {
        //!!mpi_message.gui.is_block_10s=false;

        printf("To block 10s\n");
        fflush(stdout);
        usleep(mpi_message.gui.ms_block_time *1000);
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

void mpiScatterUser::afterDeFifo(void)
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


void   mpiScatterUser:: gotMPIGuiSettings(guiSignalMessage mes_)
{


    //copy all stuff from mes_into our local copy. calls operator=...
    mpi_message.gui=mes_.message;

    //make sire we are not calcing an image
    mpi_message.num_images_to_calc=0;
    mpi_message.mpi_image=false;

    if (mpi_message.gui.is_rst_in_queue)
    {
        printf("Resetting Input Queue\n");fflush(stdout);

        free_queue.emptyQueue();
        int imgsizeshort=mpi_message.gui.size_x * mpi_message.gui.size_y;
        int oneMB=1024*1024;
        int qlen = (mpi_message.gui.input_queue_size_mb*oneMB)/(sizeof(short)*imgsizeshort);
        //!!int qlen = free_queue.num_fill_items;
        free_queue.fillQueue(qlen,imgsizeshort);
        //!!free_queue.resizeQueue(imgsizeshort);
        mpi_message.gui.input_queue_len_RBV=qlen;

        printf("Qlen %d Frames, Qsize %d MB, ImgSize %dMb",
               qlen,
               (qlen*imgsizeshort*sizeof(short))/oneMB,
               imgsizeshort*sizeof(short)/oneMB);
        fflush(stdout);

    }

    if (mpi_message.gui.command==mpi_message.gui.ave_darks_now)
    {

        mpi_message.mpi_accum_specs=true;
        mpi_message.mpi_accum_darknoise=true;
        mpi_message.num_dark_images_to_accum=mpi_message.gui.num_dark;
        mpi_message.gui.command=mpi_message.gui.nop;
        sendMPISetup();
        mpi_message.mpi_accum_specs=false;
        sendMPISetup();

        //mpi_message.gui.command=mpi_message.gui.nop;


    }

    mes_.message=mpi_message.gui;

    mpiScatter::gotMPIGuiSettings(mes_);


}

