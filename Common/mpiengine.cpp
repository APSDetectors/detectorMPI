
/*********************************************************************************
 *  This code developed at Argonne National Laboratory
 *  Author: Timothy Madden, Sufeng Niu
 *  tmadden@anl.gov
 *
 *  March, 2015
 *
 *
 *********************************************************************************/



#include "mpiengine.h"
#include <QHostAddress>
/*
 *have onluy ONE workd of processes... make it simple.
 *barriers and fences block everyone.
 *each process has a window of mrmroy so every rank does a window create
 *
 *rank 0 runs stream input port. on signal on new idp image, cinrcvrmpi slot takes new image off fifo
 *and copies into public image buffer.
 *
 *rank 0
 *
 *.bcast (from rank0) new image. or other message. the message tells new image, and what calcs to do.
 *copy new image into public memory
 *
 *fence
 *get a chunk of image into proveate memory cor calcs
 *fence
 *
 *be w worker- nothing else to do... compute part of image
 *barrier (let workers finish)
 *  fence
 *put result to rank 1
 *fence
 *
 *barrier
 *
 *
 *rank1
 *this is last process- it is the qt gui user uses.
 *bcast (from rank 0
 *fance,
 *get a chink of iamge from rank0
 *fence,
 *
 *be a worker- compute part of image calcs
 *barrier- workers done
 *fence
 *include whatever result into funal image.(copy to public area of rank 1)
 *fence
 *send result to regular c code(gui and epics)
 *barier
 *
 *rank 2,N-1
 *these are worker processes
 *bcast from rank0 (new image)
 *fence
 *get part of image from rank 0
 *fence
 *do calcs
 *barrier
 *fence
 *put chunk of data(result) to rank 1.
 *fance
 *barrier
 *
 *
 *data flow
 *fccdcin-->udp---> rank 0---->rank2,N-1--->rank1-->epics
 *
 *rank 1 has udp conn to cin command ports.
 *rank0 has udp conn to cin data ports
 *
 *
 *
 *
 *rank0 has reg sreamimput port that gets cin data over udp
 *rank1 has streaminpit mpi that gets final image after mpi calcs.
 *all ranks have mpiu eninge.
 *
 *rank0 works as such:
 *0 sep. thrad calles MPIRcv, waits for any message from rank 1. this is for setting uyp cinnetwork, conn.diss con network. multh
 *
 *1 new image from cin complete, gets put into fifo. qt signal new image
 *2. qt slot in cinrcvrmpi for new image. copy to public mem and do mpi oneloop
 *3. do nothing until next signal from new image.
 *
 *rank1 is this:
 *0 mainloop thread may or may not call MPISend to rank 0, to set something up, like conn cin network. multi thread mpi here!!!
 *1. sep. thread runs mpi one loop in streaminportmpi, clocks on 1st bcast. waits for new image. runs mpioneloop
 *2. when new image over mpi it does work, and finally gets final answer from other ranks. mpione lop done
 *3.  in streaminpitmpi copies iomage into fifo, signals new imqge,
 *4. mainwindow slot gets sinal and new image from fifo- runs normal qt stuff for fccd, sending to disp and epics.
 *
 *rank2, N-1 do this:
 *1. run infinete loop - just runs mpioneloop over and over. no gui at all.
 **/

//!!MPI_Datatype mpiEngine::def_types[7] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_LONG};
//!!int mpiEngine::def_len[7] = {1, 1, 1, 1, 1, 1, 1};

mpiEngine::~mpiEngine()
{


}

/*****************************************************************************************************
 * We have a few main issues with mpiEngine:
 *1) We have up to 32 public images that are ushorts. this is a list of pointers points into
 * a single continuous mempry called public_short_image_memory. The list of points is
 * public_short_image[]. These iamges can be shared as MPI window data. any data that must
 *be transferred between ranks should be in public_short_image[].
 *2) likewise we ahve a group of double images ahtat are a similar window. this is for transferring
 *double data between tranks. the memory is called public_double_image_memory. The images are a list of
 *pointers into public_double_image_memory called public_double_image.
 *3) to set up these mem spaces we set
 *    pub_sh_img_size_bytes- size in bytes of ONE ushort image.
 *    pub_sh_img_nimgs- number of ushort iamges
 *   pub_db_img_size_bytes- size in bytes of ONE double image.
 *    pub_db_img_nimgs- number of double iamges
 *
 *   We set the above varirales can call setupMPI(). Call shutdownMPI, reset variables and call setupMPI
 *   to cahnge.
 *4) num_imgs_per_rank- this var is generatlly set to 1. that means that when we process images in real time,
 *each rank computes 1 image. If we have small images, we set num_imgs_per_rank to a larger number.
 *5)  Rank0 gets iamges from a detector and scatters them to all ranks, keeping some for itself.
 *   Then all ranks process images in tandem.
 *  finally, Rank 1 gathers images from all ranks and sends to gui and output.
 *6)  When we scatter- we figure out how many iamges to get:num_imgs_per_rank * nranks.
 *   We send  num_imgs_per_rank to rank0, then num_imgs_per_rank rank1, 2, etc.
 *7) of we only have 1 image to get, we send 1 image to rank0, and only rank0 does calcs. Therefore
 *there is a flag that determines if any rank has any work to do.
 *8) There is a special trick for integrating images. Say we have to average 161 images using 2 ranks.
 *  Say the detector has a random image rate, so sometimes only 1 rank gets an image at once, and sometimes
 * 2 ranks get images. The result is that rank0 may integrate 97 images, and rank1 gets 64 images. Furthermore,
 * at the end of the integtation, where we onluy need one more images to complete 161, the detector returns
 * 2 frames, and we scatter both. then we may be integrating an extra image.
 *
 *  The above problem is solved by keeping count of the total number of images scattered to all ranks.
 *  that means sum(rank0, rank1...), the total number images each time we integrate. We do a simple calc where
 *  all ranks know the total number of scattered images. if that number reaches 161, all ranks stop. if for example
 *the number passes 161, where we scatter 162 images , then all ranks will ahve this 162 number known to them.
 * in this case only low order ranks will integrate a new image until we get exactly 161. taht is, for 2 ranks, and 162
 *iamges scattered, rank0 will compute the 161'st image, and rank1 will ignore the 162nd image.
 *9) once all the images are integrated, we end iup with several partial sums in each rank. then each rank will send
 *its partial sim to rank1. rank1 will accum the partial sums.
 *10) next, each rank will GET the total sum from rank1 to have a local copuy of the final integrated sum.
 *
 *
 *another important thing is that mpiengine has a way to convert QT singlas into MPI messages and
 *vice versa.
 *We make a class in signalmessage.h called guiMessageFields, that is the message from th gui to all
 *mpi ranks, so gui can set up mpi.
 *We then have guiSignalMessage that is a qt signal that is sent w/ QT. mpiEngine::sendMPIGuiSettings
 *will accept a signal, then extract the guiMessageFields and send as a bionary message as MPI.
 *Likewise, mpiEngine::waitSentMessages() will receive this message on some other rank, then  generate
 *a corresponding guiSignalMessage QT signal in that process. a potential problem is that we are not
 *taking the time to convert into a special MPI type using MPI_INT, etc... so endianness could be messed up
 *over a network. Have to test this.
 ************** ************************************************************************************/

