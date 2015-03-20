
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "xpcsgui.h"
#include "ui_xpcsgui.h"
#include <qmath.h>
#include "stdio.h"
#include "stdlib.h"

xpcsGui::xpcsGui(
        imageQueue &data,
        imageQueue &free,
        QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::xpcsGui),
    current_gui_state(),
    free_queue(free),
    data_queue(data),
    colorTable(),
    dispTimer(),
    last_image_message(),
    dispTextTimer()
{
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
    this->connect(&dispTextTimer,SIGNAL(timeout()),SLOT(updateText()),Qt::QueuedConnection);
    dispTextTimer.start(disp_time_ms);


}

xpcsGui::~xpcsGui()
{
    delete ui;
    delete[] draw_buff;
    delete dispImage;
}



void xpcsGui::drawImage(imageQueueItem *item)
{
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
            if (imaddr>item->specs->img_len_shorts)
                imaddr=item->specs->img_len_shorts-1;

            ival = item->img_data[imaddr];
            cval=sqrt_table[ival];
            //!!cval = ival%256;

            if (draddr>=maxdraddr)
                draddr=maxdraddr-1;

            draw_buff[draddr]=cval;
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
void xpcsGui::clearImage()
{
    for (int i=0;i<draw_img_xsize*draw_img_ysize;i++)
        draw_buff[i]=100;

    ui->label_dispImage->setPixmap(QPixmap::fromImage(*dispImage));
}

void xpcsGui::newImage(mpiSignalMessage mes)
{

    imageQueueItem *item;

    last_image_message=mes.message;

    bool stat = data_queue.dequeueIfOk(&item);
    if (stat)
    {

        if (dispTimer.elapsed()>disp_time_ms)
        {

            drawImage(item);

            updateText();

            dispTimer.restart();
        }

        free_queue.enqueue(item);
    }

}



void xpcsGui::updateText()
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




            ui->progressBar_numImages->setMinimum(0);
            ui->progressBar_numImages->setMaximum(current_gui_state.message.num_images);
            ui->progressBar_numImages->setValue(last_image_message.img_calc_counter);


            ui->label_dispTextLeft->setText(left);

            ui->label_inQueueLen_RBV->setText(QString().setNum(last_image_message.gui.input_queue_len_RBV));
            ui->label_outQueueLen_RBV->setText(QString().setNum(last_image_message.gui.output_queue_len_RBV));


            current_gui_state.message.input_queue_len_RBV=last_image_message.gui.input_queue_len_RBV;
            current_gui_state.message.output_queue_len_RBV=last_image_message.gui.output_queue_len_RBV;

}

void xpcsGui::on_radioButton_testimages_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_testimgs;

}

void xpcsGui::on_radioButton_linuxpipe_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_pipe;
}

void xpcsGui::on_lineEdit_pipename_editingFinished()
{

}

void xpcsGui::on_lineEdit_pipename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.inpipename,arg1.toStdString().c_str());
}

void xpcsGui::on_checkBox_pipeconnect_clicked(bool checked)
{
    //if (checked)
    //    current_gui_state.message.command=current_gui_state.message.conn_input;
   // else
    //    current_gui_state.message.command=current_gui_state.message.disconn_input;


    //!! emit signal w. current_gui_state.message
  // current_gui_state.message.command=current_gui_state.message.nop;
  // emit guiState(current_gui_state);


}

void xpcsGui::on_checkBox_avgdark_clicked(bool checked)
{
    current_gui_state.message.is_acq_dark=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
}

void xpcsGui::on_lineEdit_ntoavg_textChanged(const QString &arg1)
{
    current_gui_state.message.num_dark=arg1.toInt();
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);

}

void xpcsGui::on_checkBox_subdark_clicked(bool checked)
{
    current_gui_state.message.is_sub_dark=checked;
     current_gui_state.message.command=current_gui_state.message.nop;
     emit guiState(current_gui_state);
}

void xpcsGui::on_radioButton_tifffile_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_tiff;
}

void xpcsGui::on_lineEdit_tiffpath_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffpath,arg1.toStdString().c_str());
}

void xpcsGui::on_lineEdit_tiffbasename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffbasename,arg1.toStdString().c_str());

}

void xpcsGui::on_linEdit_tiffnumber_textChanged(const QString &arg1)
{
    current_gui_state.message.tiffnumber=arg1.toInt();
}

