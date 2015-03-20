
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



/********************************************************
 *
 *
 *
 *********************************************************/

#include "tinytiff.h"
#include "stdio.h"
#include "stdlib.h"

//namespace tinytiff_plugin {

tinytiff::tinytiff()
{
	this->file_pointer=0;
	for (int i=0;i<header_size;i++)
		head_data[i] = 0;

	header_ptr=head_data;

			 num_store_frames=1;
		 stored_frame_counter=0;

  is_use_pipe = false;


}




HANDLE tinytiff::open_w(char *filename)
{
HANDLE fx;
#ifdef _WIN32
LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\EpicsPipedTiff"); 
#endif 

stored_frame_counter=0;
#ifdef _WIN32

if (!is_use_pipe)
{
	fx = CreateFile(
			filename,           // file to create 
             GENERIC_WRITE,                // open for writing 
             0,                            // do not share 
             NULL,                         // default security 
             CREATE_ALWAYS,                // overwrite existing 
             FILE_ATTRIBUTE_NORMAL,       // normal file 
             NULL);                        // no attr. template 

}
else
{

fx= CreateNamedPipe(
 lpszPipename,
  PIPE_ACCESS_OUTBOUND,
  PIPE_TYPE_BYTE|PIPE_NOWAIT,
  PIPE_UNLIMITED_INSTANCES,
  512,
  512,
  0,
  NULL);

}


if (fx==INVALID_HANDLE_VALUE)
{
	printf("tinytiff.cpp: open_w- could not CreateFile or CreateNamedPipe\n");

}
#else
fx=	fopen(filename,"w");
#endif

this->file_pointer=fx;
this->fp=fx;
strcpy(this->file_name,filename);
return(fx);

}

void tinytiff::close(HANDLE fx)
{
	
	if (fx==0)
		return;
	
#ifdef _WIN32
CloseHandle(fx);
#else
fclose(fx);
#endif
this->file_pointer = 0;
this->fp = 0;
}

/////////////////////////////////////////////////////
//
//ifd ifdlen header specs extspecs nextifd all in class space
///////////////////////////////////////////////////

bool tinytiff::tifCheck(char* filename)
{ 
#ifdef _WIN32
 	
	DWORD got;
#else
	unsigned int got;
#endif

	bool is_tiff =false;

 // fp = fopen(filename,'r','n');
#ifdef _WIN32

	fp = CreateFile(filename,           // file to create 
             GENERIC_READ,                // open for writing 
             0,                            // do not share 
             NULL,                         // default security 
             OPEN_EXISTING,                // overwrite existing 
             FILE_ATTRIBUTE_NORMAL,       // normal file 
             NULL);  // no attr. template 

	


	if (fp == INVALID_HANDLE_VALUE)
		throw tinytiff_exception("tinytiff::tifCheck bad file handle");


	ReadFile( 
		fp, 
		(unsigned char*) &header, 
		 sizeof(tiff_header), 
		&got, 
		0
		); 
#else
	fp = fopen(filename,"r");
	if (fp == 0)
		throw tinytiff_exception("tinytiff::tifCheck bad file handle");


	fread((void *)&header,sizeof(tiff_header),1,fp);

#endif



#ifdef _WIN32
	CloseHandle(fp);
#else
		fclose(fp);
#endif

		try 
		{
			tifEndian();
		}
		catch (tinytiff_exception error)
		{
			is_tiff = 0;
			return(is_tiff);
		}

	if (header.fortytwo == 42)
	{
		is_tiff = 1;
	}
	return(is_tiff);

}


void tinytiff::setMultiFrames(int num_frames)
{
  num_store_frames=num_frames;
}
/////////////////////////////////////////////////////
//
//ifd ifdlen header specs extspecs nextifd all in class space
///////////////////////////////////////////////////

bool tinytiff::tifCheck(HANDLE fp)
{ 
#ifdef _WIN32
	DWORD got;
#else 	
	unsigned int got;
#endif

	bool is_tiff =false;

	this->fp = fp;
 // fp = fopen(filename,'r','n');
#ifdef _WIN32

		SetFilePointer(
			fp,
			0,
			0,
			FILE_BEGIN);

	ReadFile( 
		fp, 
		(unsigned char*) &header, 
		 sizeof(tiff_header), 
		&got, 
		0
		); 

		SetFilePointer(
			fp,
			0,
			0,
			FILE_BEGIN);

#else
		fseek(fp,0,SEEK_SET);


	fread((unsigned char*) &header,sizeof(tiff_header), 1,fp);

		fseek(fp,0,SEEK_SET);


#endif
		try 
		{
			tifEndian();
		}
		catch (tinytiff_exception error)
		{
			is_tiff = 0;
			return(is_tiff);
		}
	

	if (header.fortytwo == 42)
	{
		is_tiff = 1;
	}
	return(is_tiff);

}

