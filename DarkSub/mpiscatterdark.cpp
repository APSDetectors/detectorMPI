#include "mpiscatterdark.h"

mpiScatterDark::mpiScatterDark(
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


mpiScatterDark::~mpiScatterDark()
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
void mpiScatterDark::onDeFifo(imageQueueItem *item)
{

     mpi_message.imgspecs.inpt_img_cnt=item->specs->inpt_img_cnt;
}

/*************************************************************************
 *
 *
 ************************************************************************/

void mpiScatterDark::beforeDeFifo(void)
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

void mpiScatterDark::afterDeFifo(void)
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


void   mpiScatterDark:: gotMPIGuiSettings(guiSignalMessage mes_)
{

    if (my_mpi->is_print_trace)
    {
        printf("mpiScatterDark::gotMPIGuiSettings- got sig\n");
        fflush(stdout);
    }


    //copy all stuff from mes_into our local copy. calls operator=...
    mpi_message.gui=mes_.message;



    //
    // Propagate the message to all MPI ranks
    //



        if (mpi_message.gui.command==mpi_message.gui.start_calcs)
        {

            my_mpi->beforeFirstCalc(mpi_message);
            //clear dark images if acc dark image is turned on.
            if (mpi_message.gui.is_acq_dark)
            {
                mpi_message.mpi_accum_specs=true;
                sendMPISetup();
                mpi_message.mpi_accum_specs=false;

            }

            emit connectInput(mes_);
        }
        else if(mpi_message.gui.command==mpi_message.gui.stop_calcs)
        {


            emit disconnectInput();
        }
        else
            sendMPISetup();



}

