#ifndef SAVEASTIFF_THREAD_H
#define SAVEASTIFF_THREAD_H

#include <QtCore>
#include <QThread>
#include <QFile>

#include "imageprocessor.h"

class SaveAsTiff_thread : public QObject
{
    Q_OBJECT


public:
    explicit SaveAsTiff_thread();
    ~SaveAsTiff_thread();

private slots:
    void Save_Image_Thread ( char * image_header, quint16 * image, quint64 image_size,  QString FilePath);
};

#endif // SAVEASTIFF_THREAD_H