/////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////


bool tinytiff::tifEndian(void)
{
	bool is_big;
 if (header.endian == 0x4949)//0x4949=18761
 { 
  is_big = false;
  }
 else if (header.endian == 0x4d4d)
 {
	 is_big=true;
 }
 else 
	 throw tinytiff_exception("tinytiff::tifEndian  doesn't seem to be TiFF"); 

	 
	 return is_big;

}

/////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////



///////////////////////////////////////////////////
//
//define these args in the clss- ifd, nextifd, ifdlen. make a void
//
//////////////////////////////////////////////////////
void tinytiff::tifIFD(char* filename,unsigned int offset)
{
  unsigned int ifdlen;
  bool is_tiff;

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif

  unsigned int dir_start;

	is_tiff = tifCheck(filename);

	if (is_tiff)
	{
		if (offset==0)
		{
			dir_start = tifFindIFD();
		}
		else
		{
			dir_start = offset;
		}

#ifdef _WIN32

		fp = CreateFile(filename,           // file to create 
             GENERIC_READ,                // open for writing 
             0,                            // do not share 
             NULL,                         // default security 
             OPEN_EXISTING,                // overwrite existing 
             FILE_ATTRIBUTE_NORMAL,       // normal file 
             NULL);  // no attr. template 


		if (fp == INVALID_HANDLE_VALUE)
			throw tinytiff_exception("tinytiff::tifIFD bad file handle");






//		fseek(fp,dir_start,'bof');
		SetFilePointer(
			fp,
			(int)dir_start,
			0,
			FILE_BEGIN);




		//fields = fread(fp,1,'ushort');

		ReadFile( 
			fp, 
			(unsigned char*)&num_ifd_fields, 
			sizeof(unsigned short), 
			&got, 
			0
			); 


		ifdlen=num_ifd_fields*12;
	    
//		ifd = fread(fp,fields*12,'uchar');
		ReadFile( 
			fp, 
			(unsigned char*)ifd, 
			ifdlen, 
			&got, 
			0
			); 

//		nextifd = fread(fp,1,'ul');
		ReadFile( 
			fp, 
			(unsigned char*)&next_ifd, 
			sizeof(unsigned int), 
			&got, 
			0
			); 
	    
	    
		

	CloseHandle(fp);
#else
		fp=fopen(filename,"r");
		if (fp == 0)
			throw tinytiff_exception("tinytiff::tifIFD bad file handle");

		fseek(fp,dir_start,SEEK_SET);
		fread(&num_ifd_fields,sizeof(short),1,fp);
		ifdlen=num_ifd_fields*12;
		fread(ifd,1,ifdlen,fp);
			fread(&next_ifd,sizeof(unsigned int),1,fp);

	fclose(fp);


#endif


	for (int k=0;k<16;k++)
		scalars[k] = ifd[k+11].valoffs;


	}
	else
	{
		throw tinytiff_exception("tinytiff::tifIFD Not a Tiff File");
	} 
	
 
}

void tinytiff::tifIFD(HANDLE fp,unsigned int offset)
{
  unsigned int ifdlen;
  bool is_tiff;


#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif


  unsigned int dir_start;

  this->fp = fp;

	is_tiff = tifCheck(fp);

	if (is_tiff)
	{
		if (offset==0)
		{
			dir_start = tifFindIFD();
		}
		else
		{
			dir_start = offset;
		}

		

#ifdef _WIN32
		SetFilePointer(
			fp,
			(int)dir_start,
			0,
			FILE_BEGIN);

		ReadFile( 
			fp, 
			(unsigned char*)&num_ifd_fields, 
			sizeof(unsigned short), 
			&got, 
			0
			); 
		ifdlen=num_ifd_fields*12;
	    
		ReadFile( 
			fp, 
			(unsigned char*)ifd, 
			ifdlen, 
			&got, 
			0
			); 

		ReadFile( 
			fp, 
			(unsigned char*)&next_ifd, 
			sizeof(unsigned int), 
			&got, 
			0
			); 
	    
	  
		#else

		fseek(fp,dir_start,SEEK_SET);
		fread(&num_ifd_fields,sizeof(short),1,fp);
		ifdlen=num_ifd_fields*12;
		fread(ifd,1,ifdlen,fp);
			fread(&next_ifd,sizeof(unsigned int),1,fp);



#endif

		for (int k=0;k<16;k++)
			scalars[k] = ifd[k+11].valoffs;


	}
	else
	{
		throw tinytiff_exception("tinytiff::tifIFD Not a Tiff File");
	} 
	
 
}



