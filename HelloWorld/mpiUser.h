#ifndef mpiUser_H
#define mpiUser_H

#include "mpiengine.h"
#include "signalmessage.h"

class mpiUser : public mpiEngine
{
public:
    mpiUser();
    ~mpiUser();


    virtual void beforeCalcs(mpiBcastMessage &message);
    virtual void beforeFirstCalc(mpiBcastMessage &message);









  virtual  int doImgCalcs(void);



};

#endif // mpiUser_H
