
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpicontrolgui.h"
#include "ui_mpicontrolgui.h"
#include <qmath.h>
#include "stdio.h"
#include "stdlib.h"

mpiControlGui::mpiControlGui(
        imageQueue &data,
        imageQueue &free,
        int text_params_type_,
        QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mpiControlGui),
    current_gui_state(),
    free_queue(free),
    data_queue(data),
    colorTable(),
    dispTimer(),
    last_image_message(),
    imageTimer(),
    myimm(8000000)

{

    is_need_gui_update=false;
    is_need_settings_sendout=false;
    text_params_type=text_params_type_;

    ui->setupUi(this);
    draw_buff=new unsigned char[draw_img_xsize*draw_img_ysize];
    sqrt_table=new unsigned char[draw_sqrt_len];

    dispImage=new QImage(draw_buff,draw_img_xsize,draw_img_ysize,QImage::Format_Indexed8);

    dispImage->setColorTable(colorTable);
    int i;

    for ( i = 0; i < 256; i++)
            colorTable.push_back(QColor(i, i, i).rgb());


    for ( i=0;i<draw_sqrt_len;i++)
    {
        sqrt_table[i]=(unsigned char)(floor(qSqrt((float)i)));
    }

    dispTimer.start();
    //this->connect(&dispTextTimer,SIGNAL(timeout()),SLOT(updateText()),Qt::QueuedConnection);
    //!!dispTextTimer.start();

    imageTimer.start();

}

mpiControlGui::~mpiControlGui()
{
    delete ui;
    delete[] draw_buff;
    delete dispImage;
}



void mpiControlGui::drawImage(imageQueueItem *item)
{

    unsigned short *imdata  = item->img_data;
    unsigned char* cimg=(unsigned char*)(item->img_data);

    immHeader *immh = (immHeader*)cimg;

    if (immh->mode==2 && \
            immh->rows==item->specs->size_y && \
            immh->cols==item->specs->size_x)
    {
        int sx, sy,npix,ticks;

        myimm.IMMtoRaw(
                    (unsigned char*)item->img_data,
                    item->specs->size_x * item->specs->size_y,
                    (unsigned short*)myimm.imm_buffer,
                    &sx,
                    &sy,
                    &npix,
                    &ticks);

        imdata = (unsigned short*)myimm.imm_buffer;
    }


    // Decimate image to fit into draw buffer.
    //sqrt the image to make it 8 bits.
    int maxdim=item->specs->size_x;
    if (item->specs->size_y > maxdim)
        maxdim= item->specs->size_y;

    double dec_factor_d = ((double)maxdim)/((double)draw_img_xsize);

    //decimation factor- how much to shrink image
    int dec_factor=(int)ceil(dec_factor_d);
    if (dec_factor==0) dec_factor=1;

    // draw iamge coords
    int drx,dry;
    // data iamge coords
    int imx,imy;
    int imaddr;


    unsigned int ival;
    unsigned int cval;
    int draddr = 0;
    int maxdraddr = draw_img_xsize*draw_img_ysize;

    unsigned char maxdrval=0;

    for (imy=0; imy<item->specs->size_y; imy+=dec_factor)
        for (imx=0; imx<item->specs->size_x;imx+=dec_factor)
        {

            draddr = (imy/dec_factor)*draw_img_xsize + (imx/dec_factor);

            int imaddr= imy*item->specs->size_x + imx;
            if (imaddr>item->specs->mem_len_shorts)
                imaddr=item->specs->mem_len_shorts-1;

            ival = imdata[imaddr];
            cval=sqrt_table[ival];
            //!!cval = ival%256;

            if (draddr>=maxdraddr)
                draddr=maxdraddr-1;

            float cval2 = current_gui_state.message.contrast * (float)cval + current_gui_state.message.brightness;
            draw_buff[draddr]=(unsigned int)cval2;
            if (cval>maxdrval)
                maxdrval=cval;




        }

    if (maxdrval==0)
            maxdrval=1;

    unsigned char mfact = 256/maxdrval;

    //!!for (draddr=0;draddr<maxdraddr;draddr++)
     //!!   draw_buff[draddr]=draw_buff[draddr]*mfact;




      ui->label_dispImage->setPixmap(QPixmap::fromImage(*dispImage));
}
// grey the disp image to clear it.
void mpiControlGui::clearImage()
{
    for (int i=0;i<draw_img_xsize*draw_img_ysize;i++)
        draw_buff[i]=100;

    ui->label_dispImage->setPixmap(QPixmap::fromImage(*dispImage));
}