mpiEngine::mpiEngine(QObject *parent) :
    QObject(parent),
    my_message()
{

    int k;

    int default_size = 1024*1024*8;

     pub_sh_img_nimgs=4;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
     pub_sh_img_size_bytes=default_size*sizeof(short);//def img size- large image. in pixels
     //if we pass several images to a rank, we line them up by img_spacing_shorts.
     //this is an offset from public_short_image[n] pointer  to an image.
      img_spacing_shorts=pub_sh_img_size_bytes/sizeof(short);



       pub_db_img_nimgs=2;//default nimgs, determine size of memry. 4*pub_sh_img_size_bytes
       pub_db_img_size_bytes=default_size*sizeof(double);//def img size- large image.
      //bytes in rma memory for all proc...units are in shorts for puib_img_size2


        rank_dark_accum_cnt=0;

       //if we pass several images to a rank, we line them up by img_spacing_shorts.
       //this is an offset from public_short_image[n] pointer  to an image.
       img_spacing_doubles =  pub_db_img_size_bytes/sizeof(double);





      //if we have small images we will want one rank to do several iamges.
     num_imgs_per_rank=1;



    // frame number, this is lowest frame number for all ranks. it is frame number of rank 0.

     image_counter_rank0=0;




       is_print_trace=true;
        trace_count=0;

    err_stat=0;



    is_finish_darks_done=false;
    is_finish_darks = false;

}
/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

int mpiEngine::setupMPI2(int argc, char *argv[])
{

}

int mpiEngine::setupMPI(int argc, char *argv[])
{
    printTrace("setupMPI");

    /* MPI Initialization */

    int threadingmode;
    int req_threadmode = MPI_THREAD_MULTIPLE;

    MPI_Init_thread(&argc, &argv,req_threadmode,&threadingmode);


    if (threadingmode!=req_threadmode)
    {
        printf("========ERROR- cannot get rquested thread mode==========\n");
         fflush(stdout);
    }

    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);





        hw_info();






    // debug();



        printf("total physical process: %d\n", ttlprocs);
        fflush(stdout);
        if (ttlprocs > numprocs)
        {
            printf("Warning: number of process is not the same as cpu core number\n");
            printf("this will affect computation performance\n");
        }





   // image_info = (struct image_info_type *)malloc(sizeof(struct image_info_type));

    MPI_Barrier(MPI_COMM_WORLD);

    // check processor rank
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("-- processor %s, rank %d out of %d processors\n", processor_name, rank, numprocs);
    fflush(stdout);

    // udp need 1 process, and 1 process for writing to disk
    compprocs = numprocs - 2;

    // 1st barrier
    MPI_Barrier(MPI_COMM_WORLD);


    \
//!!make this a loop...
//!make ONE extra image. if rank is 4, we make room for 5 images.
//!
    public_short_image_memory=new unsigned short[ (pub_sh_img_size_bytes/sizeof(short))*pub_sh_img_nimgs];
    public_double_image_memory=new double[ (pub_db_img_size_bytes/sizeof(double))*pub_db_img_nimgs];

    assignPubImgPtrs();

   //!! make an array of windows, so the whole fifo can be mapped to the windows, avoiding a mem copy
    MPI_Win_create(public_short_image[0], pub_sh_img_size_bytes*pub_sh_img_nimgs, sizeof(unsigned short), MPI_INFO_NULL, MPI_COMM_WORLD, &win);


    MPI_Win_create(public_double_image[0], pub_db_img_size_bytes*pub_db_img_nimgs, sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &windouble);

     setupMPI2( argc, argv);


     MPI_Barrier(MPI_COMM_WORLD);


    return(0);
}

// if we do one img per rank, image_number=0. If we do 2 imgs per rank,
// then image_number can be 0 or 1. public_iamge[] pointers are calculted
// to look into the image mem block.
void mpiEngine::assignPubImgPtrs(void)
{
    int kk;


    public_short_image[0]= public_short_image_memory;
    public_double_image[0]= public_double_image_memory;

 //public_short_image[1]= new unsigned short[pub_img_size];
        for ( kk=1;kk<pub_sh_img_nimgs;kk++)
        {
            public_short_image[kk]=public_short_image[kk-1] + img_spacing_shorts;
        }


        for ( kk=1;kk<pub_db_img_nimgs;kk++)
        {  //ptr arithmetic assume we add num doubles.
            public_double_image[kk]=public_double_image[kk-1] + img_spacing_doubles;
        }


}






