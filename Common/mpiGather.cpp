
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpiGather.h"

mpiGather::mpiGather(
        mpiEngine *mpi,
        imageQueue &dq,
        imageQueue &fq) :
    my_message(),
    free_queue(fq),
    data_queue(dq)
{
    my_mpi=mpi;

    is_lost_image=false;
}



void mpiGather::getGuiSpecs(guiSignalMessage message)
{
    //parse stuff in gui message.. set stuff up


        readPendingDatagrams();


}





void mpiGather::callMPIStuff(void)
{
    my_mpi->trace_count=0;



    //rank 0 will scatter here, so we must fence to keep in suync
    my_mpi->fenceMPIData2();
    my_mpi->fenceMPIData2();

    // in rank 1...N-1, my_message is created by rank0, and rerutnrd to all other ranks
    int stat= my_mpi->mpiOneLoop(my_message);





    // get specs from the root process streaminport- we are rcv BCAST from rank 0. WE are rank 1
    my_mpi->Bcast(&my_message,sizeof(my_message), 0);
   \

    //did it acc a dark?

    //rank counter is who many images in tandem we calculated. only valid for MPI_WHOLE_IMGS defined

    int num_images_dequed=0;



    // we will gather if any image thru pipe, but we must fence no matter what
   my_mpi->fenceMPIData2();

}



void mpiGather::beforeGather(void)
{

}

void mpiGather::readPendingDatagrams()
{
// wait for images to come from mpi processes, then signal main window.

    bool is_running;
    is_running=true;
int num_images_dequed=0;
    int imgx=0;
    int imgy=0;
    int nimgs=0;
imageQueueItem *item=0;
is_lost_image=false;
    while(is_running)
    {

        callMPIStuff();

        beforeGather();

        //wait ro image to come from mpi code, and do calcs. final image appears in mpi public memory.
        //imgx, y are img size for ank0 but dummy here. img size from  mpi

       // my_mpi->is_print_trace=true;

       my_mpi->resetScatGathCounter();

        //get finished image from mpi pub memory, then copy onto our fifo, then signal new image to mainwindow

        if (my_message.mpi_image)
        {
            num_images_dequed=0;
            for (int imgk=0;imgk<my_message.num_images_to_calc;imgk++)
            {
                if (free_queue.dequeueIfOk(&item))
                {
                    my_message.imgspecs.is_lost_images=false;
                }
                else
                {
                    my_message.imgspecs.is_lost_images=true;
                    is_lost_image=true;
                }

                if (!my_message.imgspecs.is_lost_images)
                {

                    if (is_lost_image)
                        item->specs->error_code|4;

                    // num of pixels in image.
                    //!!item->specs->img_len_shorts=my_message.imgspecs.size_pixels;
                      my_mpi->gatherImage(item->specs->img_len_shorts,item,0);
                    //inc internal counter, used to keep track of which rank and me offset
                    // we get image from.
                    my_mpi->incScatGathCounter();
                    num_images_dequed++;
                    data_queue.enqueue(item);
                }
            }//for (int imgk=0;imgk<img_calc_counter;imgk++)
        }//my_mpi->mpi_image)
        // we will gather if any image thru pipe, but we must fence no matter what
       my_mpi->fenceMPIData2();

       my_message.images_in_fifo_mpi=data_queue.count();

       for (int ddk=0; ddk<num_images_dequed;ddk++)
        {
           mpiSignalMessage sigmess;\

           sigmess.message=my_message;
         emit signaldataready_mpi(sigmess);
       }

        if (my_message.mpi_stop_calc_loop)
        {
            printf("mpiGather::readPendingDatagrams-my_mpi->mpi_b_stop_calc_loop\n ");
            fflush(stdout);
            is_running=false;
            break;
        }

    }//while is running

}
