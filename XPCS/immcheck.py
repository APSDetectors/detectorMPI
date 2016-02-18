
"""

#How to use
#run ipython

execfile('immcheck.py')

iname = '/local/testa_00200-01199.imm'
checkFile(iname)

#to read headers of images

fp = open(iname,'r')

h = readHeader(fp)
print h
#call if you are skipping iamge data, and not reading iamge data.
getNextHeaderPos(fp,h)

h = readHeader(fp)
print h
getNextHeaderPos(fp,h)


clf()
h = readHeader(fp)
img = getImage(fp,h)
#do not call getNextHeader. getImage seeks to next header already
print h
#displauy image
figimage(img)


clf()
h = readHeader(fp)
img = getImage(fp,h)
#do not call getNextHeader. getImage seeks to next header already
print h
#displauy image
figimage(img)



#etc ..\
#raw IMM

iname = '/local/testyraw_00100-00199.imm'

checkFile(iname)

fp.close()

"""




import struct


#comment these ouit of not drawing images, but only reading headers"
import numpy as np
import matplotlib.pyplot as plt



imm_headformat = "ii32s16si16siiiiiiiiiiiiiddiiIiiI40sf40sf40sf40sf40sf40sf40sf40sf40sf40sfffiiifc295s84s12s"

imm_fieldnames = [
'mode',
'compression',
'date',
'prefix',
'number',
'suffix',
'monitor',
'shutter',
'row_beg',
'row_end',
'col_beg',
'col_end',
'row_bin',
'col_bin',
'rows',
'cols',
'bytes',
'kinetics',
'kinwinsize',
'elapsed',
'preset',
'topup',
'inject',
'dlen',
'roi_number',
'buffer_number',
'systick',
'pv1',
'pv1VAL',
'pv2',
'pv2VAL',
'pv3',
'pv3VAL',
'pv4',
'pv4VAL',
'pv5',
'pv5VAL',
'pv6',
'pv6VAL',
'pv7',
'pv7VAL',
'pv8',
'pv8VAL',
'pv9',
'pv9VAL',
'pv10',
'pv10VAL',
'imageserver',
'CPUspeed',
'immversion',
'corecotick',
'cameratype',
'threshhold',
'byte632',
'empty_space',
'ZZZZ',
'FFFF'

]




iname = '/local/testa_00200-01199.imm'

def checkFile(fname):
    fp = open(fname,'rb')

    lastcor=-1
    lastbn = -1;
    n_corerror = 0
    n_bnerror = 0

    while True:
        h = readHeader(fp)
        if h!='eof':
            print 'buffer number %d'%h['buffer_number']
            print 'corecotick %d'%h['corecotick']

            if lastbn==-1: lastbn = h['buffer_number']-1
            if lastcor==-1: lastcor = h['corecotick']-1

            dbn = h['buffer_number'] - lastbn
            dcor = h['corecotick'] - lastcor

            if dbn>1: n_bnerror=n_bnerror+1

            if dcor>1: n_corerror = n_corerror+1


            lastbn = h['buffer_number']
            lastcor = h['corecotick']

            getNextHeaderPos(fp,h)
        else: break

    print "Skipped Buffer numbers %d"%n_bnerror
    print "Skipped Corecoticks %d"%n_corerror

    fp.close()

def readHeader(fp):
    bindata = fp.read(1024)
    if bindata=='':
        return('eof')

    imm_headerdat = struct.unpack(imm_headformat,bindata)
    imm_header ={}
    for k in range(len(imm_headerdat)):
        imm_header[imm_fieldnames[k]]=imm_headerdat[k]
    return(imm_header)


def getNextHeaderPos(fp,header):
    dlen = header['dlen']
    if header['compression']==6:
        fp.seek(dlen*6,1)
    else:
        fp.seek(dlen*2,1)




#getImage requres numpy, comment out of no numpy


def getImage(fp,h):
    dlen = h['dlen']

    if h['compression']==6:
        loc_b = fp.read(4*dlen)
        pixloc = struct.unpack('%di'%dlen,loc_b)

        val_b = fp.read(2*dlen)
        pixval = struct.unpack('%dH'%dlen,val_b)

        imgdata = np.array( [0] * (h['rows'] * h['cols']))

        for k in range(dlen):
            imgdata[ pixloc[k] ] = pixval[k]

    else:
        pixdat=fp.read(2*dlen)
        pixvals=struct.unpack('%dH'%dlen,pixdat)
        imgdata=np.array(pixvals)



    imgdata = imgdata.reshape(h['rows'], h['cols'])

    return(imgdata)




