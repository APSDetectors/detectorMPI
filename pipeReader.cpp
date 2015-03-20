
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "pipeReader.h"

 void pipeReader::getPipeImages(void)
{
     //!!open pipe here
     pipeBinaryFormat pread;
     imageQueueItem *item;
     imageSignalMessage imgspecs;




   int cnt=0;
   int stat = 0;

   while(!is_got_close_message && is_pipe_open && cnt<guisettings.num_images && (stat==0))
    {
        //get piped images for ever. killing theadd will stop this.

        if (free_queue.dequeueIfOk(&item))
        {

           stat =  pread.readDataBlock(in_pipe,item);

            data_queue.enqueue(item);

            imgspecs.message.size_pixels=item->specs->img_len_shorts;
            imgspecs.message.frame_number= test_frame_number;
            imgspecs.message.is_lost_images =false;
            imgspecs.message.inpt_img_cnt=test_frame_number;
           imgspecs.message.size_x= item->specs->size_x;
            imgspecs.message.size_y  = item->specs->size_y;

            test_frame_number++;
            emit newImageReady(imgspecs);

            cnt++;


        }

    }

    if (is_pipe_open)
    {
        printf("Close inopuit pipt");
        fflush(stdout);
        fclose(in_pipe);
        in_pipe  = 0;
        is_pipe_open==false;
    }
    //!! we must close pipe here

}

void pipeReader::streamTestImages(void)
{
    int cnt=0;

    while(!is_got_close_message && cnt<guisettings.num_images)
    {
        //get piped images for ever. killing theadd will stop this.
        usleep(guisettings.test_img_period*1000);
        makeTestImage();
        cnt++;
    }



}

void pipeReader::makeTestImage(void)
{
    imageQueueItem *item;
    bool stat;
    unsigned short noise;
    imageSignalMessage imgspecs;

    stat=free_queue.dequeueIfOk(&item);

    if (stat)
    {
        item->specs->is_raw=true;
        item->specs->frame_number=test_frame_number;
        item->specs->inpt_img_cnt=test_frame_number;
       //!! item->specs->img_cnt_udp=test_frame_number;
        //!!item->specs->imm_num_pixels=guisettings.size_x*guisettings.size_y;
        item->specs->img_len_shorts=guisettings.size_x*guisettings.size_y;
        item->specs->size_x=guisettings.size_x;
        item->specs->size_y=guisettings.size_y;

        for (int pix=0;pix<item->specs->img_len_shorts;pix++)
        {
            noise = (unsigned short)(qrand()%256);

            item->img_data[pix]=(unsigned short)(pix%16384);
            item->img_data[pix]+=noise;
        }

        data_queue.enqueue(item);
        imgspecs.message.size_pixels=item->specs->img_len_shorts;
        imgspecs.message.frame_number= test_frame_number;
        imgspecs.message.is_lost_images =false;
        imgspecs.message.inpt_img_cnt=test_frame_number;
       imgspecs.message.size_x= item->specs->size_x;
        imgspecs.message.size_y  = item->specs->size_y;

        test_frame_number++;
        emit newImageReady(imgspecs);
    }
}










//====================================================================
//                              Constructors
//====================================================================

pipeReader::pipeReader(
        imageQueue &free,
        imageQueue &data) :
    guisettings(),
    free_queue(free),
    data_queue(data)
{


    is_got_close_message=false;
    is_pipe_open=false;

    in_pipe=0;
   test_frame_number = 0;

   //signal()


}

pipeReader::~pipeReader(){

}


void pipeReader::onSignal(int param)
{

}

//====================================================================
//                              Slots
//====================================================================

//!! should be queud connection and rn on imagepipe thread.
void pipeReader::openPipe(guiSignalMessage data)
{
    is_pipe_open=false;
    guisettings=data.message;

    is_got_close_message =false;

    printf("pipeReader::openPipe()\n");fflush(stdout);


if (guisettings.input_type==guisettings.is_input_testimgs)
{
    //run forever... then we must kill thrae to get out.
    // we run this on qthread, then we conn qthread terminiate ti pipeReader closePort()
    // qthread start is not conn, it just ruyns qt event loop. we conn open pipe to som hieher controller
    streamTestImages();
}


   if (guisettings.input_type==guisettings.is_input_pipe)
    {


       in_pipe = fopen(guisettings.inpipename,"r");
       if (in_pipe!=0)
       {
           is_pipe_open=true;
           printf("pipeReader::openPipe  opened pipe %s\n",
                  guisettings.inpipename);
           fflush(stdout);
       }

        //runs forever until closePIpe on other thread stops it.
       if (is_pipe_open)
            getPipeImages();
       else
       {
           printf("ERROR - pipeReader::openPipe could not open pipe %s",
                  guisettings.inpipename);
           fflush(stdout);
       }

    }
}

//!! this shoudl NON queued and should run on calling thread, not image pipe thread.
void pipeReader::closePipe()
{

    printf("pipeReader::closePipe()\n");fflush(stdout);
    is_got_close_message=true;

}


//
// take images from free quue and send to data quque, emit data ready signal.
// we have to had alrdy collected tons of images. the old iamges will be sitting on the free queue.
// we jsut reusethem this is for debugging for fast mpi
//

void pipeReader::debugStreamImages(int numimages)
{
 imageSignalMessage mysig;
 imageQueueItem *data_block_buffer;

    for (int k=0;k<numimages;k++)
    {
        if (free_queue.dequeueIfOk(&data_block_buffer))
        {

            data_queue.enqueue(data_block_buffer);
            //!!need to set these?
            //!!mysig.message.data_block_size_pixels=0;

            emit newImageReady(mysig);
        }
        else
           break;

    }

}


//!!TJM changed this alot- added fifo_incnt

//!!TJM changed this alot- added fifo_incnt