/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

// given tot num images to proc by all ranks, figure out how many
// iamges this rank must calc.
int mpiEngine::getNumImgs2CalcThisRank(int nimgs)
{
   int nimgs_thisrank=0;

      if (nimgs>(rank*num_imgs_per_rank))
          nimgs_thisrank=nimgs - rank*num_imgs_per_rank;

      if (nimgs_thisrank>num_imgs_per_rank)
          nimgs_thisrank=num_imgs_per_rank;

      return(nimgs_thisrank);
}
//figure out if this rank has to calc on any images.
//pass in total num images that need to be processed byh all ranks
bool mpiEngine::getIsCalc(int nimgs)
{
   bool is_calc_image=false;

       if (nimgs>(rank*num_imgs_per_rank))
           is_calc_image=true;

       return(is_calc_image);

}



/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

/**
void mpiEngine::debug()
{
    int i = 0;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    gethostname(hostname, sizeof(hostname));
    printf("PID %d on %s ready for attach\n", getpid(), hostname);
    fflush(stdout);
    while(0 == i)
        sleep(5);
}
*/
/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/


void mpiEngine::shutdownMPI()
{
    MPI_Barrier(MPI_COMM_WORLD);

    printTrace("shutdownMPI");

    /* clean up */
    //!!free(strip_buff);
   //!! free(comp_rank);
   //!! free(forward_rank);
   //!! free(backward_rank);



        MPI_Win_free(&win);
        MPI_Win_free(&windouble);


        delete public_short_image[0];
        delete public_double_image[0];

        shutdownMPI2();

    MPI_Finalize();

}


void mpiEngine::shutdownMPI2()
{

}

/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

//run by ranks 1 to N-1. this waits for images from mpi messaging.
// rank 0 waits for images from udp, so does not run this. it calls mpiOneloop for each image it gets
void mpiEngine::mpiCalcLoop(void)
{

    bool is_running;
int nimgs;
    int imgx,imgy;

    imgx=0;
    imgy=0;

    mpiBcastMessage mpi_message;

    is_print_trace=false;
   is_running=true;

   printf("Calc Worker Process Started-Rank %d\n",rank);
   fflush(stdout);

   while(is_running)
   {
       //rank 0 scatters, so we sync
       fenceMPIData2();
       fenceMPIData2();

        int stat=mpiOneLoop(mpi_message);



       //we are rank N, so WE  receive the BCAST
       Bcast(&mpi_message,sizeof(mpi_message), 0);

       //rank 1 will gether here, so we must fence to keep in suync
       fenceMPIData2();
       fenceMPIData2();



   }
}



/*****************************************************************************************************
 * one image is computed per rank. each rank gets whole image. for dark accum, the data needs tobe
 *comp[iled from all ranks to make one ark img. then the final dark img must e copied to all ranks.
 *give message, size, and number of images
 **************************************************************************************************/

int mpiEngine::mpiOneLoop(mpiBcastMessage &message)
{

    printTrace("mpiEngine::mpiOneLoop");



        mpiEngineRank0EditMessage( message);

       Bcast(&message,sizeof(message), 0);
      // message gets copied to mymessage
       parseMessage(message);

        doImgCalcs();
        //i think this is legal..as long as not const ref
        // any changes in img calcs are applied to message
        // message is later bcast
        message=my_message;

    MPI_Barrier(MPI_COMM_WORLD);

       return(0);

}


int mpiEngine::doImgCalcs(void)
{
    return(0);
}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/
/**
int mpiEngine::Bcast(int message,int *imgx, int *imgy, int root)
{

    printTrace("Bcast");

    int mesx[4];
    mesx[0]=message;
    mesx[1]=*imgx;
    mesx[2]=*imgy;

    // wait for message that img is read.
   // MPI_BCAST broadcasts a message from the process with rank root to all processes of the group, itself included. It is
    //called by all members of group using the same arguments for comm, root. On return, the contents of root's communication
    //buffer has been copied to all processes.     MPI_Bcast(&memindex, 1, MPI_INT, 0, MPI_FOR_WORLD);
MPI_Bcast(mesx, 4, MPI_INT, root, MPI_COMM_WORLD);

message=mesx[0];
*imgx=mesx[1];
*imgy=mesx[2];

return(message);

}

*/
void* mpiEngine::Bcast(void *message,int count, int root)
{

    printTrace("Bcast");


    // wait for message that img is read.
   // MPI_BCAST broadcasts a message from the process with rank root to all processes of the group, itself included. It is
    //called by all members of group using the same arguments for comm, root. On return, the contents of root's communication
    //buffer has been copied to all processes.     MPI_Bcast(&memindex, 1, MPI_INT, 0, MPI_FOR_WORLD);
MPI_Bcast(message, count, MPI_CHAR, root, MPI_COMM_WORLD);



return(message);

}
/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/


