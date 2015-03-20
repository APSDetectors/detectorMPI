#include <QtGui>
#include "streaminportthread.h"

StreamInPortThread::StreamInPortThread(QObject *parent)
    : QThread(parent)
{
    mutex.lock();
    abort = false;
    init_thread = false;
    new_block_ready = false;
    data_block = NULL;

    condition.wakeOne();
    mutex.unlock();
}

StreamInPortThread::~StreamInPortThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();

    mutex.unlock();

    wait();
}
//! [1]

//! [3]
void StreamInPortThread::run()
{
    //-----------------------------------< Thread Variables >---------
    // Note: All other variables are actualy in the Main Window Thread
    //----------------------------------------------------------------
    //-----------< Buffer >
    quint16 data_block_size = 0; // Thread copy of block size
    quint16 buffer_count = 0;
    uchar *data_block_buffer = NULL;

    //-----------
    bool new_block = false;
    quint16 buffer_count_next;
    quint16 overflow_count;
    qint64 pending_size = 0;
    qint64 read_size = 0;
    uchar  local_buffer[1024];

    //-----------< Port >
    bool open = false;
    QUdpSocket *udpSocket = NULL;
    //------------
    QHostAddress from_address; // Where the recieved packet came from
    quint16 from_port; // Where the recieved packet came from


    forever {
        //------------------------------------------------< Sleep Thread When Closed >
        mutex.lock();
        if (!open || !init_thread)
            condition.wait(&mutex);
        mutex.unlock();

        //------------------------------------------------------< Abort Thread >
        if (abort)
            return;

        //-----------------------------------------------------< Initialize Thread >
        if (init_thread){
            mutex.lock();
            init_thread = false;
            open = true;
            //-----------------------< Allocate Buffers >
            data_block_size = block_size;

            if (&data_block_buffer != NULL) free(data_block_buffer);

            data_block_buffer = (uchar*) malloc(data_block_size);

            if (data_block_buffer == NULL) {
                emit signalerror(ERROR_BUFFER_SIZE_NOT_VALID);
                open = false;
                mutex.unlock();
                continue;
            }

            //-----------------------< Bind the socket >
            if (udpSocket != NULL )
            {
                udpSocket->close();
                free(udpSocket);
            }

            udpSocket = new QUdpSocket;
            udpSocket->moveToThread(QThread::currentThread());


            connect (&udpSocket, SIGNAL(readyRead()), this, SIGNAL(Process()));

            if ( !udpSocket->bind(LocalDataIP, LocalStreamInPortNum) )
            {
                emit signalerror(ERROR_COULD_NOT_BIND_PORT);
                open = false;
                mutex.unlock();
                continue;
            }


            if ( !udpSocket->writeDatagram("dummy data",CameraDataIP, CameraStreamInPortNum) )
            {
                emit signalerror(ERROR_COULD_NOT_SEND_PACKET);
                open = false;
                mutex.unlock();
                continue;
            } else {
                emit signalopen();
            }
            mutex.unlock();
        }

        //-----------------------------------------------------< Thread Main Loop >
        if(!open)
            continue;

        //------------------< Sleep Thead Until New Data >
        udpSocket->waitForReadyRead(3000); // Sleep the thread until data is ready

        //------------------< Get New Data >
        new_block = false;
        pending_size = udpSocket->pendingDatagramSize();

        if (pending_size > 0) {
            mutex.lock();
            emit signaldataready(data_block_size); // <------------------------- Just for test
            mutex.unlock();

            read_size = udpSocket->readDatagram(local_buffer, pending_size, &from_address, &from_port);

            buffer_count_next = read_size + buffer_count;

            if (buffer_count_next > data_block_size) {
                overflow_count = buffer_count_next - data_block_size;
                memcpy (data_block_buffer + buffer_count, local_buffer, read_size - overflow_count);
                memcpy (data_block_buffer, local_buffer + read_size - overflow_count, overflow_count);
                buffer_count = overflow_count;
                new_block = true;

            } else if (buffer_count_next = data_block_size ){
                memcpy (data_block_buffer + buffer_count, local_buffer, read_size);
                buffer_count = 0;
                new_block = true;

            } else {
                memcpy (data_block_buffer + buffer_count, local_buffer, read_size);
                buffer_count += read_size;

            }
        }

        //------------------< Announce New Data Block >
        mutex.lock();
        if (new_block && open && data_block_size == block_size) {
            new_block = false;
            new_block_ready = true;
            memcpy (data_block, data_block_buffer, data_block_size);
            emit signaldataready(data_block_size); // to signal trigger external threads
        }
        mutex.unlock();
    }
}

void StreamInPortThread::openport(QHostAddress CameraIP,quint16 CameraPort, QHostAddress LocalIP, quint16 LocalPort,quint16 BlockSize)
{
    QMutexLocker locker(&mutex); // locks mutex untill the function terminates

    CameraDataIP = CameraIP;
    CameraStreamInPortNum = CameraPort;
    LocalDataIP = LocalIP;
    LocalStreamInPortNum = LocalPort;

    block_size = BlockSize;

    //------------< Create Buffer >
    if (&data_block != NULL) free(data_block);

    data_block = (uchar*) malloc(BlockSize);
    init_thread = true;

    //-------------< Start Thread >
    if (!isRunning()) {
        start(LowPriority);
    } else {
        condition.wakeOne();
    }
}

void StreamInPortThread::closeport()
{
    mutex.lock();
    init_thread = false;
    mutex.unlock();
    emit signalclosed(NO_ERROR);
}
quint16 StreamInPortThread::readdata(uchar *buffer)
{
    mutex.lock();
    memcpy(buffer,data_block,block_size);
    new_block_ready = false;
    mutex.unlock();

    return block_size;
}