///////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////


unsigned int tinytiff::tifFindIFD(void)
{
  return(header.first_ifd);
}
///////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////

void tinytiff::tifRd(char* filename,unsigned short* img)
{
// single img tif only
//unsigned char ifd[1024]; define in class 
//unsigned int nextifd; define in class 
unsigned int offset = 0;
int strt;
int sz;

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif

//unsigned int ifdlen; define in class 
//unsigned int specs[6]; define in header in class scope
// make emptuy IFD
		specs.width = 0;
	specs.length= 0;
	specs.bit_depth=16;
	specs.compression = 1;
	specs.strip_offset = sizeof(tiff_header);
	specs.strip_bytecnt = 0*0*sizeof(unsigned short);
// make default empty ifd
	tifMakeIFD();
//read file and overrite the ifd

	tifIFD(filename,0);

	tifImgSpecs();
    
    strt = specs.strip_offset;
    sz = specs.strip_bytecnt;
#ifdef _WIN32

	fp = CreateFile(filename,           // file to create 
        GENERIC_READ,                // open for writing 
        0,                            // do not share 
        NULL,                         // default security 
        OPEN_EXISTING,                // overwrite existing 
        FILE_ATTRIBUTE_NORMAL,       // normal file 
        NULL);  // no attr. template 

		SetFilePointer(
			fp,
			strt,
			0,
			FILE_BEGIN);

		ReadFile( 
			fp, 
			(unsigned char*)img, 
			sz, 
			&got, 
			0
			); 
    
	CloseHandle(fp);
#else
	fopen(filename,"r");

	fseek(fp,strt,SEEK_SET);

	fread(img,1,sz,fp);
    
fclose(fp);


#endif


	//read in the header etc.
	tifRdInfo(filename);

 

}
///////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////

void tinytiff::tifRd(unsigned short* img)
{
// single img tif only
//unsigned char ifd[1024]; define in class 
//unsigned int nextifd; define in class 
unsigned int offset = 0;
int strt;
int sz;

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif

//unsigned int ifdlen; define in class 
//unsigned int specs[6]; define in header in class scope
this->fp = this->file_pointer;

		specs.width = 0;
	specs.length= 0;
	specs.bit_depth=16;
	specs.compression = 1;
	specs.strip_offset = sizeof(tiff_header);
	specs.strip_bytecnt = 0*0*sizeof(unsigned short);

	tifMakeIFD();

	tifIFD(fp,0);

	tifImgSpecs();
    
    strt = specs.strip_offset;
    sz = specs.strip_bytecnt;

#ifdef _WIN32

		SetFilePointer(
			fp,
			strt,
			0,
			FILE_BEGIN);

		ReadFile( 
			fp, 
			(unsigned char*)img, 
			sz, 
			&got, 
			0
			); 
#else
	fseek(fp,strt,SEEK_SET);

	fread(img,1,sz,fp);
    
#endif

	//read in the header etc.
	tifRdInfo(fp);
}

/////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////

void tinytiff::tifRdInfo(char* filename)
{
// single img tif only

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif
unsigned int strt;
unsigned int sz;

		specs.width = 0;
	specs.length= 0;
	specs.bit_depth=16;
	specs.compression = 1;
	specs.strip_offset = sizeof(tiff_header);
	specs.strip_bytecnt = 0*0*sizeof(unsigned short);

	tifMakeIFD();


 tifIFD(filename,0);
 tifImgSpecs();
	if (ifd[11].tag==6000)
	{
    
	    strt =ifd[11].valoffs;
		 sz = ifd[11].count;

#ifdef _WIN32

 	fp = CreateFile(filename,           // file to create 
        GENERIC_READ,                // open for writing 
        0,                            // do not share 
        NULL,                         // default security 
        OPEN_EXISTING,                // overwrite existing 
        FILE_ATTRIBUTE_NORMAL,       // normal file 
        NULL);  // no attr. template 

		SetFilePointer(
			fp,
			strt,
			0,
			FILE_BEGIN);

		ReadFile( 
			fp, 
			(unsigned char*)head_data, 
			sz, 
			&got, 
			0
			); 
    
	CloseHandle(fp);
#else

		 fopen(filename,"r");

		fseek(fp,strt,SEEK_SET);

		fread(head_data,1,sz,fp);

		fclose(fp);

#endif




	}
  
}


