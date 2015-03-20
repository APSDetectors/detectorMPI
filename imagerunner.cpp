#include "imagerunner.h"


//!!TJM
extern StreamInPort *stream_inglbl;


imageRunner::imageRunner(
        QObject *parent,
        MainWindow *mw) :
    QObject(parent)
{
    my_mainwindow=mw;
}


//--------This slot is called when a complete image is received and sends a signal when the image is finished processing
void imageRunner::thread_stream_in_emited_dataready( quint16 *data_block,
                                                    quint64 data_block_size_pixels,
                                                  quint16 frame_number,
                                                  quint16 images_in_fifo,
                                                    quint16 image_in_ptr)
{
    //StreamInPort image_pt;

   // printf("maddog %d\n",image_in_ptr);
    // QString text("New Frame: ");
    // text.append(QString("Frame Number = %1, Block Size = %2\n").arg(frame_number).arg(data_block_size_pixels));
    //  ui->textBrowser_console->append(text); // Just for Debug!




    //!!TJM
    //this is for dumping all signals queued uip with we shut camera. we then dont waste time on emptying the signal queue
    //this is so suresh can stop camera, and the images stop.
    //problem is that if they stop camera and WANT the queud images, we are sunk.

    if (my_mainwindow->is_do_img_calcs_mainwindow_Z==false)
    {
        StreamInPort::Inc_Image_Out_Ptr();
        emit signaldataready(
                    data_block,
                    data_block_size_pixels,
                    frame_number,
                    images_in_fifo,
                    image_in_ptr);
            return;
     }

    //my_mainwindow->ui->label_FrameCount->setText(QString::number(frame_number));
    //my_mainwindow->ui->label_ImagesInFIFOCnt->setText(QString::number(images_in_fifo));



    //!!TJM
   // my_mainwindow->ui->label_InptImgCnt->setText(
     //           QString::number(stream_inglbl->inpt_img_cnt));

    //!!TJM
    if (my_mainwindow->image_proc.ioc_api!=NULL)
    {
        my_mainwindow->image_proc.ioc_api->setParameter(
                    "fccd_frame_cnt",
                    (int)frame_number);

        my_mainwindow->image_proc.ioc_api->setParameter(
                    "fccd_img_in_fifo",
                    (int)images_in_fifo);



        my_mainwindow->image_proc.ioc_api->setParameter(
                    "fccd_cpu_time",
                    my_mainwindow->clocktimer.elapsed());
        my_mainwindow->image_proc.ioc_api->setParameter(
                    "fccd_hw_timest",
                    (int)0);

        my_mainwindow->image_proc.ioc_api->setParameter(
                    "fccd_inpt_img_cnt",
                    (int)stream_inglbl->inpt_img_cnt);


    }

    if (images_in_fifo == StreamInPort::number_data_blocks)
    {

        //my_mainwindow->ui->label_LostImages->setVisible(1);
        if (my_mainwindow->image_proc.ioc_api!=NULL)
            my_mainwindow->image_proc.ioc_api->setParameter("fccd_is_lost_images",(int)1);

    }

    if(my_mainwindow->remoteControlClient && my_mainwindow->remoteControlClient->isOpen())
    {
        my_mainwindow->remoteControlClient->write(
                    QString::number(frame_number).toStdString().c_str());
        my_mainwindow->remoteControlClient->write("\n\r");
        my_mainwindow->remoteControlClient->flush();
    }//

    //image is descrambled, bit shifted and updatepedestals is called. bypass this with a btn.
    my_mainwindow->image_proc.new_raw_image(
                data_block,
                data_block_size_pixels,
                my_mainwindow->save_image_as_tiff);


    if (my_mainwindow->capture_image)
    {
        if (my_mainwindow->save_image_as_tiff)
        {
            my_mainwindow->captureTiff(); //Capture the image after it is processed
        }
        else
        {
            my_mainwindow->captureBin();   //or capture unprocessed image
        }


    }//if (capture_image)
    //subs pedistals if turned on.
    //!!TJM
    my_mainwindow->image_proc.subPedestals();
    //sends image to epics if eics is loaded.
    //!!TJM
     my_mainwindow->image_proc.sendEpicsImage(
                 my_mainwindow->save_image_as_tiff);



    //ui->label_ImageDisplay->setPixmap(QPixmap::fromImage(display_image));
    //ui->label_ImageDisplay->update(); // Redraw the display

    // Done consuming image.  Increment image out counter here
    StreamInPort::Inc_Image_Out_Ptr();

    emit signaldataready(
                data_block,
                data_block_size_pixels,
                frame_number,
                images_in_fifo,
                image_in_ptr);

}