int mpiEngine::Recv(void *message, int count, int fromwho)
{

    printTrace("Recv");

    // wait for message that img is read.
   // MPI_BCAST broadcasts a message from the process with rank root to all processes of the group, itself included. It is
    //called by all members of group using the same arguments for comm, root. On return, the contents of root's communication
    //buffer has been copied to all processes.     MPI_Bcast(&memindex, 1, MPI_INT, 0, MPI_FOR_WORLD);
    MPI_Status status;
    int tag=1;
MPI_Recv(message, count, MPI_CHAR, fromwho,tag, MPI_COMM_WORLD,&status);



}
/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::Send(void *message, int count, int towho)
{

    printTrace("Send");
\

    // wait for message that img is read.
   // MPI_BCAST broadcasts a message from the process with rank root to all processes of the group, itself included. It is
    //called by all members of group using the same arguments for comm, root. On return, the contents of root's communication
    //buffer has been copied to all processes.     MPI_Bcast(&memindex, 1, MPI_INT, 0, MPI_FOR_WORLD);
    int tag=1;
MPI_Send(message, count, MPI_CHAR, towho,tag, MPI_COMM_WORLD);


}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::getMPIData(int target,int count,int tar_offset, int src_offset)
{
    printTrace("getMPIData");


    //!!
   // printf("GET tar %d c %d to %d so %d\n",target, count, tar_offset, src_offset);

        // RMA sync
        // here is where we get the data from the udp processes.
        // WinFence blocks the memoryt so the master cannot change it.
        MPI_Win_fence(0, win);
        // copy data from udp process to local
        //int offset = (rank-2)*image_info->buffer_size + ping_pong*image_info->image_size;
        // copty the data from udp server tio local memory.

        //if we mopve one rank to another, it is an mpi move. if we move rank N to rank N it is a simply copy
        if (this->rank!=target)
            MPI_Get(
                public_short_image[0] + src_offset,
                count,
                MPI_INT16_T,
                target,
                tar_offset,
                count,
                MPI_INT16_T,
                win);
        else if (src_offset != tar_offset)
           memcpy(this->public_short_image[0] + src_offset,this->public_short_image[0] + tar_offset,count*sizeof(unsigned short));

        // now we unlock the memory
        MPI_Win_fence(0, win);



}



/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::getMPIData2(int target,int count,int tar_offset, unsigned short *local_memory)
{
    printTrace("getMPIData2");


    //!!
   // printf("GET tar %d c %d to %d so %d\n",target, count, tar_offset, src_offset);

        // RMA sync
        // here is where we get the data from the udp processes.
        // WinFence blocks the memoryt so the master cannot change it.
        ///MPI_Win_fence(0, win);
        // copy data from udp process to local
        //int offset = (rank-2)*image_info->buffer_size + ping_pong*image_info->image_size;
        // copty the data from udp server tio local memory.

        //if we mopve one rank to another, it is an mpi move. if we move rank N to rank N it is a simply copy
        if (this->rank!=target)
            err_stat=MPI_Get(
                (void*)local_memory,
                count,
                MPI_INT16_T,
                target,
                tar_offset,
                count,
                MPI_INT16_T,
                win);
        else
            memcpy(local_memory,(char*)(this->public_short_image[0]) + tar_offset*sizeof(short),count*sizeof(unsigned short));

        // now we unlock the memory
       /// MPI_Win_fence(0, win);



}

/*****************************************************************************************************
 *
 *countis image size in pixel or shorts. local mem is opinter to img data to scatter
 *which_iamge is 0,1,2,3. it is puiblic_image area, where each area is pub_sh_img_size_bytes long.
 *which_image: 0, raw; 1, darksub/desc; 2, immcomp, 3, unused
 *
 *for 4 processes, 2 images per rank, we gather from processes in this order.
 *rank0, rank0, rank1,rank1,rank2,rank2,rank3,rank3
 **************************************************************************************************/

void mpiEngine::gatherImage(int count,unsigned short *local_memory,int which_image)
{
    int target, tar_offset;


    // figure out which target or rank we send image.
    //for 4 ranks target willbe for each gather: 0,1,2,3, if num_imgs_per_rank=1;
    //if num_imgs_per_rank=2, then target is 0,0,1,1,2,2,3,3,
    //target is the rank from which we get the image.
    //taroffset is the number of shorts offset from start of whole public window.

     target = int(scatgath_counter / num_imgs_per_rank);


     // which image tells which public_short_image[], all separt3ed by pub_sh_img_size_bytes shorts.
     // scatgath_counter%num_imgs_per_rank is 0,0,0,0 for num_imgs_per_rank=1;
     // and is 0,1,0,1,0,1 if num_imgs_per_rank=2.
     //taroffset is which_image*pub_sh_img_size_bytes + N*img_spacing_shorts.
     //
     tar_offset = (img_spacing_shorts * (scatgath_counter%num_imgs_per_rank)) +
             which_image*pub_sh_img_size_bytes;




    getMPIData2(target,count, tar_offset,  local_memory);



}



/*****************************************************************************************************
 *
 *countis image size in pixel or shorts. local mem is opinter to img data to scatter
 *which_iamge is 0,1,2,3. it is puiblic_image area, where each area is pub_sh_img_size_bytes long.
 *which_image: 0, raw; 1, darksub/desc; 2, immcomp, 3, unused
 *
 *for 4 processes, 2 images per rank, we gather from processes in this order.
 *rank0, rank0, rank1,rank1,rank2,rank2,rank3,rank3
 *
 *we get specs too, from publicimage 0, right after raw imga data. must set item->specs->imglenshorts
 *before calling so it knows where to find the specs data
 **************************************************************************************************/

