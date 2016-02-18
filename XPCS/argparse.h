#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <QString>
#include <QStringList>

class argParse
{
public:
    argParse();


    void parseArgs(QStringList args);

    void report(void);

    int in_q_length;
    int file_q_length;
    int disp_q_length;

    int x_size;
    int y_size;

    //text param types
    enum
    {
        fccd=0,
        pipe_plugin=1
    };

    int textsend_params;

    QString cmd_out_pipe_name;
    QString cmd_in_pipe_name;


};

#endif // ARGPARSE_H
