#ifndef mpiHello_H
#define mpiHello_H

#include "mpiengine.h"
#include "signalmessage.h"

class mpiHello : public mpiEngine
{
public:
    mpiHello();
    ~mpiHello();


    virtual void beforeCalcs(mpiBcastMessage &message);
    virtual void beforeFirstCalc(mpiBcastMessage &message);









  virtual  int doImgCalcs(void);



};

#endif // mpiHello_H
