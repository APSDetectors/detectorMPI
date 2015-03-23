
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "signalmessageUser.h"

//
//This is a message sent as a qt signal. the idea is to collect all of the settings of the gui
//in this struct, then send as a signal argument. it can be used for all the signals in the system
//to keep design simple. This info will be translated into MPI messages sent between processes.
// Put whatever you want as fields.
//

guiMessageFieldsUser::guiMessageFieldsUser()
{


      is_negative=false;
}



guiMessageFieldsUser& guiMessageFieldsUser::operator =(const guiMessageFieldsUser &other)
{
    is_negative=other.is_negative;
    return *this;
}

newImgMessageFieldsUser::newImgMessageFieldsUser()
{
}


mpiBcastMessageUser::mpiBcastMessageUser() {



}