void xpcsGui::on_radioButton_linuxOutPipe_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_pipe;
}

void xpcsGui::on_lineEdit_linuxOutpipeName_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.outpipename,arg1.toStdString().c_str());
}

void xpcsGui::on_pushButton_start_clicked()
{
    clearImage();
    current_gui_state.message.command=current_gui_state.message.start_calcs;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void xpcsGui::on_pushButton_stop_clicked()
{
    current_gui_state.message.command=current_gui_state.message.stop_calcs;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void xpcsGui::on_pushButton_clicked()
{

}

void xpcsGui::on_pushButton_10secpause_clicked()
{

}

void xpcsGui::on_checkBox_saveMPIRam_clicked(bool checked)
{
    current_gui_state.message.is_save_mpi_rams=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here.}
    emit guiState(current_gui_state);
}

void xpcsGui::on_checkBox_isPrintTrace_clicked(bool checked)
{
    current_gui_state.message.is_print_trace=checked;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void xpcsGui::on_checkBox_connOutputPipe_clicked(bool checked)
{
    if (checked)
        current_gui_state.message.command=current_gui_state.message.conn_output;
    else
        current_gui_state.message.command=current_gui_state.message.disconn_output;


    //!! emit signal w. current_gui_state.message
   current_gui_state.message.command=current_gui_state.message.nop;
   emit guiState(current_gui_state);
}

void xpcsGui::on_spinBox_imgSizeX_valueChanged(int arg1)
{
    current_gui_state.message.size_x=arg1;
}


void xpcsGui::on_spinBox_imgSizeY_valueChanged(int arg1)
{
    current_gui_state.message.size_y=arg1;
}

void xpcsGui::on_spinBox_numImgs_valueChanged(int arg1)
{
    current_gui_state.message.num_images=arg1;
}

void xpcsGui::on_spinBox_imgPeriodMs_valueChanged(int arg1)
{
    current_gui_state.message.test_img_period=arg1;
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
}

void xpcsGui::on_radioButton_nullOutput_clicked()
{
    current_gui_state.message.output_type=current_gui_state.message.is_inout_null;
}

void xpcsGui::on_checkBox_isLimitNProcs_clicked(bool checked)
{
    current_gui_state.message.is_limit_max_proc=checked;
}


void xpcsGui::on_spinBox_maxNumProcs_valueChanged(int arg1)
{
    current_gui_state.message.max_num_procs=arg1;
}

void xpcsGui::on_pushButton_10secpause_released()
{
    current_gui_state.message.is_block_10s=false;
}

void xpcsGui::on_pushButton_10secpause_pressed()
{

    current_gui_state.message.is_block_10s=true;
    current_gui_state.message.command=current_gui_state.message.nop;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void xpcsGui::on_lineEdit_numStdThresh_textChanged(const QString &arg1)
{
    current_gui_state.message.num_std_thresh=arg1.toDouble();
    current_gui_state.message.command=current_gui_state.message.update_thresh;


    //!!emit signal here
    emit guiState(current_gui_state);
    current_gui_state.message.command=current_gui_state.message.nop;

}

void xpcsGui::on_radioButton_noImm_clicked()
{
    current_gui_state.message.is_raw_imm=false;
    current_gui_state.message.is_comp_imm=false;

}

void xpcsGui::on_radioButton_rawIMM_clicked()
{
    current_gui_state.message.is_raw_imm=true;
    current_gui_state.message.is_comp_imm=false;
}

void xpcsGui::on_radioButton_compIMM_clicked()
{
    current_gui_state.message.is_raw_imm=false;
    current_gui_state.message.is_comp_imm=true;
}

void xpcsGui::on_comboBox_whichViewImg_currentIndexChanged(int index)
{
    current_gui_state.message.which_img_view=index;

     current_gui_state.message.command=current_gui_state.message.nop;//!!emit signal here
    emit guiState(current_gui_state);


}

void xpcsGui::on_spinBox_inQueueLen_valueChanged(int arg1)
{
    current_gui_state.message.input_queue_size_mb=arg1;
}

void xpcsGui::on_pushButton_resetInQueue_clicked()
{
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
         free_queue.fillQueue(qlen,imgsizeshort);

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
        updateText();



}

void xpcsGui::on_spinBox_outQueueLen_valueChanged(int arg1)
{
    current_gui_state.message.output_queue_size_mb=arg1;
}