void mpiControlGui::newImage(mpiSignalMessage mes)
{

    imageQueueItem *item;

    //flag we need new update of gui


    set_need_gui_update();
    set_need_settings_send();
    last_image_message=mes.message;


    image_elapsed = imageTimer.elapsed();

    imageTimer.restart();

    bool stat = data_queue.dequeueIfOk(&item);
    if (stat)
    {

        if (dispTimer.elapsed()>current_gui_state.message.display_time_ms)
        {

            drawImage(item);


            dispTimer.restart();
        }


        is_need_send_mess_disp_txt = true;

        memcpy( (char*)(&last_img_specs),(char*)(item->specs),sizeof(imageSpecs));

        /*if (dispTextTimer.elapsed()>current_gui_state.message.update_text_time_ms)
        {
           updateText(item);

            sendMessages();


            dispTextTimer.restart();
        }*/



        free_queue.enqueue(item);
    }

}


void mpiControlGui::set_need_settings_send()
{
    is_need_settings_sendout=true;
}

void mpiControlGui::sendMessages()
{

    if (is_need_settings_sendout==false)
        return;


    is_need_settings_sendout=false;

   // emit(sendCommand(
   //    QString("IOC setParameter NIMAGES_COUNTER 1 int %1\n").
   //             arg(last_image_message.image_counter_rank0)
   //             ));

    if (text_params_type==0)//fccd params
    {

    emit(sendCommand(
       QString("IOC setParameter fccd_frame_cnt 1 int %1\n").
                arg(last_image_message.frame_number_mpi)
                ));


    emit(sendCommand(
       QString("IOC setParameter fccd_img_in_fifo 1 int %1\n").
                arg(last_image_message.imgs_in_infifo + last_image_message.images_in_fifo_mpi)
                ));


    emit(sendCommand(
       QString("IOC setParameter NUM_CAPTURED 1 int %1\n").
                arg(last_image_message.gui.capture_counter)
                ));



    emit(sendCommand(
       QString("IOC setParameter FILE_NUMBER 1 int %1\n").
                arg(last_image_message.gui.tiffnumber_RBV)
                ));

    emit(sendCommand(
       QString("IOC setParameter CAPTURE 1 int %1\n").
                arg((int)(last_image_message.gui.is_capture_RBV))
                ));

    emit(sendCommand(
       QString("IOC setParameter fccd_inpt_img_cnt 1 int %1\n").
                arg((int)(last_image_message.imgspecs.inpt_img_cnt))
                ));



    emit(sendCommand(
       QString("IOC setParameter fccd_acq_pedistals 1 int %1\n").
                arg(last_image_message.gui.is_acq_dark_RBV)
                ));


    emit(sendCommand(
       QString("IOC setParameter fccd_cpu_time 1 int %1\n").
                arg(image_elapsed)
                ));
    }
    else if (text_params_type==1)//lamda AD pipewrite plugin
    {
        emit(sendCommand(
           QString("IOC setParameter PW_XSize 1 int %1\n").
                    arg(last_image_message.gui.size_x)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_YSize 1 int %1\n").
                    arg(last_image_message.gui.size_y)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_DataSourceType 1 int %1\n").
                    arg(last_image_message.gui.input_type)
                    ));




        emit(sendCommand(
           QString("IOC setParameter PW_NumTestImages 1 int %1\n").
                    arg(last_image_message.gui.num_images)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_TestImagePeriod 1 int %1\n").
                    arg(last_image_message.gui.test_img_period)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_InputInfImages 1 int %1\n").
                    arg(last_image_message.gui.is_infinite_num_images)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_InputQueueSize 1 int %1\n").
                    arg(last_image_message.gui.input_queue_size_mb)
                    ));



        emit(sendCommand(
           QString("IOC setParameter PW_OutputQueueSize 1 int %1\n").
                    arg(last_image_message.gui.output_queue_size_mb)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_InputDataSource 1 QString %1\n").
                    arg(last_image_message.gui.inpipename)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_OutputFileType 1 int %1\n").
                    arg(last_image_message.gui.output_type)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_OutputFilePath 1 QString %1\n").
//                    arg(last_image_message.gui.tiffpath)
                    arg(current_gui_state.message.tiffpath)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_OutputFileName 1 QString %1\n").
//                    arg(last_image_message.gui.tiffbasename)
                    arg(current_gui_state.message.tiffbasename)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_OutputFullFileName 1 QString %1\n").
                    arg(last_image_message.gui.fullfilename_RBV)
//                    arg(current_gui_state.message.fullfilename_RBV)
                    ));



        emit(sendCommand(
           QString("IOC setParameter PW_OutputNCapture 1 int %1\n").
                    arg(last_image_message.gui.num_file_capture)
                    ));



        emit(sendCommand(
           QString("IOC setParameter PW_OutputNCaptured 1 int %1\n").
                    arg(last_image_message.gui.capture_counter)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_OutputIncFileNum 1 int %1\n").
                    arg(last_image_message.gui.is_file_num_inc)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_OutputQueueNumImgs 1 int %1\n").
                    arg(last_image_message.gui.output_queue_len_RBV)
                    ));


        emit(sendCommand(
           QString("IOC setParameter PW_InputQueueNumImgs 1 int %1\n").
                    arg(last_image_message.gui.input_queue_len_RBV)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_RunState 1 int %1\n").
                    arg(current_gui_state.message.is_calcs_started)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_OutputNextNumberRBV 1 int %1\n").
                    arg(last_image_message.gui.tiffnumber_RBV)
//                    arg(current_gui_state.message.tiffnumber_RBV)
                    ));

        emit(sendCommand(
           QString("IOC setParameter PW_CalcImageNumPix 1 int %1\n").
                    arg(last_img_specs.imm_num_pixels)
                    ));



    }
    else
    {
        printf("ERROR - mpicontrolgui::sendMessages Bad text param type, check args on startup\n");
        fflush(stdout);
    }

}




