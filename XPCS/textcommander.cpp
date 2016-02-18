#include "textcommander.h"
#include <QStringList>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
textCommander::textCommander(
        QString in_pipe_name_,
        QString out_pipe_name_) :
    objmap(),
    in_pipe_name(in_pipe_name_),
    out_pipe_name(out_pipe_name_),
    app_name("app")
{
    in_pipe = 0;
    out_pipe = 0;


    is_sleep_in=false;
    is_sleep_out=false;
     is_in_pipe_open=false;
     is_waiting=true;
     is_out_pipe_open=false;
     is_makefifo=false;

}

void textCommander::addNameObject( QString name,  QObject *obj)
{
    objmap[name] = obj;
}


void textCommander::setAppName(QString name)
{
    app_name=name;
}



bool textCommander::openInputPipe(void)
{
    if ( !is_in_pipe_open)
    {
        if (is_makefifo)
             mkfifo (in_pipe_name.toStdString().c_str(), S_IRUSR| S_IWUSR);

        if (is_sleep_in)
        {

            char cmd[255];
            sprintf(cmd,"sleep 999999 > %s &",in_pipe_name.toStdString().c_str());
            system(cmd);
            strcpy(sleep_procid_in,cmd);
        }

        in_pipe = fopen(in_pipe_name.toStdString().c_str(),"r");

        if (in_pipe!=0)
        {
               is_in_pipe_open=true;
               printf("Input pipe opend %s\n",in_pipe_name.toStdString().c_str());
        }
        else
        {
            is_in_pipe_open=false;
            printf("ERROR: Could not open %s\n",in_pipe_name.toStdString().c_str());
        }

    }


}
bool textCommander::isOpenSleeps(bool is_in, bool is_out)
{
    is_sleep_in=is_in;
    is_sleep_out=is_out;
}


bool textCommander::openOutputPipe(void)
{
    if ( !is_out_pipe_open)
    {

        if (is_makefifo)
            mkfifo (out_pipe_name.toStdString().c_str(), S_IRUSR| S_IWUSR);

        if (is_sleep_out)
        {
            char cmd[255];
            sprintf(cmd,"sleep 999999 < %s &",out_pipe_name.toStdString().c_str());
            system(cmd);
            strcpy(sleep_procid_out,cmd);
        }
        out_pipe = fopen(out_pipe_name.toStdString().c_str(),"w");


        if (out_pipe!=0)
        {
               is_out_pipe_open=true;
               printf("Output pipe opend %s\n",out_pipe_name.toStdString().c_str());
        }
        else
        {
            is_out_pipe_open=false;
            printf("ERROR: Could not open %s\n",out_pipe_name.toStdString().c_str());
        }

    }

}

bool textCommander::closeInputPipe(void)
{
    if (is_in_pipe_open)
    {
    fclose(in_pipe);
    is_in_pipe_open=false;
    in_pipe = 0;
    }
}

bool textCommander::closeOutputPipe(void)
{
    if (is_out_pipe_open)
    {
    fclose(out_pipe);
    is_out_pipe_open=false;
    out_pipe = 0;
    }
}

bool textCommander::waitNextCommand(void)
{
    char line[256];
    is_waiting=true;
    printf("Waiting for Command\n");
    while(is_waiting && is_in_pipe_open)
    {

        int len;
        fgets(line,255,in_pipe);

        QString qline(line);
        QStringList args = qline.split(" ");

        bool ok;

        if (args.length()>=4)
        {

            QString app=args.at(0);
            QString obj_name = args.at(1);
            QString slot_name = args.at(2);
            int argc = args.at(3).toInt(&ok,10);

            if (app==app_name)
            {

                printf("Got Cmd: %s\n",line);

                if (argc==0)
            {
                QMetaObject::invokeMethod(
                            objmap[obj_name],
                    slot_name.toStdString().c_str(),
                    Qt::AutoConnection);

            }

                else if (argc==1)
            {
                QString argtype = args.at(4);
                QString argval = args.at(5).simplified();



                if (argtype == "int")
                {
                    bool ok;
                    int ival =argval.toInt(&ok,10);

                    QMetaObject::invokeMethod(
                                objmap[obj_name],
                        slot_name.toStdString().c_str(),
                        Qt::AutoConnection,
                                Q_ARG(int,ival) );
                }


                if (argtype == "double")
                {

                    bool ok;
                    double dval =argval.toDouble(&ok);

                    QMetaObject::invokeMethod(
                                objmap[obj_name],
                        slot_name.toStdString().c_str(),
                        Qt::AutoConnection,
                                Q_ARG(double,dval) );
                }

                if (argtype == "QString")
                {



                    QMetaObject::invokeMethod(
                                objmap[obj_name],
                        slot_name.toStdString().c_str(),
                        Qt::AutoConnection,
                                Q_ARG(QString,argval) );
                }

                if (argtype == "bool")
                {
                    bool ok;
                    int ival =argval.toInt(&ok,10);
                            bool bval = (bool)ival;
                    QMetaObject::invokeMethod(
                                objmap[obj_name],
                        slot_name.toStdString().c_str(),
                        Qt::AutoConnection,
                                Q_ARG(bool,bval) );
                }


            }



                emit gotNewCommand();
            }
            else
            {
                if (is_out_pipe_open)
                {
                    printf("Resending Cmd: %s\n",line);
                    fputs(line,out_pipe);
                    fflush(out_pipe);
                }

            }



        }
    }

    printf("NOT Waiting for Command\n");
    fflush(stdout);
}

//command like this: all are text, ascii sent over pipe.
//args are sent as type value.
// objname slotname numargs type0 val0 type1 val1 type2 val2....
// in_pipe setparams 2 int 2 QString maddog
//numargs can be 0. QStringList to 0 if no args
void textCommander::sendCommand(QString &objname, QString &slotname,int numargs,QStringList &args)
{

}

void textCommander::sendCommand(QString cmd)
{
    if (is_out_pipe_open)
    {
        printf("Send Cmd: %s\n",cmd.toStdString().c_str());
        fputs(cmd.toStdString().c_str(),out_pipe);
        fflush(out_pipe);
    }

}
