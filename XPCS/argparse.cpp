#include "argparse.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

argParse::argParse() :
    cmd_out_pipe_name("/local/xpcscmdout"),
  cmd_in_pipe_name("/local/xpcscmdin")
{
     in_q_length=20;
     file_q_length=20;
     disp_q_length=20;

     x_size=1152;
     y_size=1000;

     textsend_params=pipe_plugin;

}

void argParse::report(void)
{
    cout<<"Starting xpcsMPI"<<endl;
    cout<<"in_q_length "<< in_q_length <<endl;
    cout<<"file_q_length "<< file_q_length <<endl;
    cout<<"x_size "<< x_size <<endl;
    cout<<"y_size "<< y_size <<endl;
    cout<<"cmd_out_pipe_name "<< cmd_out_pipe_name.toStdString() <<endl;
    cout<<"cmd_in_pipe_name "<< cmd_in_pipe_name.toStdString() <<endl;
    cout<<"textsend_params "<< textsend_params <<endl;




}

void argParse::parseArgs(QStringList args)
{
    int num_args = args.size();

    bool ok;


    int curr_arg =1;

    while (curr_arg<num_args)
    {
        QString arg= args.at(curr_arg);

        if (arg=="--cmd_out_pipe_name")
        {
            curr_arg++;
            cmd_out_pipe_name = args.at(curr_arg);
        }
        if (arg=="--cmd_in_pipe_name")
        {
            curr_arg++;
            cmd_in_pipe_name = args.at(curr_arg);
        }

        if (arg=="--in_q_length")
        {
            curr_arg++;
            in_q_length = args.at(curr_arg).toInt(&ok,10);
        }
        if (arg=="--file_q_length")
        {
            curr_arg++;
            file_q_length = args.at(curr_arg).toInt(&ok,10);
        }
        if (arg=="--disp_q_length")
        {
            curr_arg++;
            disp_q_length = args.at(curr_arg).toInt(&ok,10);
        }

        if (arg=="--textsend_params")
        {
            curr_arg++;
            disp_q_length = args.at(curr_arg).toInt(&ok,10);
        }



        if (arg=="--xsize")
        {
            curr_arg++;
            x_size = args.at(curr_arg).toInt(&ok,10);
        }

        if (arg=="--ysize")
        {
            curr_arg++;
            y_size = args.at(curr_arg).toInt(&ok,10);
        }




        curr_arg++;

    }
}