void mpiControlGui::updateText(void)
{



            QString left;
            left.append("In Img Cnt: ");
            left.append(QString().setNum(last_image_message.imgspecs.inpt_img_cnt));
            left.append("\n");


            left.append("MPI Calcs: ");
            left.append(QString().setNum(last_image_message.img_calc_counter));
            left.append("\n");


            left.append("OutFifoLen: ");
            left.append(QString().setNum(last_image_message.images_in_fifo_mpi));
            left.append("\n");


            left.append("InFifoLen: ");
            left.append(QString().setNum(last_image_message.imgs_in_infifo));
            left.append("\n");





            left.append("IsRaw: ");
            if (last_img_specs.is_raw)
                    left.append(QString("raw"));
             else if (last_img_specs.is_compressed)
                    left.append(QString("IMMComp"));
             else if (last_img_specs.is_raw_imm)
                     left.append(QString("IMMRaw"));

            left.append("\n");


            if (last_img_specs.is_compressed)
            {
                left.append("IMMNumPix: ");
                left.append(QString().setNum(last_img_specs.imm_num_pixels));
                left.append("\n");

            }


            ui->progressBar_numImages->setMinimum(0);
            ui->progressBar_numImages->setMaximum(current_gui_state.message.num_images);
            ui->progressBar_numImages->setValue(last_image_message.img_calc_counter);


            ui->label_dispTextLeft->setText(left);

            ui->label_inQueueLen_RBV->setText(QString().setNum(last_image_message.gui.input_queue_len_RBV));
            ui->label_outQueueLen_RBV->setText(QString().setNum(last_image_message.gui.output_queue_len_RBV));

            ui->label_NumCaptured_RBV->setText(QString().setNum(last_image_message.gui.capture_counter));

            ui->label_fileNumberRBV->setText(QString().setNum(last_image_message.gui.tiffnumber_RBV));

            ui->label_fullFileName_RBV->setText(last_image_message.gui.fullfilename_RBV);

            current_gui_state.message.input_queue_len_RBV=last_image_message.gui.input_queue_len_RBV;
            current_gui_state.message.output_queue_len_RBV=last_image_message.gui.output_queue_len_RBV;

            current_gui_state.message.is_capture_RBV=last_image_message.gui.is_capture_RBV;

}

