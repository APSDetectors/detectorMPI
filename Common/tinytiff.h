
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



/*******************************************************************************
 *
 * tifflib.h
 *
 *	Author: Tim Madden
 *	Date:	8/10/03
 *	Project:MBC CCD Detector DAQ.
 *
 *
 *
 *
 ******************************************************************************/


/*
 * Include files.
 * TJM change longs to ints for 64 bit... 
 */


#ifdef _WIN32
#include <windows.h>
#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HANDLE FILE*
#endif
/*
 * Double incluson protection.
 */
#ifndef _TINYTIFF_H
	#define _TINYTIFF_H

//namespace tinytiff_plugin {

class tinytiff_exception
{
	public:
		enum error_code
		{
			ok,
			xfer_not_starting,
			not_implemented,
			unknown
		};

		// make err message mess
		tinytiff_exception(const char *mess);
		// make default err message
		tinytiff_exception(
			error_code er,
			const char *mess);
		// make default err message
		tinytiff_exception(error_code er);
		// make default err message
		tinytiff_exception();
		// Return error message.
		char* err_mess(void);

		// Return error message.
		error_code getErrCode(void);



	protected:
	// Error message in this exception.
		char err[256];

		error_code code;

};


/*******************************************************************************
 *
 *	Class tifffile
 *
 *
 ******************************************************************************/

class tinytiff 
{
	public:
		// make file object.
		tinytiff();

		HANDLE open_w(char *filename);
		void close(HANDLE fp);
		//if we call this, we can set num_frames to store to the tiff file.
		// works only wit tifWr( with no filename.
		void setMultiFrames(int num_frames);
		// returns true if a TIFF file
		bool tifCheck(char* filename);
		bool tifCheck(HANDLE fp);
		// return true of big endian- onlu little endian supported
		bool tifEndian(void);

		void tifIFD(char* filename,unsigned int offset);

		void tifIFD(HANDLE fp,unsigned int offset);

		unsigned int tifFindIFD(void);

		void tifRd(char* filename,unsigned short* img);
		void tifRd(unsigned short* img);


		// reads info on tiff file- fills in structures
		void tifRdInfo(char* filename);
		void tifRdInfo(HANDLE fp);

		void tifWr(
			char* filename,
			unsigned short *img,
			int sizex,
			int sizey);

		void tifWr(
			unsigned short *img,
			int sizex,
			int sizey);

	


		void tifWriteIFD(void);

		void tifMakeIFD(void);
		void tifImgSpecs(void);


		char* getHeader(void);
		void putStrHeader(char * strg);



		class img_specs 
		{
		public:
			int width;
			int length;
			int bit_depth;
			int compression;
			int strip_offset;
			int strip_bytecnt;
		};

		class ifd_field 
		{
		public:
			unsigned short tag;
			unsigned short type;
			unsigned int count;
			int valoffs;

		};

		class tiff_header
		{
		public:
			unsigned short endian;
			unsigned short fortytwo;
			unsigned int first_ifd;
		};
	enum 
		{
			header_size = 4096
		};

		char head_data[header_size];

		// numbers to put into tiff ifd .
		int scalars[16];
		img_specs specs;

		//public handle to store a file pointer from woopen wclose
		// copied into fp for all operations of this class.
		HANDLE file_pointer;
		char file_name[255];

		//if true, open a pipe instead of a file.
		bool is_use_pipe;

	protected:

		int num_store_frames;
		int stored_frame_counter;
		// file pointer
	//	FILE *fp;
		//current file we use...
		HANDLE fp;
		
		//char file_name[255];

		

		// header data not tiff header- bessrc data header
	//	char head_data[header_size];
		tiff_header header;

		char *header_ptr;




		// raw IFD data buffer
		ifd_field ifd[64];
		ifd_field *ext_specs;
		unsigned short num_ifd_fields;
		unsigned int next_ifd;


};
//};//namespace
#endif

