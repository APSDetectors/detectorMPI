#ifndef MPICONTROLGUI_H
#define MPICONTROLGUI_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "signalmessage.h"
#include "imagequeueitem.h"

namespace Ui {
class mpiControlGui;
}

class mpiControlGui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit mpiControlGui(
            imageQueue &data,
            imageQueue &free,
            QWidget *parent = 0);
    ~mpiControlGui();
    

    // time delay for disp image draw
     QTime dispTimer;
     //time delay for text update on gui.
     QTimer dispTextTimer;

     // draws latest image on screen
    void drawImage(imageQueueItem *item);
    // clear image on screen
void clearImage();



// size of drawing window on screen shoudl agree w/ mpicontrol.ui
    enum {
        draw_img_xsize=320,
        draw_img_ysize=320,
        draw_sqrt_len=65536,
        disp_time_ms = 500
    };

    // drawing buffer- pixels go here.
    unsigned char *draw_buff;
    // converts puixels to sqrt.
    unsigned char *sqrt_table;

    // display image obj and coloar table
    QImage *dispImage;
    QVector<QRgb> colorTable;


public slots:

    virtual void newImage(mpiSignalMessage mes);
    //update text on the gui
    virtual void updateText();

private slots:
    void on_radioButton_testimages_clicked(bool checked);

    void on_radioButton_linuxpipe_clicked(bool checked);

    void on_lineEdit_pipename_editingFinished();

    void on_lineEdit_pipename_textChanged(const QString &arg1);

    void on_checkBox_pipeconnect_clicked(bool checked);


    void on_radioButton_tifffile_clicked(bool checked);

    void on_lineEdit_tiffpath_textChanged(const QString &arg1);

    void on_lineEdit_tiffbasename_textChanged(const QString &arg1);

    void on_linEdit_tiffnumber_textChanged(const QString &arg1);

    void on_radioButton_linuxOutPipe_clicked(bool checked);

    void on_lineEdit_linuxOutpipeName_textChanged(const QString &arg1);

    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();


    void on_checkBox_isPrintTrace_clicked(bool checked);

    void on_checkBox_connOutputPipe_clicked(bool checked);

    void on_spinBox_imgSizeX_valueChanged(int arg1);

    void on_spinBox_imgSizeY_valueChanged(int arg1);

    void on_spinBox_numImgs_valueChanged(int arg1);

    void on_spinBox_imgPeriodMs_valueChanged(int arg1);

    void on_radioButton_nullOutput_clicked();


    void on_checkBox_negative_clicked(bool checked);

signals:
    void guiState(guiSignalMessage mes);

private:

    guiSignalMessage current_gui_state;

    mpiBcastMessage last_image_message;

    Ui::mpiControlGui *ui;

    imageQueue &free_queue;
    imageQueue &data_queue;


};

#endif // MPICONTROLGUI_H