void mpiEngine::gatherImage(int count, imageQueueItem *item,int which_image)
{
    int target, tar_offset;


    // figure out which target or rank we send image.
    //for 4 ranks target willbe for each gather: 0,1,2,3, if num_imgs_per_rank=1;
    //if num_imgs_per_rank=2, then target is 0,0,1,1,2,2,3,3,
    //target is the rank from which we get the image.
    //taroffset is the number of shorts offset from start of whole public window.

     target = int(scatgath_counter / num_imgs_per_rank);


     // which image tells which public_short_image[], all separt3ed by pub_sh_img_size_bytes shorts.
     // scatgath_counter%num_imgs_per_rank is 0,0,0,0 for num_imgs_per_rank=1;
     // and is 0,1,0,1,0,1 if num_imgs_per_rank=2.
     //taroffset is which_image*pub_sh_img_size_bytes + N*img_spacing_shorts.
     //
     tar_offset = (img_spacing_shorts * (scatgath_counter%num_imgs_per_rank)) +
             which_image*pub_sh_img_size_bytes;



    // get img data
    getMPIData2(target,count, tar_offset,  item->img_data);
    //get img specs from pubimg 0
    // only one image will work...

    int tar_offset2 = my_message.mpi_image_spec_offset_shorts;

    getMPIData2(target,(int)(imageSpecs::spec_len_short), tar_offset2, (unsigned short*) (item->specs));



}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::putMPIData(int target,int count,int tar_offset,int src_offset)
{

    printTrace("putMPIData");

    //!!
    //printf("PUT tar %d c %d to %d so %d\n",target, count, tar_offset, src_offset);

        // RMA sync
        // here is where we get the data from the udp processes.
        // WinFence blocks the memoryt so the master cannot change it.
        MPI_Win_fence(0, win);
        // copy data from udp process to local
        //int offset = (rank-2)*image_info->buffer_size + ping_pong*image_info->image_size;
        // copty the data from udp server tio local memory.

        if (this->rank != target)
            MPI_Put(
                public_short_image[0]+src_offset,
                count,
                MPI_INT16_T,
                target,
                tar_offset,
                count,
                MPI_INT16_T,
                win);
        else if (src_offset != tar_offset)
            memcpy(this->public_short_image[0]+tar_offset,this->public_short_image[0]+src_offset,count*sizeof(unsigned short));



        // now we unlock the memory
        MPI_Win_fence(0, win);



}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::putMPIData2(int target,int count,int tar_offset,unsigned short *local_memory)
{
    printTrace("putMPIData2");

    //!!
    //printf("PUT tar %d c %d to %d so %d\n",target, count, tar_offset, src_offset);

        // RMA sync
        // here is where we get the data from the udp processes.
        // WinFence blocks the memoryt so the master cannot change it.
        ///MPI_Win_fence(0, win);
        // copy data from udp process to local
        //int offset = (rank-2)*image_info->buffer_size + ping_pong*image_info->image_size;
        // copty the data from udp server tio local memory.

        if (this->rank != target)
            MPI_Put(
                (void*)local_memory,
                count,
                MPI_INT16_T,
                target,
                tar_offset,
                count,
                MPI_INT16_T,
                win);
        else
            memcpy((char*)(this->public_short_image[0])+tar_offset*sizeof(short),(char*)local_memory,count*sizeof(unsigned short));



        // now we unlock the memory
        //MPI_Win_fence(0, win);



}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/
void mpiEngine::resetScatGathCounter(void)
{
    // counts iamges scattered to processes. Every time we do a put to the mpi priocesses
    // this will increment. used to calc which rank the image goes to.
    scatgath_counter=0;
}



/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/

void mpiEngine::incScatGathCounter(void)
{
scatgath_counter++;
}

/*****************************************************************************************************
 *
 *countis image size in pixel or shorts. local mem is opinter to img data to scatter
 *
 *Order of scatter, say we have 4 processes, 2 iamges per rank.
 *images will scatter to ranks in this order:
 *rank0,rank0,rank1,rank1,rank2,rank2,rank3,rank3.
 *You get the idea...
 *
 **************************************************************************************************/

void mpiEngine::scatterImage(int count,unsigned short *local_memory)
{
    int target, tar_offset;


    // figure out which target or rank we send image.
     target = int(scatgath_counter / num_imgs_per_rank);
    // always scatter to publicimage[0], at top of the window, plus tar_offset based
     // on imgspacing shorts...
     tar_offset = img_spacing_shorts * (scatgath_counter%num_imgs_per_rank);

    putMPIData2( target, count, tar_offset,local_memory);

    //scatgath_counter++;


}



/*****************************************************************************************************
 *
 *countis image size in pixel or shorts. local mem is opinter to img data to scatter
 *
 *Order of scatter, say we have 4 processes, 2 iamges per rank.
 *images will scatter to ranks in this order:
 *rank0,rank0,rank1,rank1,rank2,rank2,rank3,rank3.
 *You get the idea...
 *
 **************************************************************************************************/

void mpiEngine::scatterImage(imageQueueItem *item)
{
    int target, tar_offset;


    // figure out which target or rank we send image.
     target = int(scatgath_counter / num_imgs_per_rank);
    // always scatter to publicimage[0], at top of the window, plus tar_offset based
     // on imgspacing shorts...
     tar_offset = img_spacing_shorts * (scatgath_counter%num_imgs_per_rank);

     int count = item->specs->img_len_shorts;
     // size of image specs in shorts. add extra short incase size gives odd number of bytes


     //put raw img data to publiciage 0
    putMPIData2( target, count, tar_offset,item->img_data);

    //onluy one image will work per rank
    tar_offset = my_message.mpi_image_spec_offset_shorts;
    //put img specs just after img data.
    putMPIData2( target, (int)(imageSpecs::spec_len_short),tar_offset ,(unsigned short*)(item->specs));

    //scatgath_counter++;


}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/


void mpiEngine::fenceMPIData(void)
{

    printTrace("fenceMPIData-A");

//Telll the other processes that the memory is ready to get.
MPI_Win_fence(0, win);
//
    printTrace("fenceMPIData-B");



MPI_Win_fence(0, win);

    printTrace("fenceMPIData-C");

}



void mpiEngine::fenceMPIData2(void)
{

    printTrace("fenceMPIData2-A");

//Telll the other processes that the memory is ready to get.
MPI_Win_fence(0, win);
    printTrace("fenceMPIData2-B");

//
}