void tinytiff::tifRdInfo(HANDLE fp)
{
// single img tif only

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif

  unsigned int strt;
unsigned int sz;

this->fp = fp;

		specs.width = 0;
	specs.length= 0;
	specs.bit_depth=16;
	specs.compression = 1;
	specs.strip_offset = sizeof(tiff_header);
	specs.strip_bytecnt = 0*0*sizeof(unsigned short);

	tifMakeIFD();

 tifIFD(fp,0);
 tifImgSpecs();
    
	if (ifd[11].tag==6000)
	{
		strt =(unsigned int)ifd[11].valoffs;
		sz = ifd[11].count;

#ifdef _WIN32

		SetFilePointer(
			fp,
			strt,
			0,
			FILE_BEGIN);

		ReadFile( 
			fp, 
			(unsigned char*)head_data, 
			sz, 
			&got, 
			0
			); 
#else

		fseek(fp,strt,SEEK_SET);

		fread(head_data,1,sz,fp);

#endif

    
	}  
}


///////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
void tinytiff::tifWr(
	char* filename,
	unsigned short *img,
	int sizex,
	int sizey)
{


#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif


	specs.width = sizex;
	specs.length= sizey;
	specs.bit_depth=16;
	specs.compression = 1;
	specs.strip_offset = sizeof(tiff_header);
	specs.strip_bytecnt = sizex*sizey*sizeof(unsigned short);

	tifMakeIFD();
	header.endian = 18761;
	header.fortytwo  = 42;
	header.first_ifd = specs.strip_bytecnt + sizeof(tiff_header)+header_size;

#ifdef _WIN32

if (!is_use_pipe)
{
	fp = CreateFile(
			filename,           // file to create 
             GENERIC_WRITE,                // open for writing 
             0,                            // do not share 
             NULL,                         // default security 
             CREATE_ALWAYS,                // overwrite existing 
             FILE_ATTRIBUTE_NORMAL,       // normal file 
             NULL);                        // no attr. template 


}
else
{
fp= CreateNamedPipe(
  "\\\\.\\pipe\\EpicsPipedTiff",
  PIPE_ACCESS_OUTBOUND,
  PIPE_TYPE_BYTE|PIPE_NOWAIT|PIPE_REJECT_REMOTE_CLIENTS,
  PIPE_UNLIMITED_INSTANCES,
  10000000,
  10000,
  100,
  0);
}

//write TIFF header



	WriteFile(
	fp,
		&header,
		sizeof(tiff_header),
		&got,
		NULL
		);

	//write image
	WriteFile(
	fp,
		(unsigned char*) img,
		specs.strip_bytecnt,
		&got,
		NULL
		);
	//write header
	WriteFile(
	fp,
		(unsigned char*) head_data,
		header_size,
		&got,
		NULL
		);

	ifd[11].tag = 6000;
	ifd[11].type = 2;
	ifd[11].count = 4096;
	ifd[11].valoffs = sizeof(tiff_header) + specs.strip_bytecnt;


tifWriteIFD();


	WriteFile(
	fp,
		(unsigned char*) &next_ifd,
		sizeof(int),
		&got,
		NULL
		);


CloseHandle(fp);

#else
    fp=	fopen(filename,"w");

//write TIFF header

	fwrite(&header,sizeof(tiff_header),1,fp);

	//write image
	fwrite(img,1,specs.strip_bytecnt,fp);
	//write header
	fwrite(head_data,1,header_size,fp);

	ifd[11].tag = 6000;
	ifd[11].type = 2;
	ifd[11].count = 4096;
	ifd[11].valoffs = sizeof(tiff_header) + specs.strip_bytecnt;


tifWriteIFD();

	fwrite(&next_ifd,sizeof(int),1,fp);

fclose(fp);

#endif

}

