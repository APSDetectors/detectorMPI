#include "mpicontrolgui.h"
#include "ui_mpicontrolgui.h"
#include <qmath.h>


mpiControlGui::mpiControlGui(
        imageQueue &data,
        imageQueue &free,
        QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mpiControlGui),
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

mpiControlGui::~mpiControlGui()
{
    delete ui;
    delete[] draw_buff;
    delete dispImage;
}



void mpiControlGui::drawImage(imageQueueItem *item)
{
    // Decimate image to fit into draw buffer.
    //sqrt the image to make it 8 bits.
    int maxdim=item->specs->size_x;
    if (item->specs->size_y > maxdim)
        maxdim= item->specs->size_y;


    //decimation factor- how much to shrink image
    int dec_factor=maxdim/draw_img_xsize;
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
void mpiControlGui::clearImage()
{
    for (int i=0;i<draw_img_xsize*draw_img_ysize;i++)
        draw_buff[i]=100;

    ui->label_dispImage->setPixmap(QPixmap::fromImage(*dispImage));
}

void mpiControlGui::newImage(mpiSignalMessage mes)
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



void mpiControlGui::updateText()
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



}

void mpiControlGui::on_radioButton_testimages_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_testimgs;

}

void mpiControlGui::on_radioButton_linuxpipe_clicked(bool checked)
{
    current_gui_state.message.input_type=current_gui_state.message.is_input_pipe;
}

void mpiControlGui::on_lineEdit_pipename_editingFinished()
{

}

void mpiControlGui::on_lineEdit_pipename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.inpipename,arg1.toStdString().c_str());
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

void mpiControlGui::on_radioButton_tifffile_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_tiff;
}

void mpiControlGui::on_lineEdit_tiffpath_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffpath,arg1.toStdString().c_str());
}

void mpiControlGui::on_lineEdit_tiffbasename_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.tiffbasename,arg1.toStdString().c_str());

}

void mpiControlGui::on_linEdit_tiffnumber_textChanged(const QString &arg1)
{
    current_gui_state.message.tiffnumber=arg1.toInt();
}

void mpiControlGui::on_radioButton_linuxOutPipe_clicked(bool checked)
{
    current_gui_state.message.output_type=current_gui_state.message.is_output_pipe;
}

void mpiControlGui::on_lineEdit_linuxOutpipeName_textChanged(const QString &arg1)
{
    strcpy(current_gui_state.message.outpipename,arg1.toStdString().c_str());
}

void mpiControlGui::on_pushButton_start_clicked()
{
    clearImage();
    current_gui_state.message.command=current_gui_state.message.start_calcs;
    //!!emit signal here
    emit guiState(current_gui_state);
}

void mpiControlGui::on_pushButton_stop_clicked()
{
    current_gui_state.message.command=current_gui_state.message.stop_calcs;
    //!!emit signal here
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
}

void mpiControlGui::on_spinBox_imgSizeX_valueChanged(int arg1)
{
    current_gui_state.message.size_x=arg1;
}


void mpiControlGui::on_spinBox_imgSizeY_valueChanged(int arg1)
{
    current_gui_state.message.size_y=arg1;
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
}




void mpiControlGui::on_checkBox_negative_clicked(bool checked)
{
    current_gui_state.message.is_negative = checked;
    //the following lines allow the change in calcs during live run
    // emit will send to all ranks. nop means just update some gui settings.
    current_gui_state.message.command=current_gui_state.message.nop;
    emit guiState(current_gui_state);
}