void mpiControlGui::on_radioButton_testimages_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_testimgs;

}

void mpiControlGui::on_radioButton_linuxpipe_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_pipe;
     set_need_settings_send();

}

void mpiControlGui::on_lineEdit_pipename_editingFinished()
{

}

void mpiControlGui::on_lineEdit_pipename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.inpipename,arg1.toStdString().c_str());
     set_need_settings_send();
}

void mpiControlGui::on_checkBox_pipeconnect_clicked(bool checked)
{
    //if (checked)
    //    current_gui_state.message.command=current_gui_state.message.conn_input;
   // else
    //    current_gui_state.message.command=current_gui_state.message.disconn_input;


    //!! emit signal w. current_gui_state.message
  // current_gui_state.message.command=current_gui_state.message.nop;
  // emit guiState(current_gui_state);


}

void mpiControlGui::on_checkBox_avgdark_clicked(bool checked)
{
    current_gui_state.message.is_acq_dark=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
     set_need_settings_send();
}

void mpiControlGui::on_lineEdit_ntoavg_textChanged(const QString &arg1)
{
    current_gui_state.message.num_dark=arg1.toInt();
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
     set_need_settings_send();

}

void mpiControlGui::on_checkBox_subdark_clicked(bool checked)
{
    current_gui_state.message.is_sub_dark=checked;
     current_gui_state.message.command=current_gui_state.message.nop;
     emit guiState(current_gui_state);
      set_need_settings_send();
}

void mpiControlGui::on_radioButton_tifffile_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_tiff;

    current_gui_state.message.is_set_outputtype=true;
    emit guiState(current_gui_state);
    current_gui_state.message.is_set_outputtype=false;
     set_need_settings_send();
}

void mpiControlGui::on_lineEdit_tiffpath_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffpath,arg1.toStdString().c_str());

    strcpy(last_image_message.gui.tiffpath,arg1.toStdString().c_str());
    set_need_settings_send();
}

void mpiControlGui::on_lineEdit_tiffbasename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffbasename,arg1.toStdString().c_str());
    strcpy(last_image_message.gui.tiffbasename,arg1.toStdString().c_str());
     set_need_settings_send();

}

void mpiControlGui::on_linEdit_tiffnumber_textChanged(const QString &arg1)
{
    current_gui_state.message.tiffnumber=arg1.toInt();
    current_gui_state.message.tiffnumber_RBV=current_gui_state.message.tiffnumber;
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    current_gui_state.message.is_set_tiffnumber=true;
    emit guiState(current_gui_state);

    current_gui_state.message.is_set_tiffnumber=false;
    //!! what the baby...
    last_image_message.gui.tiffnumber_RBV=current_gui_state.message.tiffnumber;
     set_need_settings_send();

}

void mpiControlGui::on_radioButton_linuxOutPipe_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_pipe;
    current_gui_state.message.is_set_outputtype=true;
    emit guiState(current_gui_state);
    current_gui_state.message.is_set_outputtype=false;
     set_need_settings_send();
}

void mpiControlGui::on_lineEdit_linuxOutpipeName_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.outpipename,arg1.toStdString().c_str());
     set_need_settings_send();
}

void mpiControlGui::on_pushButton_start_clicked()
{
    clearImage();
    current_gui_state.message.command=current_gui_state.message.start_calcs;

    current_gui_state.message.is_calcs_started=true;


    //!!emit signal here
    emit guiState(current_gui_state);
     set_need_settings_send();
}

void mpiControlGui::on_pushButton_stop_clicked()
{

    current_gui_state.message.command=current_gui_state.message.stop_calcs;
    //!!emit signal here

current_gui_state.message.is_calcs_started=false;

    emit guiState(current_gui_state);
 set_need_settings_send();
}

void mpiControlGui::on_pushButton_clicked()
{

}

void mpiControlGui::on_pushButton_10secpause_clicked()
{

}

void mpiControlGui::on_checkBox_saveMPIRam_clicked(bool checked)
{
    current_gui_state.message.is_save_mpi_rams=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here.}
    emit guiState(current_gui_state);
}

