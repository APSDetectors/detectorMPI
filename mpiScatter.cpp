
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpiScatter.h"

#include "mpiengine.h"


//extern mpiEngine *myMPI;

//extern StreamInPort *stream_inglbl;


mpiScatter::mpiScatter(mpiEngine *mpi,imageQueue &free, imageQueue &data ) :
    disp_timer(),
    temp_queue(),
    mpi_message(),
    free_queue(free),
    data_queue(data)
{
    my_mpi=mpi;




    //!! rank_dbg_limit=mpi->numprocs;
     //!!is_rank_dbg_limit=false;

    fifolen=64;

    num_imgs_to_stream=0;

    //num images sto calc on ONE new image call back.ONE mpi calc kickoff
    img_calc_counter=1;

    num2dequeue=1;

    signals_no_frames=0;

     num_queued_img_signals=0;
     num_imgs_de_fifoed=0;
     num_slots_called=0;


     disp_timer.start();

}
/*************************************************************************
 *
 *
 ************************************************************************/


mpiScatter::~mpiScatter()
{

}


/*************************************************************************
 *
 *
 ************************************************************************/



bool mpiScatter::deQueueSignals(void)
{
    //count nmber of times we call this slot.
    num_slots_called++;
    num_queued_img_signals=num_imgs_de_fifoed-num_slots_called;

    // if we have two many signals queued then we just return
    if (num_queued_img_signals>10)
        return(true);

    return(false);
}

/*************************************************************************
 *
 *
 ************************************************************************/



int mpiScatter::deFifoScatterImages(void)
{


    imageQueueItem *item;

    if (my_mpi->is_print_trace)
    {
        printf("mpiScatter::deFifoImages()\n");
        fflush(stdout);
    }



    bool is_got_img=false;
   //count images we dequeue, to calc, on all ranks
    int img_calc_counter=0;


    //get up to numprocs images from the fifo. we will do mpi calcs on up to N images at once. where N is num pf procs.
    // each rank gets one whole image.Images are not split up into chunks but are kept whole, and computed on at once several at a time

    //here we dequeue images from the data_queue, and scatter them to ranks. using mem puts.
   //we must fence and unfence as we scatter to sync processes






    //dequeue same number of images we have ranks, if images are avauilable. if we have 2 ranks, we
     // try to dequeu 2 images and scatter
     num2dequeue=my_mpi->numprocs*my_mpi->num_imgs_per_rank;

        // rank dbg limit forces us to use fewer processes at once.


     beforeDeFifo();

     my_mpi->fenceMPIData2();
    my_mpi->resetScatGathCounter();

    do {


       is_got_img=false;
       if (data_queue.dequeueIfOk(&item))
       {
         is_got_img  =true;

          onDeFifo(item);

         //send image to correct process w/ correct address offset based on number of
         //processes, number of images per rank.
         //my_mpi->scatterImage(data_block_size_pixels,item->img_data);
         my_mpi->scatterImage(item);

        // inc an internal coujnter to keep track of how many iamges we scattered.
          my_mpi->incScatGathCounter();
          //get num images in the queue to measure data log jam



         //store to temp queue until we finish all transactions w/ mpi scatter
         temp_queue.enqueue(item);
         img_calc_counter++;

       }

    } while(is_got_img && img_calc_counter<num2dequeue);

    my_mpi->fenceMPIData2();


    while (!temp_queue.isEmpty())
    {
        item=temp_queue.dequeue();




        free_queue.enqueue(item);
    }



    //count number f images we take from fifo.
    // we need this to keep track of total num images dequeued, comparted to numn of
    // image qt signals we get
    num_imgs_de_fifoed+=img_calc_counter;

    return(img_calc_counter);

}


/*************************************************************************
 *
 *
 ************************************************************************/


 void mpiScatter::onDeFifo(imageQueueItem *item)
 {

 }

 /*************************************************************************
  *
  *
  ************************************************************************/


 void mpiScatter::beforeDeFifo(void)
 {


 }


//--------This slot is called when a complete image is received and sends a signal when the image is finished processing

//we hae an odd situation here. we get a signal for EVERY image. every ONE image signsls this function.
//Because we have to do computes, it make take time for the slot to finish. so the slot
//takes more than ONE image off the fifo, and passes to ranks in MPI. So we get ONE signal,
// and we usually do N imges. So we then accum lots of signals in the QT signal queue.
// so what we do is have a counter onhow many times this slot is callsd, and how many images
// we take from FIFO. The difference is the number of signals waiting in the queue.
// if we run at high rate forever, the signal queue will fill up to infinity.
// to seove this problem we compare the fifo get image counter with the counter of how many times
// we enter this slot. The result is the number of signals waiting in the queue. To empty the queue,
// we just quicky return from thsi slot so we never fill up the signal quque.
void mpiScatter::newImage(imageSignalMessage mes_)
{


    //!!int frame_number;
    //!!int data_block_size_pixels;
    //!!imageQueueItem *item;
    //!!int images_in_fifo=data_queue.count();
    //!!int inpt_img_cnt;



    // put latest image specs into our current saved mpi_message
    mpi_message.imgspecs=mes_.message;

    if (my_mpi->is_print_trace)
    {
        printf("mpiScatter::newImage()\n");
        fflush(stdout);
    }

    // make sure qt signals dont pile up. this happens because
    // we sinal all new images, but dequeue in groups.
    if (deQueueSignals())
        return;




    //my_mpi->is_print_trace=true;
    my_mpi->trace_count=0;



     img_calc_counter=deFifoScatterImages();


    afterDeFifo();

  my_mpi->mpiOneLoop(mpi_message);
    //we are rank 0, so WE BCAST
  my_mpi->Bcast(&mpi_message,sizeof(mpiBcastMessage), 0);

    //rank 1 will gether here, so we must fence to keep in suync
    my_mpi->fenceMPIData2();
    my_mpi->fenceMPIData2();





}


/*************************************************************************
 *
 *
 ************************************************************************/


void mpiScatter::afterDeFifo(void)
{

}

/*************************************************************************
 *
 *
 ************************************************************************/



void   mpiScatter:: gotMPIGuiSettings(guiSignalMessage mes_)
{

    if (my_mpi->is_print_trace)
    {
        printf("xpcsScatter::gotMPIGuiSettings- got sig\n");
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

            // empty input fifo
            imageQueueItem *item;
            while(data_queue.dequeueIfOk(&item))
            {
                free_queue.enqueue(item);
                num_imgs_de_fifoed++;
            }
        }
        else
            sendMPISetup();





}


//
// Propagate the message to all MPI ranks
//
/*************************************************************************
 *
 *
 ************************************************************************/



void   mpiScatter::sendMPISetup(void)
{


       // my_mpi->is_print_trace=true;
        my_mpi->trace_count=0;
            my_mpi->fenceMPIData();

    my_mpi->beforeCalcs(mpi_message);
            my_mpi->mpiOneLoop(mpi_message);

            my_mpi->Bcast(&mpi_message,sizeof(mpi_message), 0);

            my_mpi->fenceMPIData();
                printf("Rank 0 sent gui settings\n");
                fflush(stdout);

}