/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/



void mpiEngine::hw_info()
{
    /* CPU processor check here */
    system("lscpu");

    system("nproc > procs_num");

    FILE *cpu_info;
    cpu_info = fopen("procs_num", "r");
   if (cpu_info)
    {    fscanf(cpu_info, "%d", &ttlprocs);

    fclose(cpu_info);
}
    /* GPU info checked here */

    /* other platform */
}



  void mpiEngine::clearImages(int rr)
  {
      printTrace("clearImages");

      for (int m=0;m<pub_sh_img_size_bytes;m++)
      {


              public_short_image[rr][m]=0;

      }
  }

  void mpiEngine::clearImages(void)
  {
       printTrace("clearImages");

      for (int m=0;m<pub_sh_img_size_bytes;m++)
      {

          for (int rr=0;rr<numprocs;rr++)
              public_short_image[rr][m]=0;

      }
  }


 /*****************************************************************************************************
  *
  *
  *
  **************************************************************************************************/


 //in private, output piblic
 void mpiEngine::flipEndian(int pix_x, int pix_y,int offset, int count)
 {

     printTrace("flipEndian");


      int endcnt=offset+count;
      int m;
      unsigned short a,b;
      for (m=offset;m<endcnt;m++)
      {

          a=public_short_image[0][m];
          a=a>>8;
          b=public_short_image[0][m];
          b=(b&255)<<8;
          public_short_image[0][m]=a|b;

      }
 }






/*****************************************************************************************************
 *
 *
 *
 **************************************************************************************************/


//runs on rank 0 only.
//runs on own thread blocking on mpi receive.
//waits for messages for rank 1. allows rank1 gui to control all the mpi processes.
 void mpiEngine::waitSentMessages(void)
 {
     guiSignalMessage message;
     int count=sizeof(message.message);
     int fromwho=1;



     bool is_running=true;


     printTrace("waitSentMessages");

     while(is_running)
     {

         printf("Wait for MPI  message %d\n",rank);
         fflush(stdout);

         Recv(&message.message,  count,  fromwho);
         //Bcast(message,count, fromwho);

         printf("Got MPI  message %d\n",rank);
         fflush(stdout);

           //!! deal w/ messages send signals out

         emit signalGuiSettings( message);


  }
 }
 /*****************************************************************************************************
  *
  *
  *
  **************************************************************************************************/

 void mpiEngine::sendMPIGuiSettings(guiSignalMessage mes_)
 {

     //int message[256];
     int count=sizeof(guiMessageFields);
     int towho=0;
     printTrace("sendMPIGuiSettings");

     printf("To send MPI message GUI settings %d\n",rank);
     fflush(stdout);

     Send(&mes_.message,  count,  towho);
      int fromwho=1;
     //Bcast(message,count, fromwho);

     printf("SENT MPI message %d\n",rank);
     fflush(stdout);


 }


 // called by rank 0 only before Bcast to all ranks. makes sure
 // dasta is broadcast to all ranks the same way.
 int mpiEngine::mpiEngineRank0EditMessage(mpiBcastMessage &message)
 {

     if (rank==0)
     {
     message.image_counter_rank0=image_counter_rank0;
     message.num_imgs_per_rank = num_imgs_per_rank;
     }

 }

 //called by all ranks after Bcast, so local class vars match message.
 int mpiEngine::parseMessage(mpiBcastMessage &message)
 {

     my_message=message;



     if (my_message.mpi_is_print_trace)
         is_print_trace=true;
     else
         is_print_trace=false;


     if (my_message.mpi_accum_specs)
     {
         printf("Rank %d got accum specs\n",rank);
         fflush(stdout);



         num_dark_integrate=my_message.num_dark_images_to_accum;
         clearDarks(0);
         dark_integrate_counter=0;
        rank_dark_accum_cnt=0;

     }




     int nimgs = my_message.num_images_to_calc;
     num_images_to_calc=my_message.num_images_to_calc;

     image_counter_rank0=my_message.image_counter_rank0;
     num_imgs_per_rank=my_message.num_imgs_per_rank;


    num_imgs2calc_thisrank=getNumImgs2CalcThisRank(nimgs);

    is_calc_image=getIsCalc(nimgs) && my_message.mpi_image>0;

    num_images_to_acc_this_rank=0;

    num_images_to_acc_this_rank=calcNumDarksAcc(nimgs);


    img_size_x = my_message.imgspecs.size_x;
    img_size_y = my_message.imgspecs.size_y;
    img_num_pixels = my_message.imgspecs.size_pixels;

 }


