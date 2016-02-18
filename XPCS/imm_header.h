
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/




#ifndef IMM_HEADER_H
#define IMM_HEADER_H


struct immHeader
{

qint32	     		mode;         		//comp mode
qint32	     		compression;  //comp?
quint8	  		date[32];        //date today
quint8	  		prefix[16];       //?
qint32	     		number;     // filenum
quint8	  		suffix[16];
qint32	     		monitor;    //0
qint32	     		shutter;    //0
qint32	     		row_beg;    //
qint32	   		row_end;      // whatever they are
qint32	     		col_beg;//fseek 31
qint32	   		col_end;
qint32	     		row_bin;      //1
qint32	     		col_bin;      //1
qint32	     		rows;         //
qint32	     		cols;
qint32	     		bytes;        //2
qint32	     		kinetics;     //0 part of ccd
qint32	     		kinwinsize;   //0
double	  		elapsed;      //timestamp
double	  		preset;       //exp time
qint32	     		topup;     //0
qint32	     		inject;       //0
 quint32	     		dlen;
qint32	     		roi_number;   //1

qint32	  	buffer_number;	// byte 160
quint32	  	systick;//0

quint8	   		pv1[40];
float	   		pv1VAL;
quint8	   		pv2[40];
float	   		pv2VAL;
quint8	   		pv3[40];
float	   		pv3VAL;
quint8	   		pv4[40];
float	   		pv4VAL;
quint8	   		pv5[40];
float	   		pv5VAL;
quint8	   		pv6[40];
float	   		pv6VAL;
quint8	   		pv7[40];
float	   		pv7VAL;
quint8	   		pv8[40];
float	   		pv8VAL;
quint8	   		pv9[40];
float	   		pv9VAL;
quint8	   		pv10[40];
float	   		pv10VAL;

float	   		imageserver;  //make up
float	   		CPUspeed;     //0

enum {immver=12};

qint32	     		immversion;   //immver
qint32		  corecotick;   //620
qint32	     		cameratype;//
float	   		threshhold;   //my val

//here is 632 bytes- or byte 0 to 631

qint8 byte632;//the is byte 632 counting from 0 to 632

// 1024- (633+84 + 12)
qint8 empty_space[295];

enum {z_len=84};
enum {f_len = 12};

//fill with 00's
quint8 ZZZZ[z_len];

//fill with FF's
quint8 FFFF[f_len];


enum {header_size=1024};


};

#endif // IMM_HEADER_H
