
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/


#include <QDir>
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
    guisettings(),
    pwrite(),
    lastfilename("NULL"),
    directory(QString('/'))
{

    pipe = 0;
    is_pipe_open=false;
    capture_counter=0;
    init_frame_number = 0;

    is_capture_started=false;
    file_number=0;

     myfile=0;
     is_file_open=false;

    last_imm_buffer_number=-1;
}


void pipeWriter::gotGuiSetting(guiSignalMessage mes)
{

    // tiffnumber_RBV gets copied w/ bad value...
    guisettings=mes.message;



    //open pipe here if we are going to...

    //restart tiff numbering...
    if (guisettings.is_set_tiffnumber)
    {
        file_number=guisettings.tiffnumber;
        guisettings.tiffnumber_RBV=file_number;
        printf("pipeWriter::gotGuiSettings update filenumber\n");
        fflush(stdout);
    }

    if (guisettings.is_set_outputtype)
    {
        //close whatever output we are using.
        // new output will open on next image.
        if ( is_file_open)
        {
            if (guisettings.is_output_tiff)
                tifWriter.close(myfile);
            else
                fclose(myfile);


            is_file_open = false;
            myfile = 0;
        }
        closePipe();
    }

#if 0
    if (guisettings.start_capture)
     {

        capture_counter=0;
        is_capture_started=true;
    }

    if (guisettings.stop_capture)
     {


        is_capture_started=false;
    }
    #endif




#if 0
    if (guisettings.command==guisettings.start_calcs)
        openPipe();


    if (guisettings.command==guisettings.stop_calcs)
        closePipe();
#endif

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
    //imgdata=img.message;


        guisettings=img.message.gui;

        imageQueueItem *item;
        bool stat = data_queue.dequeueIfOk(&item);
        //write out file if we capture infinite num files, or cap cnt less then limit
        bool pstat = false;

        if (stat)
        {
            if (guisettings.output_type==guisettings.is_output_tiff)
            {




               // if (guisettings.is_bigstreamfile)
               // {
                    if (!is_file_open)
                    {


                        //QString tiffname;
                        //make sire we have path
                         pstat = directory.mkpath(QString(guisettings.tiffpath));


                         if (pstat)
                         {
                            lastfilename = QString(guisettings.tiffpath) + QString("/") + QString(guisettings.tiffbasename);
                            lastfilename = lastfilename + QString("_%1.tif").arg(
                                        QString().setNum(file_number,10),5,QChar('0'));


                            myfile = tifWriter.open_w((char*)lastfilename.toStdString().c_str());
                            tifWriter.setMultiFrames(guisettings.num_file_capture);
                            capture_counter=0;
                            is_file_open=true;
                         }
                    }

                    if (is_file_open)
                    {
                            tifWriter.tifWr(item->img_data,item->specs->size_x,item->specs->size_y);



                        capture_counter++;
                        if (guisettings.is_file_num_inc)
                            file_number++;


                        if (capture_counter>=guisettings.num_file_capture)
                        {
                            tifWriter.close(myfile);
                            is_file_open=false;
                        }

                    }
            }
            else if ( guisettings.output_type==guisettings.is_output_imm)
            {

                //if (guisettings.is_bigstreamfile)
                //{
                if (!is_file_open)
                {
                    //make sire we have path
                     pstat = directory.mkpath(QString(guisettings.tiffpath));

                     if (pstat)
                     {
                        lastfilename = QString(guisettings.tiffpath) + QString("/") + QString(guisettings.tiffbasename);
                        lastfilename = lastfilename + QString("_%1-%2.imm").
                                   arg( QString().setNum(file_number,10),5,QChar('0')).
                                    arg(QString().setNum(file_number+guisettings.num_file_capture-1,10),5,QChar('0'));



                       myfile = fopen(lastfilename.toStdString().c_str(),"w");
                        capture_counter=0;
                        is_file_open=true;
                        init_frame_number = item->specs->inpt_img_cnt;
                     }

                }


                if (is_file_open)
                {
                    immHeader *header = (immHeader*)item->img_data;
                    header->buffer_number= item->specs->inpt_img_cnt - init_frame_number;

                    if (last_imm_buffer_number!=-1)
                    {
                        if (header->buffer_number-last_imm_buffer_number > 1)
                        {
                            printf("pipeiwriter::newImage- possible skipped imm buffer number\n");
                            fflush(stdout);
                        }
                        else if (header->buffer_number == last_imm_buffer_number)
                        {
                            printf("pipeWriter::newImage - possible repeated imm buffer number\n");
                            fflush(stdout);
                        }
                    }

                    last_imm_buffer_number= header->buffer_number;



                    int nwrite = fwrite(item->img_data, 2,item->specs->num_pixels,myfile);

                    capture_counter++;

                    if (guisettings.is_file_num_inc)
                        file_number++;


                    if (capture_counter>=guisettings.num_file_capture)
                    {
                        fclose(myfile);
                        myfile = 0;
                        is_file_open=false;
                    }

                }
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

                //capture_counter++;
            }



            strcpy(guisettings.fullfilename_RBV,lastfilename.toStdString().c_str());
            item->specs->capture_counter=capture_counter;
            free_queue.enqueue(item);

            //imgdata.images_in_fifo_mpi=data_queue.count();
            guisettings.capture_counter=capture_counter;
            guisettings.tiffnumber_RBV=file_number;

            //!!imgdata.gui=guisettings;
            img.message.gui=guisettings;
            //!!img.message=imgdata;

            emit finishedImage(img);

        }


}
