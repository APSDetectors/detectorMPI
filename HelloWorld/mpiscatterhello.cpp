#include "mpiscatterhello.h"

mpiScatterHello::mpiScatterHello(
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


mpiScatterHello::~mpiScatterHello()
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
void mpiScatterHello::onDeFifo(imageQueueItem *item)
{

     mpi_message.imgspecs.inpt_img_cnt=item->specs->inpt_img_cnt;
}

/*************************************************************************
 *
 *
 ************************************************************************/

void mpiScatterHello::beforeDeFifo(void)
{



}


/*************************************************************************
 *
 *
 ************************************************************************/

void mpiScatterHello::afterDeFifo(void)
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


void   mpiScatterHello:: gotMPIGuiSettings(guiSignalMessage mes_)
{

    if (my_mpi->is_print_trace)
    {
        printf("mpiScatterHello::gotMPIGuiSettings- got sig\n");
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


            emit connectInput(mes_);
        }
        else if(mpi_message.gui.command==mpi_message.gui.stop_calcs)
        {


            emit disconnectInput();
        }
        else
            sendMPISetup();



}

