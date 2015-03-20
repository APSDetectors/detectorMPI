#ifndef STREAMINPORTTHREAD_H
#define STREAMINPORTTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

enum PortError {
    NO_ERROR,
    ERROR_BUFFER_SIZE_NOT_VALID,
    ERROR_COULD_NOT_SEND_PACKET,
    ERROR_COULD_NOT_BIND_PORT
};

//! [0]
class StreamInPortThread : public QThread
{
    Q_OBJECT

public:
   StreamInPortThread(QObject *parent = 0);
    ~StreamInPortThread();

    void openport( QHostAddress CameraIP,
                   quint16      CameraPort,
                   QHostAddress LocalIP,
                   quint16      LocalPort,
                   quint16      BlockSize);
    void closeport();

    quint16 readdata(uchar *buffer);

signals:
    void signaldataready( quint16 data_block_size);
    void signalclosed(enum PortError error_number);
    void signalopen();
    void signalerror(enum PortError error_number);

protected:
    void run(); // The Actual Thread

private:
    //----------- Thread
    QMutex mutex;
    QWaitCondition condition;
    bool abort;
    bool init_thread;
    bool new_block_ready;
    //----------- Buffer
    quint16 block_size;
    char *data_block;
    //----------- Other
    //quint16 max_skip_count;
    //quint16 skip_count;
    //----------- Port
    QHostAddress LocalDataIP;
    quint16 LocalStreamInPortNum;
    QHostAddress CameraDataIP;
    quint16 CameraStreamInPortNum;
    //QUdpSocket *udpSocket;
private slots:

};
//! [0]


#endif // STREAMINPORTTHREAD_H