///////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////
void tinytiff::tifWr(
	unsigned short *img,
	int sizex,
	int sizey)
{


#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif

	if (stored_frame_counter>=num_store_frames)
		throw tinytiff_exception("tinytiff::tifWr Attempt to store too many frames");


	this->fp = this->file_pointer;

	specs.width = sizex;
	specs.length= sizey;
	specs.bit_depth=16;
	specs.compression = 1;

	// this tells where data is rel to start of file. we are limited to 32 bits
	// will be tiffheader + numimgs*(ifdsize + imgsize + medadatasize+ ifdnext + ifdnumfields)
	specs.strip_offset = sizeof(tiff_header) + 
		stored_frame_counter * (
			sizex*sizey*sizeof(unsigned short) + 
			header_size + 
			num_ifd_fields*sizeof(ifd_field) + 
			sizeof(unsigned short) + 
			sizeof(unsigned int));

	specs.strip_bytecnt = sizex*sizey*sizeof(unsigned short);

	tifMakeIFD();

		//we put metadata right after the image data.
	ifd[11].tag = 6000;
	ifd[11].type = 2;
	ifd[11].count = 4096;
	ifd[11].valoffs = specs.strip_offset + specs.strip_bytecnt;

	header.endian = 18761;
	header.fortytwo  = 42;
	header.first_ifd = specs.strip_bytecnt + sizeof(tiff_header)+header_size;

		// if last image to put in tiff file next ifd =0, else calc offset to next ifd
	if (num_store_frames==(stored_frame_counter+1))
		next_ifd=0;
	else
	{
		//calc offset to where next ifd will be next time we store a shot in this tiff file
		//we write in this order: tiff header (for 1st shotonly), image data, user header data, ifd,nextifd
		//size of image data
		// size of data header the user sets
		next_ifd=0;
		//image start plus image size= end of image
		next_ifd+=specs.strip_offset + specs.strip_bytecnt;
		// user header data size
		next_ifd+=header_size;

		//now we should point to start of THIS ifd. Now add NEXT Image and NEXT header
		next_ifd+=num_ifd_fields*sizeof(ifd_field);// add the IFD we will write (THIS IFD)
		next_ifd+=sizeof(int);// int at end of THIOS image ifd
		next_ifd+=sizeof(short);// int at beg of THIOS image ifd

		//NEXT image data
		next_ifd+=specs.strip_bytecnt;//next image data
		// user header data size
		next_ifd+=header_size;
		//now we are at start pf MEXT IFD
	}

//write TIFF header

#ifdef _WIN32

	// if 1st frame we write the header
	if (stored_frame_counter==0)
	{
		WriteFile(
		fp,
			&header,
			sizeof(tiff_header),
			&got,
			NULL
			);
	}

	//write image
	WriteFile(
	fp,
		(unsigned char*) img,
		specs.strip_bytecnt,
		&got,
		NULL
		);
	//write header
	WriteFile(
	fp,
		(unsigned char*) head_data,
		4096,
		&got,
		NULL
		);



tifWriteIFD();


	WriteFile(
	fp,
		(unsigned char*) &next_ifd,
		sizeof(int),
		&got,
		NULL
		);


#else
//write TIFF header

	// we write the tiff header only on 1st image
	if (stored_frame_counter==0)
	{
		fwrite(&header,sizeof(tiff_header),1,fp);
	}
	//write image
	fwrite(img,1,specs.strip_bytecnt,fp);
	//write user metat data 
	fwrite(head_data,1,header_size,fp);

	

//write the ifd
tifWriteIFD();

	// inc the counter as we just finish an image
	



	fwrite(&next_ifd,sizeof(int),1,fp);


#endif

stored_frame_counter++;
}



/////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////
void tinytiff::tifWriteIFD(void)
{
	bool is_ok;

#ifdef _WIN32
DWORD got;
#else
  unsigned int got;
#endif
	

	
#ifdef _WIN32
// query the file posisiton- setting CURRENT and 0,0 does a query, meaning move 0 from current.
LARGE_INTEGER movedst;
LARGE_INTEGER fpos;

movedst.HighPart=0;
movedst.LowPart=0;
	SetFilePointerEx(
	 fp,
	 movedst,
	&fpos,
	FILE_CURRENT
		);


// write num of foelds
	is_ok =WriteFile(
	fp,
		(unsigned char*) &num_ifd_fields,
		sizeof(unsigned short),
		&got,
		NULL
		);

//write ifd

		is_ok =WriteFile(
			fp,
		(unsigned char*) ifd,
		num_ifd_fields*sizeof(ifd_field),
		&got,
		NULL
		);
#else
// write num of foelds

	fwrite(&num_ifd_fields,sizeof(unsigned short),1,fp);

//write ifd
	size_t sifd = sizeof(ifd_field);

	fwrite(ifd,sifd,num_ifd_fields,fp);

#endif

}