void mpiControlGui::on_checkBox_isPrintTrace_clicked(bool checked)
{
    current_gui_state.message.is_print_trace=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void mpiControlGui::on_checkBox_connOutputPipe_clicked(bool checked)
{
    if (checked)
        current_gui_state.message.command=current_gui_state.message.conn_output;
    else
        current_gui_state.message.command=current_gui_state.message.disconn_output;


    //!! emit signal w. current_gui_state.message
   current_gui_state.message.command=current_gui_state.message.nop;
   emit guiState(current_gui_state);
    set_need_settings_send();
}

void mpiControlGui::on_spinBox_imgSizeX_valueChanged(int arg1)
{
    if (arg1<32)
        arg1=32;

    current_gui_state.message.size_x=arg1;

    on_pushButton_resetInQueue_clicked();
     set_need_settings_send();
}


void mpiControlGui::on_spinBox_imgSizeY_valueChanged(int arg1)
{
    if (arg1<32)
        arg1=32;

    current_gui_state.message.size_y=arg1;

    on_pushButton_resetInQueue_clicked();
     set_need_settings_send();
}

void mpiControlGui::on_spinBox_numImgs_valueChanged(int arg1)
{
    current_gui_state.message.num_images=arg1;
}

void mpiControlGui::on_spinBox_imgPeriodMs_valueChanged(int arg1)
{
    current_gui_state.message.test_img_period=arg1;
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
}

void mpiControlGui::on_radioButton_nullOutput_clicked()
{
    current_gui_state.message.output_type=current_gui_state.message.is_inout_null;
    current_gui_state.message.is_set_outputtype=true;
    emit guiState(current_gui_state);
    current_gui_state.message.is_set_outputtype=false;
     set_need_settings_send();
}

void mpiControlGui::on_checkBox_isLimitNProcs_clicked(bool checked)
{
    current_gui_state.message.is_limit_max_proc=checked;
}


void mpiControlGui::on_spinBox_maxNumProcs_valueChanged(int arg1)
{
    current_gui_state.message.max_num_procs=arg1;
}

void mpiControlGui::on_pushButton_10secpause_released()
{
    current_gui_state.message.is_block_10s=false;
}

void mpiControlGui::on_pushButton_10secpause_pressed()
{

    current_gui_state.message.is_block_10s=true;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void mpiControlGui::on_lineEdit_numStdThresh_textChanged(const QString &arg1)
{
    current_gui_state.message.num_std_thresh=arg1.toDouble();
    current_gui_state.message.command=current_gui_state.message.update_thresh;


    //!!emit signal here - makes all ranks update threshlds
    emit guiState(current_gui_state);
    // now emit a nop, so ransk to not update threshs on each image...
    current_gui_state.message.command=current_gui_state.message.nop;
     emit guiState(current_gui_state);
     set_need_settings_send();

}

void mpiControlGui::on_radioButton_noImm_clicked()
{
    current_gui_state.message.is_raw_imm=false;
    current_gui_state.message.is_comp_imm=false;
    //send to all ranks...
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
   emit guiState(current_gui_state);
     set_need_settings_send();

}

void mpiControlGui::on_radioButton_rawIMM_clicked()
{
    current_gui_state.message.is_raw_imm=true;
    current_gui_state.message.is_comp_imm=false;
    //send to all ranks...
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
   emit guiState(current_gui_state);
     set_need_settings_send();
}

void mpiControlGui::on_radioButton_compIMM_clicked()
{
    current_gui_state.message.is_raw_imm=false;
    current_gui_state.message.is_comp_imm=true;
    //send to all ranks...
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
   emit guiState(current_gui_state);
     set_need_settings_send();
}

void mpiControlGui::on_comboBox_whichViewImg_currentIndexChanged(int index)
{
    current_gui_state.message.which_img_view=index;

     current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    emit guiState(current_gui_state);


}

void mpiControlGui::on_spinBox_inQueueLen_valueChanged(int arg1)
{
    current_gui_state.message.input_queue_size_mb=arg1;
     set_need_settings_send();
     set_need_gui_update();
}

void mpiControlGui::on_pushButton_resetInQueue_clicked()
{
#if 1
    //!! we disable reset queue for now... it has bugs
    //!! for now if we change x y size, we keep same queue.
    //!! when prog starts, just have max x and y size when starting



    on_pushButton_stop_clicked();

    current_gui_state.message.is_rst_in_queue=true;
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
     current_gui_state.message.is_rst_in_queue=false;



    //
     // reset output queue
     //


         printf("resetting output queues\n");
         fflush(stdout);

         free_queue.emptyQueue();
         int imgsizeshort=current_gui_state.message.size_x * current_gui_state.message.size_y;
         int oneMB=1024*1024;
         int qlen = (current_gui_state.message.output_queue_size_mb*oneMB)/(sizeof(short)*imgsizeshort);
         //!!int qlen = free_queue.num_fill_items;
         free_queue.fillQueue(qlen,imgsizeshort);
        //!!free_queue.resizeQueue(imgsizeshort);
         current_gui_state.message.output_queue_len_RBV=qlen;

         printf("Qlen %d Frames, Qsize %d MB, ImgSize %dMb\n",
                qlen,
                (qlen*imgsizeshort*sizeof(short))/oneMB,
                imgsizeshort*sizeof(short)/oneMB);
         fflush(stdout);


         //
         //done reset out queue
         //

         //
         // set qlen for input queue... for now we calc here. should be returned from mpi...
         //
         qlen = (current_gui_state.message.input_queue_size_mb*oneMB)/(sizeof(short)*imgsizeshort);

        current_gui_state.message.input_queue_len_RBV=qlen;
        last_image_message.gui=current_gui_state.message;
        set_need_gui_update();
        set_need_settings_send();

#endif

}

void mpiControlGui::on_spinBox_outQueueLen_valueChanged(int arg1)
{
    current_gui_state.message.output_queue_size_mb=arg1;
}

void mpiControlGui::on_checkBox_descramble_clicked(bool checked)
{
    current_gui_state.message.is_descramble=checked;

     current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    emit guiState(current_gui_state);
     set_need_settings_send();

}

void mpiControlGui::on_checkBox_flipendian_clicked(bool checked)
{
    current_gui_state.message.is_flipendian=checked;

     current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    emit guiState(current_gui_state);
     set_need_settings_send();

}

void mpiControlGui::blockAllGuiSignals(bool is)
{
    QList<QWidget *> widgetList = this->findChildren<QWidget *>();
    QList<QWidget *>::const_iterator widgetIter	(widgetList.begin());
    QList<QWidget *>::const_iterator lastWidget	(widgetList.end());

    while ( widgetIter !=  lastWidget)
    {
        (*widgetIter)->blockSignals(is);
        ++widgetIter;
    }
}


void mpiControlGui::set_need_gui_update()
{
    is_need_gui_update = true;
}

void mpiControlGui::update_gui_from_settings()
{
    if (is_need_gui_update)
    {


     is_need_gui_update = false;
    blockAllGuiSignals(true);



       updateText();

        //sendMessages();




    ui->checkBox_IncFileNum->setChecked(current_gui_state.message.is_file_num_inc);

    ui->checkBox_avgdark->setChecked(current_gui_state.message.is_acq_dark);

   // ui->checkBox_flipendian->blockSignals(true);
    ui->checkBox_flipendian->setChecked(current_gui_state.message.is_flipendian);
//ui->checkBox_flipendian->blockSignals(false);

   // ui->checkBox_descramble->blockSignals(true);
    ui->checkBox_descramble->setChecked(current_gui_state.message.is_descramble);
//ui->checkBox_descramble->blockSignals(false);

    ui->checkBox_isLimitNProcs->setChecked(current_gui_state.message.is_limit_max_proc);

   // ui->checkBox_isPrintTrace->blockSignals(true);
    ui->checkBox_isPrintTrace->setChecked(current_gui_state.message.is_print_trace);
    //ui->checkBox_isPrintTrace->blockSignals(false);

    ui->checkBox_saveMPIRam->setChecked(current_gui_state.message.is_save_mpi_rams);

   // ui->checkBox_subdark->blockSignals(true);
    ui->checkBox_subdark->setChecked(current_gui_state.message.is_sub_dark);
   // ui->checkBox_subdark->blockSignals(false);


    ui->checkBox_infiniteInImgs->setChecked(current_gui_state.message.is_infinite_num_images);

   // ui->checkBox_captureInfiniteNum->setChecked(current_gui_state.message.is_infinite_capture);

   // ui->checkBox_bigStreamFile->setChecked(current_gui_state.message.is_bigstreamfile);

    ui->linEdit_tiffnumber->setText(QString().number(current_gui_state.message.tiffnumber));
    ui->lineEdit_linuxOutpipeName->setText(QString(current_gui_state.message.outpipename));
    ui->lineEdit_ntoavg->setText(QString().number(current_gui_state.message.num_dark));
    ui->lineEdit_numStdThresh->setText(QString().number(current_gui_state.message.num_std_thresh));
    ui->lineEdit_pipename->setText(QString(current_gui_state.message.inpipename));
    ui->lineEdit_tiffbasename->setText(QString(current_gui_state.message.tiffbasename));
    ui->lineEdit_tiffpath->setText(QString(current_gui_state.message.tiffpath));


    ui->lineEdit_NumFiles2Capture->setText(QString().number(current_gui_state.message.num_file_capture));

    if (current_gui_state.message.input_type==current_gui_state.message.is_input_pipe)
    {
        ui->radioButton_linuxpipe->setChecked(true);
        ui->radioButton_testimages->setChecked(false);
    }

    if (current_gui_state.message.input_type==current_gui_state.message.is_input_testimgs)
    {
        ui->radioButton_linuxpipe->setChecked(false);
        ui->radioButton_testimages->setChecked(true);
    }

    if (current_gui_state.message.is_raw_imm)
    {
        ui->radioButton_compIMM->setChecked(false);
        ui->radioButton_noImm->setChecked(false);
        ui->radioButton_rawIMM->setChecked(true);
    }
    else if (current_gui_state.message.is_comp_imm)
    {
        ui->radioButton_compIMM->setChecked(true);
        ui->radioButton_noImm->setChecked(false);
        ui->radioButton_rawIMM->setChecked(false);
    }
    else
    {
        ui->radioButton_compIMM->setChecked(false);
        ui->radioButton_noImm->setChecked(true);
        ui->radioButton_rawIMM->setChecked(false);
    }


    if (current_gui_state.message.output_type==current_gui_state.message.is_output_pipe)
    {
        ui->radioButton_linuxOutPipe->setChecked(true);
        ui->radioButton_nullOutput->setChecked(false);
        ui->radioButton_tifffile->setChecked(false);
        ui->radioButton_immFileOutput->setChecked(false);
    }


    if (current_gui_state.message.output_type==current_gui_state.message.is_inout_null)
    {
        ui->radioButton_linuxOutPipe->setChecked(false);
        ui->radioButton_nullOutput->setChecked(true);
        ui->radioButton_tifffile->setChecked(false);
        ui->radioButton_immFileOutput->setChecked(false);

    }


    if (current_gui_state.message.output_type==current_gui_state.message.is_output_tiff)
    {
        ui->radioButton_linuxOutPipe->setChecked(false);
        ui->radioButton_nullOutput->setChecked(false);
        ui->radioButton_tifffile->setChecked(true);
        ui->radioButton_immFileOutput->setChecked(false);

    }


    if (current_gui_state.message.output_type==current_gui_state.message.is_output_imm)
    {
        ui->radioButton_linuxOutPipe->setChecked(false);
        ui->radioButton_nullOutput->setChecked(false);
        ui->radioButton_tifffile->setChecked(false);
        ui->radioButton_immFileOutput->setChecked(true);

    }


   /* ui->spinBox_imgPeriodMs->blockSignals(true);
    ui->spinBox_imgSizeX->blockSignals(true);
    ui->spinBox_imgSizeY->blockSignals(true);
    ui->spinBox_inQueueLen->blockSignals(true);
    ui->spinBox_maxNumProcs->blockSignals(true);
    ui->spinBox_numImgs->blockSignals(true);
    ui->spinBox_outQueueLen->blockSignals(true);*/


    ui->spinBox_imgPeriodMs->setValue(current_gui_state.message.test_img_period);
    ui->spinBox_imgSizeX->setValue(current_gui_state.message.size_x);
    ui->spinBox_imgSizeY->setValue(current_gui_state.message.size_y);
    ui->spinBox_inQueueLen->setValue(current_gui_state.message.input_queue_size_mb);
    ui->spinBox_maxNumProcs->setValue(current_gui_state.message.max_num_procs);
    ui->spinBox_numImgs->setValue(current_gui_state.message.num_images);
    ui->spinBox_outQueueLen->setValue(current_gui_state.message.output_queue_size_mb);


    /*ui->spinBox_imgPeriodMs->blockSignals(false);
    ui->spinBox_imgSizeX->blockSignals(false);
    ui->spinBox_imgSizeY->blockSignals(false);
    ui->spinBox_inQueueLen->blockSignals(false);
    ui->spinBox_maxNumProcs->blockSignals(false);
    ui->spinBox_numImgs->blockSignals(false);
    ui->spinBox_outQueueLen->blockSignals(false);*/

  //  if (current_gui_state.message.is_capture_RBV)
   //     ui->label_isCapturing->setText("Capturing");
  //  else
   //      ui->label_isCapturing->setText("No Capture");


blockAllGuiSignals(false);

    }

}



void mpiControlGui::on_pushButton_acqdarks_clicked()
{

    //on_checkBox_avgdark_clicked(true);

    current_gui_state.message.command=current_gui_state.message.ave_darks_now;
    emit guiState(current_gui_state);

    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);

    set_need_settings_send();

}

void mpiControlGui::on_checkBox_infiniteInImgs_clicked(bool checked)
{

    current_gui_state.message.is_infinite_num_images=checked;

    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
   emit guiState(current_gui_state);
    set_need_settings_send();

}

void mpiControlGui::on_radioButton_immFileOutput_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_imm;
    current_gui_state.message.is_set_outputtype=true;
    emit guiState(current_gui_state);
    current_gui_state.message.is_set_outputtype=false;

    set_need_settings_send();

}

