#ifndef IMAGERUNNER_H
#define IMAGERUNNER_H

#include <QObject>


#include "mainwindow.h"

class imageRunner : public QObject
{
    Q_OBJECT
public:
    explicit imageRunner(QObject *parent = 0,MainWindow *mw=0);
    
    MainWindow *my_mainwindow;

signals:
    void signaldataready(
            quint16 *data_block,
            quint64 data_block_size_pixels,
            quint16 frame_number,
            quint16 images_in_fifo,
            quint16 img_in_ptr);

public slots:
 void thread_stream_in_emited_dataready(
         quint16 *data_block,
         quint64 data_block_size_pixels,
         quint16 frame_number,
         quint16 images_in_fifo,
         quint16 image_in_ptr);

};

#endif // IMAGERUNNER_H