/////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////

void tinytiff::tifMakeIFD(void)
{
 int field;
	num_ifd_fields = 27;
	next_ifd = 0;

	ifd[0].tag = 256; 
	ifd[0].type = 3;
	ifd[0].count = 1;
	ifd[0].valoffs = specs.width;


	ifd[1].tag = 257; 
	ifd[1].type = 3;
	ifd[1].count = 1;
	ifd[1].valoffs = specs.length;


	ifd[2].tag = 258; 
	ifd[2].type = 3;
	ifd[2].count = 1;
	ifd[2].valoffs = specs.bit_depth;


	ifd[3].tag = 259; 
	ifd[3].type = 3;
	ifd[3].count = 1;
	ifd[3].valoffs = specs.compression;


	ifd[4].tag = 262; 
	ifd[4].type = 3;
	ifd[4].count = 1;
	ifd[4].valoffs = 1;


	ifd[5].tag = 270; 
	ifd[5].type = 2;
	ifd[5].count = 0;
	ifd[5].valoffs = 0;


	ifd[6].tag = 273; 
	ifd[6].type = 4;
	ifd[6].count = 1;
	ifd[6].valoffs = specs.strip_offset;


	ifd[7].tag = 277; 
	ifd[7].type = 3;
	ifd[7].count = 1;
	ifd[7].valoffs = 1;


	ifd[8].tag = 278; 
	ifd[8].type = 3;
	ifd[8].count = 1;
	ifd[8].valoffs = specs.width;


	ifd[9].tag = 279; 
	ifd[9].type = 4;
	ifd[9].count = 1;
	ifd[9].valoffs = specs.strip_bytecnt;

	ifd[10].tag = 284; 
	ifd[10].type = 3;
	ifd[10].count = 1;
	ifd[10].valoffs = 1;

	ifd[11].tag = 6000; 
	ifd[11].type = 3;
	ifd[11].count = 0;
	ifd[11].valoffs = 0;

	 for (field=12;field<num_ifd_fields;field++)
	 {
		ifd[field].tag = 6000 + field - 11; 
		ifd[field].type = 4;
		ifd[field].count = 1;
		ifd[field].valoffs = scalars[field-11];
	 }

}

/////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////


void tinytiff::tifImgSpecs(void)
{
int field;

 for (field=0;field<num_ifd_fields;field++)
 {
//  [tag, typ, numval, valoff] = tifParseIFD(field,ifd);
 
  if (ifd[field].tag == 256) //img width
    specs.width = ifd[field].valoffs;
 
  if (ifd[field].tag == 257) //img len
    specs.length = ifd[field].valoffs;
  
  


  if (ifd[field].tag == 258) //bits per samp
     specs.bit_depth = ifd[field].valoffs;
 
  
  
  if (ifd[field].tag == 259) //compression
     specs.compression = ifd[field].valoffs;
 
  
  
  if (ifd[field].tag == 273) //strip offsets - where img is
    specs.strip_offset = ifd[field].valoffs;
  
  
  if (ifd[field].tag == 279) //img dtat length
     specs.strip_bytecnt = ifd[field].valoffs;
  
  

 }

}



char* tinytiff::getHeader(void)
{

	return head_data;
}

void tinytiff::putStrHeader(char * strg)
{
		
	strcpy(header_ptr,strg);
	while (*header_ptr != 0) header_ptr++;

}


/****************************************************************************8
*
*
*
*
*******************************************************************************/



tinytiff_exception::tinytiff_exception()
{
	strcpy(err,"error");
	code = unknown;
}
		// make default err message
tinytiff_exception::tinytiff_exception(
			error_code er,
			const char *mess)
{
	strcpy(err,mess);
	code = er;
}
		// make default err message
tinytiff_exception::tinytiff_exception(error_code er)
{
	code = er;
}
		// make default err message

tinytiff_exception::tinytiff_exception(const char *x)
{
	strcpy(err,x);
	code = unknown;

}


char* tinytiff_exception::err_mess(void)
{
	return err;
}


// Return error message.
tinytiff_exception::error_code tinytiff_exception::getErrCode(void)
{
	return code;
}

//};//namespace
