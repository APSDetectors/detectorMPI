#include "mainwindow.h"
#include "ui_mainwindow.h"

//!!TJM
#include "imagerunner.h"

//!! TJM
//#include "stopgui.h"

MainWindow *mwindow_glbl;

StreamInPort *stream_inglbl;


extern "C"  //!!TJM end
{



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    StreamInPort stream_in;


    QThread stream_in_thread;

    //!!TJM
    //stopGUI stopwindow(0,&w);
    //QThread stopwindowthread;

    //!! TJM
    QThread imgRunThread;

    //!!TJM
    imageRunner irun(0,&w);

    //!!TJM
    mwindow_glbl=&w;
    stream_inglbl=&stream_in;

    //------------------------------------< Thread >
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<quint16>("quint16");
    qRegisterMetaType<quint64>("quint64");
    qRegisterMetaType<enum PortError>("enum PortError");
    qRegisterMetaType<enum TriggerMode>("enum TriggerMode");

    qRegisterMetaType<char>("char");
    qRegisterMetaType<QString>("QString");

    // jtw test  Connect signals to slots
//    save_image.connect(&w, SIGNAL(signal_save_image(char ,
//                                                    quint16,
//                                                    quint64,
//                                                    QString)),

//                       SLOT (save_image_thread(char ,
//                                               quint16,
//                                               quint64,
//                                               QString)), Qt::QueuedConnection);

    // jtw test Connect output signals -- may want to add error handling here!!

    // jtw move to thread
    ////save_image.moveToThread(&save_image_thread);


    // Main Window: Signal ---> Stream In: Slot
    stream_in.connect(&w, SIGNAL(signal_stream_in_port_open( QHostAddress ,
                                           quint16      ,
                                           QHostAddress ,
                                           quint16      ,
                                           quint64      )),
                            SLOT(OpenPort( QHostAddress ,
                                          quint16      ,
                                          QHostAddress ,
                                          quint16      ,
                                          quint64      )),Qt::QueuedConnection);

    stream_in.connect(&w, SIGNAL(signal_stream_in_port_close()), SLOT(ClosePort()), Qt::QueuedConnection);


    //!!TJM
    w.connect(
              &irun,
              SIGNAL(signaldataready(
                         quint16 *,
                         quint64,
                         quint16,
                         quint16,
                         quint16 )),
              SLOT(thread_stream_in_emited_dataready(
                       quint16 *,
                       quint64,
                       quint16,
                       quint16 ,
                       quint16)),
                Qt::QueuedConnection);

    //!!TJM
    irun.connect(
          &stream_in,
          SIGNAL(signaldataready(
                     quint16 *,
                     quint64,
                     quint16,
                     quint16,
                     quint16 )),
          SLOT(thread_stream_in_emited_dataready(
                   quint16 *,
                   quint64,
                   quint16,
                   quint16 ,
                   quint16)),
          Qt::QueuedConnection);







    w.connect(&stream_in, SIGNAL(signalclosed(enum PortError )),
              SLOT(thread_stream_in_emited_closed(enum PortError )), Qt::QueuedConnection);

    w.connect(&stream_in, SIGNAL(signalopen()),
              SLOT(thread_stream_in_emited_open()), Qt::QueuedConnection);

    w.connect(&stream_in, SIGNAL(signalerror(enum PortError )),
              SLOT(thread_stream_in_emited_error(enum PortError )), Qt::QueuedConnection);

    w.connect(&stream_in, SIGNAL(signalstatus(quint64, bool, quint64, bool)),
              SLOT(thread_stream_in_emited_status(quint64, bool, quint64, bool)), Qt::QueuedConnection);


    stream_in.moveToThread(&stream_in_thread);
    irun.moveToThread(&imgRunThread);

    //stopwindow.moveToThread(&stopwindowthread);
    //stopwindow.show();
    //stopwindowthread.start();
    //stream_in_thread.connect(&w, SIGNAL(sendTime(QString)), SLOT(setText(QString)));

    w.show();
    stream_in_thread.start(QThread::HighestPriority);
    imgRunThread.start();
    a.exec();                       // infinite loop,  everything from here on is driven by events (mouse clicks, signals, etc).
    stream_in_thread.quit();
    stream_in_thread.wait();
    return 0;
}

}