/*******************************************************************************************
 *
 * We stpre an imagespecs object right after iamge pixel data in the puiblic short images.
 * this is done with scatter(imageQueueItem *). here we return the imagespecs location
 * so we can read it.
 *******************************************************************************************/

 imageSpecs* mpiEngine::getImageSpecs(int whichimg)
{


     int count;//need to calc imgsize/numproc-2.



     //get partial image from rank0. this is the raw img
     //!!count = img_size_x * img_size_y;
     // this number is bcast from scatter...
     // my_message is updated every mpiOneLoop, in parseMessage
     // it will be equal to imageQueueItem::specs->img_len_shorts,
     // which is malloc'ed mem size in shorts for img data.
     //we store in public image the image data, followed by specs
    count = my_message.mpi_image_spec_offset_shorts;

     //we shoudl be sending image specs and placing just after image data.
     //needed for imm comp definiately so we can oput a counter into corecotics in imm header
     image_specs=(imageSpecs*)(public_short_image[whichimg] + count);
     image_specs->processed_by_rank=rank;

     return(image_specs);

 }




 /*****************************************************************************************************
  *
  *
  *
  **************************************************************************************************/

 //input private image, output fordisp public image, and sdark_image
 void mpiEngine::accumDarkStd(int which_sh_img, int which_doub_image)
 {


     //class bariables
     //int num_dark_integrate;
    // int dark_integrate_counter;

      printTrace("enter mpiEngine::accumDarkStd");
     int m;

    // if (dark_integrate_counter==num_dark_integrate)
     //    return;


     double fv,ndi,dki;
     unsigned short sdki;

     if (is_acc_darks)
     {
        printTrace("accumDarkStd- ACCUM");
         ndi=1.0;
         if (num_dark_integrate!=0)
          ndi = 1.0 / ((double)num_dark_integrate);

         for (m=0;m<img_num_pixels;m++)
         {
              fv=(double)(public_short_image[which_sh_img][m]);
             public_double_image[which_doub_image][m]+=fv;
           //  dki=floor(public_double_image[which_doub_image][m]*ndi);
            // sdki=(unsigned short)dki;
            //     public_short_image[which_sh_img][m]=sdki;

         }
        rank_dark_accum_cnt++;

        // dark_integrate_counter++;
    }

 }





 /*****************************************************************************************************
  *
  *
  *
  **************************************************************************************************/

 //input private image, output fordisp public image, and sdark_image
 void mpiEngine::accumDarkStd(double *doubimg, int which_doub_image)
 {
     printTrace("enter mpiEngine::accumDarkStd(doub img) ");


     int m;


     if (is_acc_darks)
     {
        printTrace("mpiEngine::accumDarkStd(doub img)  ACCUM");
         for (m=0;m<img_num_pixels;m++)
         {

              public_double_image[which_doub_image][m]+=doubimg[m];
         }
        // dark_integrate_counter++;
    }

 }

 bool mpiEngine::combineDarkStd(int which_doub_image)
 {
     float ndi;

    ndi=1.0;
     if (num_dark_integrate!=0)
      ndi = 1.0 / ((float)num_dark_integrate);

      combineDarkStd( which_doub_image,ndi);

 }


 bool mpiEngine::combineDarkStd(int which_doub_image,double mult_factor)
 {

     bool is_did_combine = false;

     printTrace("mpiEngine::combineDarkStd");

     if (is_finish_darks)
     {
        //is_finish_darks =false;
        is_did_combine=true;

        printTrace("mpiEngine::combineDarkStd- finishing darks");


           int numpix=img_num_pixels;



           if (rank==1)
           {
              printTrace("Other ranks sending partial sums ");

               MPI_Win_fence(0, windouble);
               //wait for other ranks to put their dark img to rank 1. it accs into the img

               MPI_Win_fence(0, windouble);


           }
           else
           {

               printTrace("Sending partial sum ");


               MPI_Win_fence(0, windouble);
             //  printf("rank %d acc dark to main rank\n",rank);
              // fflush(stdout);

               MPI_Accumulate(
                   public_double_image[which_doub_image],//src
                   pub_db_img_size_bytes/sizeof(double),
                   MPI_DOUBLE,
                   1,//target rank
                   which_doub_image * img_spacing_doubles,//tar offset
                   pub_db_img_size_bytes/sizeof(double),//count of pixels
                   MPI_DOUBLE,
                   MPI_SUM,
                   windouble);
               MPI_Win_fence(0, windouble);
               //now dark has all accums



           }



           //for rank 1, we do 1/N
           if (rank==1)
           {

               printTrace("Calc final var/dark img ");



               for (int m=0;m<numpix;m++)
               {


                  public_double_image[which_doub_image][m]=
                          public_double_image[which_doub_image][m]*mult_factor;

               }


          }

           if (rank==1)
           {
               printTrace("Other ranks getting final dark/std ");


               MPI_Win_fence(0, windouble);
               //wait for other ranks to put their dark img to rank 1. it accs into the img

               MPI_Win_fence(0, windouble);
               //now dark has all accums



           }
           else
           {

               printTrace("getting final dark/std ");

               MPI_Win_fence(0, windouble);
               // copy data from udp process to local


               MPI_Get(
                           public_double_image[which_doub_image],
                           pub_db_img_size_bytes/sizeof(double),
                       MPI_DOUBLE,
                       1,//target get from rank 1
                       which_doub_image*img_spacing_doubles,//tar offset
                           pub_db_img_size_bytes/sizeof(double),
                       MPI_DOUBLE,
                       windouble);

               MPI_Win_fence(0, windouble);

          }



     }
     return(is_did_combine);
 }


 /*****************************************************************************************************
  *
  * clears public_double_image 0. We assume we integrate into pub image 0 for dark accum
  *
  **************************************************************************************************/

 void mpiEngine::clearDarks(int which_doub_image)
  {
         dark_integrate_counter=0;
         printTrace("clearDarks");


         for (int m=0;m<img_num_pixels;m++)
         {
             public_double_image[which_doub_image][m]=0.0;

         }
  }




 //
 // figure out if this rank has to acc a dark img.
 // clears dark images when necessary, keeps track of how many darks are int3egrated over all ranks.
 //

 int mpiEngine::calcNumDarksAcc(int nimgs)

 {


int nimgs_this_rank=num_imgs2calc_thisrank;


     //!! is_finished_darks=false;

     // we may pass only ONE image to ranks. If only one img, then only one rank does anything.
     // we may have up to numprocs images, in which case all ranks do calcs.
     // each rank gets a whole image. The order the imagescome from camera go to
     // rank 0, 1,... etc the earliest image goes to the smallest rank.
     // all proc is inplace. we have a large puiblic window, room for numproc images.
     //each image is pointed to in puiblic_image[m]. public_short_imagep0[ is the biginning
     // of the window. public_short_image[1] is same as public_short_imagep[0] pluse the size of the image.
     //

     //if (nimgs>rank && (message&mpi_b_image)>0)
      //   is_calc_image=true;



   int num_imgs_to_acc=0;

   is_acc_darks=false;
   is_finish_darks=false;

   //when we accum images, we set these vars,
   if (my_message.mpi_accum_darknoise &&
           dark_integrate_counter<num_dark_integrate)
   {
       is_acc_darks=true;
       is_finish_darks=false;
       is_finish_darks_done=false;
   }

   //if we want 10 images, and we have 9 done, accum one more and finish
   if (!is_finish_darks_done && dark_integrate_counter>=num_dark_integrate-1 )
   {
       is_acc_darks=true;
       is_finish_darks=true;
        is_finish_darks_done=true;

   }

   if (!my_message.mpi_accum_darknoise)
   {
       is_acc_darks=false;
       is_finish_darks=false;
   }



     if (is_acc_darks)
     {
         printTrace("calcNumDarksAcc ");

         //in pri out pri



         if (dark_integrate_counter<num_dark_integrate)
         {




             //here is an odd situation. we are acc darks on all ranks. say all ranks get a frame and
             // if we acc on all ranks we get MORE acc images than num_dark_ to integrate. Then we have
             // integrated more than N images. IN this case we could just in increase N my the number of images
             // integrated. The other solution is to not integrate on all  ranks based on this case. We choose
             //the 2nd solution. N_too_many_darks is the number of extra darks we were passed to ranks. If we
             //have 2 ranks, and we are passed one extra image, then N_too_many_darks=1. What we do is tell
             //that many ranks NOT to acc a dark. So if we have 4 ranks, N_too_many_darks=2, and we were passed
             // 4 images, one for each rank, then we only calc on 2 ranks. Then we do not calc too many darks in the
             //accum. If N_too_many_darks=2, and we have 4 ranks, we onlyu calc on rank0, and 1. If we have 2 ranks,
             // and nimgs =2, and N_too_many_darks=1, then only rank0 does calcs.
             // if NOT too may darks, or far from done acc, then N_too_many_darks<0. We onluy have a problem if
             //N_too_many_darks>0

             int N_too_many_darks=dark_integrate_counter + nimgs - num_dark_integrate;
            // if N_too_many_darks>0 we cut back ranks... if 0 or <0 all ranks acc.
             int imgs_2_acc= nimgs - N_too_many_darks;
             // see if we were passed more images than we need to actually acc.

             if (rank < imgs_2_acc)
                 num_imgs_to_acc=nimgs_this_rank;
             else
                 num_imgs_to_acc=0;
   #if 0
           /*  if (N_too_many_darks>0)
             {
                // here we drop off ranks. if nimg=1, nimprocs=2, N_too_many_darks=1 then NO ranks should calc.
                 //if nimgs=2, N_too_many_darks=2, and numprocs =2  then No ranks should calc.
                 // if nimgs=2, N_too_many_darks=1, and numprocs=2 tghen only rank 0 should calc
                 //calc how many ranks should actually compute an image.

                 // if how_many_ranks_acc=0, than no ranks calc.
                 //if how_many_ranks_acc<0, then no ranks calc.
                 // if how_many_ranks_acc=1, then rank 0 will calc.

                 //!! this logic only works with 2 ranks...
                 //!!num_imgs_to_acc=nimgs_this_rank - N_too_many_darks;
                 //!!if (num_imgs_to_acc<0)
                 //!!    num_imgs_to_acc=0;

                    // assume nimgs_this_rank is 1 or 0.
                 if (rank<N_too_many_darks)
                     num_imgs_to_acc=nimgs_this_rank;

                 printTrace("Got too many darks ");


             }
             else
             {
                 num_imgs_to_acc=nimgs_this_rank;
             }*/
#endif

             //becauseopther ranks are acc darks too, all ranks should keep track of how many dars have been accumed.
             dark_integrate_counter+=nimgs;

             if (this->is_print_trace)
             {
                 printf("dark_integrate_counter = %d\n",dark_integrate_counter);
                 fflush(stdout);
             }

         }
     }

     if (num_imgs_to_acc==0)
         is_acc_darks=false;

     num_images_to_acc_this_rank=num_imgs_to_acc;

     char msg[255];
     sprintf(msg,"num_images_to_acc_this_rank %d,\nis_acc_darks %d,\n is_finish_darks %d,\nis_finish_darks_done,\n  num_dark_integrate %d,\ndark_integrate_counter %d \n",
             num_images_to_acc_this_rank,
             is_acc_darks,
             is_finish_darks,
             is_finish_darks_done,
             num_dark_integrate,
             dark_integrate_counter
             );
     printTrace(msg);
     return(num_imgs_to_acc);
 }



 /*****************************************************************************************************
  *
  *
  *
  **************************************************************************************************/


 void mpiEngine::writeImageRaw(void *ptr, int lenbytes,char* fnbase,int fnum)
 {
     FILE *fp;


     printTrace("writeImageRaw");


     char fn[256];
     sprintf(fn,"%s_Rank%d_%03d.bin",fnbase,rank,fnum);
     fp = fopen(fn,"wb");
     if (fp!=0)
     {
     fwrite(ptr, 1,lenbytes,fp);
     fclose(fp);
     }
     else
     {
         printf("Rank %d could not open/wb file %s\n",rank,fn);
         fflush(stdout);
     }
 }


 void mpiEngine::printTrace(char *m)
 {
     if (is_print_trace)
     {
         if (m!=0)
            printf("Trace Rank %d,PID %d, Cnt %d:  %s \n",rank,getpid(), trace_count, m);
         else
            printf("Trace Rank %d,PID %d, Cnt %d:   \n",rank, getpid(),trace_count);

         trace_count++;

         fflush(stdout);
     }

 }
