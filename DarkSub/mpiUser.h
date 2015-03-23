#ifndef MPIUser_H
#define MPIUser_H

#include "mpiengine.h"
#include "signalmessage.h"

class mpiUser : public mpiEngine
{
public:
    mpiUser();
    ~mpiUser();


    virtual void beforeCalcs(mpiBcastMessage &message);
    virtual void beforeFirstCalc(mpiBcastMessage &message);
    virtual void afterCalcs(mpiBcastMessage &message);









  virtual  int doImgCalcs(void);



    void subDark(int whichimg);
    void calcThresh(void);


    virtual int setupMPI2(int argc, char *argv[]);


    virtual void shutdownMPI2();



     unsigned short *sdark_image;

};

#endif // MPIUser_H
