
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "pipewriter.h"

pipeWriter::pipeWriter(
        imageQueue &dq,
        imageQueue &fq,
        QObject *parent) :
    data_queue(dq),
    free_queue(fq),
    QObject(parent),
    tifWriter(),
    imgdata(),
    guisettings()
{

    pipe = 0;
    is_pipe_open=false;

}


void pipeWriter::gotGuiSetting(guiSignalMessage mes)
{
    guisettings=mes.message;

    //open pipe here if we are going to...

    //restart tiff numbering...
    guisettings.tiffnumber_RBV=guisettings.tiffnumber;


    if (guisettings.command==guisettings.start_calcs)
        openPipe();


    if (guisettings.command==guisettings.stop_calcs)
        closePipe();


}

void pipeWriter::openPipe(void)
{
    if (guisettings.output_type==guisettings.is_output_pipe && !is_pipe_open)
    {
        pipe = fopen(guisettings.outpipename,"w");

        if (pipe!=0)
        {
               is_pipe_open=true;
               printf("Output pipe opend %s\n",guisettings.outpipename);
        }
        else
        {
            is_pipe_open=false;
            printf("ERROR: Could not open %s\n",guisettings.outpipename);
        }

    }


}

void pipeWriter::closePipe(void)
{
    if (guisettings.output_type==guisettings.is_output_pipe && is_pipe_open)
    {
    fclose(pipe);
    is_pipe_open=false;
    pipe = 0;
    }

}

void pipeWriter::newImage(mpiSignalMessage img)
{
    imgdata=img.message;

    pipeBinaryFormat pwrite;

        imageQueueItem *item;
        bool stat = data_queue.dequeueIfOk(&item);
        if (stat)
        {
            if (guisettings.output_type==guisettings.is_output_tiff)
            {
                QString tiffname;
                tiffname = QString(guisettings.tiffpath) + QString("/") + QString(guisettings.tiffbasename);
                tiffname = tiffname + QString("_%1.tif").arg(
                            QString().setNum(guisettings.tiffnumber_RBV,10),5,QChar('0'));
                guisettings.tiffnumber_RBV++;



                tifWriter.tifWr(
                            (char*)(tiffname.toStdString().c_str()),
                            item->img_data,
                            item->specs->size_x,
                            item->specs->size_y);
            }
            else if (guisettings.output_type==guisettings.is_output_pipe)
            {

                if (!is_pipe_open)
                {
                    openPipe();
                }

                //write to output pipe

                if (is_pipe_open)
                    pwrite.writeDataBlock(pipe,item);
            }




            free_queue.enqueue(item);

            imgdata.images_in_fifo_mpi=data_queue.count();
            img.message=imgdata;

            emit finishedImage(img);

        }


}
