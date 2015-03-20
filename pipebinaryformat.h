
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#ifndef PIPEBINARYFORMAT_H
#define PIPEBINARYFORMAT_H


#include <imagequeueitem.h>
/**
 * @brief The pipeBinaryFormat class
 *
 * This class defines the binary format images come into and go out of the pipes.
 * It is some binary formt. you can make your own class. derived from this one, and
 * pass to pipeReader and pipeWriter.
 *
 */
class pipeBinaryFormat
{
public:
    pipeBinaryFormat();
    

    virtual int readDataBlock(FILE *fp,imageQueueItem *item);
    virtual int writeDataBlock(FILE *fp,imageQueueItem *item);

    static char zzz[];

    virtual int checkErrors(int nread);
    
};

#endif // PIPEBINARYFORMAT_H
