#ifndef TEXTCOMMANDER_H
#define TEXTCOMMANDER_H

#include <QString>
#include <QObject>
#include <QMap>
#include <stdio.h>

class textCommander : public QObject
{
    Q_OBJECT

public:
    textCommander(QString in_pipe_name_, QString out_pipe_name_);

    void addNameObject( QString name,  QObject *obj);
    void setAppName(QString name);

   signals:
    void gotNewCommand();

   public slots:

    bool openInputPipe(void);
    bool openOutputPipe(void);

    bool closeInputPipe(void);
    bool closeOutputPipe(void);

    bool waitNextCommand(void);
    //command like this: all are text, ascii sent over pipe.
    //args are sent as type value.
    // objname slotname numargs type0 val0 type1 val1 type2 val2....
    // in_pipe setparams 2 int 2 QString maddog
    //numargs can be 0. QStringList to 0 if no args
    //!! only 0 or 1 args supported...
    void sendCommand(QString &objname, QString &slotname,int numargs,QStringList &args);

    void sendCommand(QString cmd);
    //start sleep processes to connt o the pipes
    bool isOpenSleeps(bool is_in, bool is_out);

protected:

    bool is_sleep_in;
    bool is_sleep_out;

    bool is_makefifo;

    QMap<QString,QObject*> objmap;
    QString in_pipe_name;
    QString out_pipe_name;

    //output pipe- named linux pipe.
    FILE *in_pipe;
    bool is_in_pipe_open;

    QString app_name;

    //output pipe- named linux pipe.
    FILE *out_pipe;
    bool is_out_pipe_open;

    bool is_waiting;


    char sleep_procid_in[255];
    char sleep_procid_out[255];
};


#endif // TEXTCOMMANDER_H