void mpiControlGui::on_lineEdit_NumFiles2Capture_textEdited(const QString &arg1)
{
    bool ok;
    current_gui_state.message.num_file_capture=arg1.toInt(&ok,10);
    set_need_settings_send();
}

void mpiControlGui::on_pushButton_Capture_clicked()
{
#if 0
    current_gui_state.message.start_capture=true;
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    current_gui_state.message.is_set_tiffnumber=false;
    emit guiState(current_gui_state);

     current_gui_state.message.start_capture=false;
#endif
}

void mpiControlGui::on_checkBox_captureInfiniteNum_clicked(bool checked)
{
#if 0
    current_gui_state.message.is_infinite_capture=checked;
#endif
}

void mpiControlGui::on_pushButton_StopCapture_clicked()
{
#if 0
    current_gui_state.message.stop_capture=true;
    current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    current_gui_state.message.is_set_tiffnumber=false;
    emit guiState(current_gui_state);
     current_gui_state.message.stop_capture=false;
#endif

}

void mpiControlGui::on_checkBox_IncFileNum_clicked(bool checked)
{
    current_gui_state.message.is_file_num_inc=checked;

    set_need_settings_send();

}

void mpiControlGui::on_checkBox_bigStreamFile_clicked(bool checked)
{
#if 0
    current_gui_state.message.is_bigstreamfile=checked;
#endif
}

void mpiControlGui::on_spinBox_msBlockTime_valueChanged(int arg1)
{
    current_gui_state.message.ms_block_time=arg1;
}

void mpiControlGui::on_spinBox_msDisplayTime_valueChanged(int arg1)
{
    current_gui_state.message.display_time_ms=arg1;
}

void mpiControlGui::on_lineEdit_brightness_returnPressed()
{

}

void mpiControlGui::on_lineEdit_contrast_returnPressed()
{

}

void mpiControlGui::on_lineEdit_brightness_textEdited(const QString &arg1)
{
    current_gui_state.message.brightness=arg1.toFloat();
}

void mpiControlGui::on_lineEdit_contrast_textEdited(const QString &arg1)
{
    current_gui_state.message.contrast=arg1.toFloat();
}

void mpiControlGui::on_checkBox_block_clicked(bool checked)
{
    current_gui_state.message.is_block_10s=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here
    emit guiState(current_gui_state);



}
