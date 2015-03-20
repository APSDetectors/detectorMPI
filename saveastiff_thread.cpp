#include "saveastiff_thread.h"

// Consturctor
SaveAsTiff_thread::SaveAsTiff_thread()
{
}

// Destructor
SaveAsTiff_thread::~SaveAsTiff_thread()
{
}

// Slots
void SaveAsTiff_thread::Save_Image_Thread(char *image_header, quint16 *image, quint64 image_size, QString FilePath)
{
    QFile SaveFile(FilePath);

    if (!SaveFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered))
    {
        //return false;
        // emit error
    }

    //SaveFile.write( proc_image_header,256 + proc_image_size_pixels*2);
    SaveFile.write( image_header,256);
    SaveFile.write( (char*)image, image_size);

    SaveFile.close();

    // emit done
    // return true;

}
