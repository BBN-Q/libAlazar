#!/usr/bin/env python
import numpy as np
import argparse
from ctypes import *
import ctypes.util
import time
import platform

class LibAlazar():

    class ConfigData(Structure):
        _fields_ = [
                    ("acquireMode",     c_char_p),
                    ("bandwidth",       c_char_p),
                    ("clockType",       c_char_p),
                    ("delay",           c_double),
                    ("enabled",         c_bool),
                    ("label",           c_char_p),
                    ("recordLength",    c_uint32),
                    ("nbrSegments",     c_uint32),
                    ("nbrWaveforms",    c_uint32),
                    ("nbrRoundRobins",  c_uint32),
                    ("samplingRate",    c_double),
                    ("triggerCoupling", c_char_p),
                    ("triggerLevel",    c_double),
                    ("triggerSlope",    c_char_p),
                    ("triggerSource",   c_char_p),
                    ("verticalCoupling",c_char_p),
                    ("verticalOffset",  c_double),
                    ("verticalScale",   c_double),
                    ("bufferSize",      c_uint32),

                   ]

    class AcquisitionParams(Structure):
        _fields_ = [("samplesPerAcquisition", c_uint32),
                    ("numberAcquistions",     c_uint32)]

    def __init__(self):

        osType = platform.system()
        if 'Darwin' in osType:
            self.lib = CDLL('../../build/bin/libAlazar.dylib')
        elif 'Windows' in osType:
            self.lib = CDLL('../../build/bin/libAlazar.dll')
        else:
            self.lib = CDLL('../../build/bin/libAlazar.so')
            

        self._connectBoard = self.lib.connectBoard
        self._connectBoard.argtypes = [c_uint32,c_char_p]
        self._connectBoard.restype = c_int32

        self._setAll = self.lib.setAll
        self._setAll.argtypes = [c_uint32,POINTER(self.ConfigData),POINTER(self.AcquisitionParams)]
        self._setAll.restype = c_int32

        self._disconnect = self.lib.disconnect
        self._disconnect.argtypes = [c_uint32]
        self._disconnect.restype = c_int32

        self._stop = self.lib.stop
        self._stop.argtypes = [c_uint32]
        self._stop.restype = c_int32

        self._acquire = self.lib.acquire
        self._acquire.argtypes = [c_uint32]
        self._acquire.restype = c_int32

        self._wait_for_acquisition = self.lib.wait_for_acquisition
        self._wait_for_acquisition.argtypes = [c_uint32,POINTER(c_float),POINTER(c_float)]
        self._wait_for_acquisition.restype = c_int32
        

    def connectBoard(self,boardId,logFile):
        self.boardId = boardId
        ret = self._connectBoard(boardId,logFile.encode('ascii'));
        return(ret)

    def setAll(self,config):

        self.configData = self.ConfigData()
        fieldNames = [ name for name, ftype in self.ConfigData._fields_]

        for k in config.keys():
            if k in fieldNames:
                value = config[k] 
                if isinstance(value,str):
                    value = value.encode('ascii')
                setattr(self.configData,k,value)
            else:
                #todo log error
                print('ERROR: %s is not in the config data'%k)
        
        self.config = config
        
        self.acquisitionParams = self.AcquisitionParams()

        retVal = self._setAll(self.boardId,byref(self.configData),byref(self.acquisitionParams))
        if retVal < 0:
            return(retVal)

        self.numberAcquistions     = self.acquisitionParams.numberAcquistions
        self.samplesPerAcquisition = self.acquisitionParams.samplesPerAcquisition

        self.ch1Buffer   = np.zeros(self.samplesPerAcquisition,dtype=np.float32)
        self.ch1Buffer_p = self.ch1Buffer.ctypes.data_as(POINTER(c_float))

        self.ch2Buffer   = np.zeros(self.samplesPerAcquisition,dtype=np.float32)
        self.ch2Buffer_p = self.ch2Buffer.ctypes.data_as(POINTER(c_float))

        return 0

    def disconnect(self):
        retVal = self._disconnect(self.boardId)
        return retVal

    def stop(self):
        retVal = self._stop(self.boardId)
        return retVal

    def acquire(self):
        retVal = self._acquire(self.boardId)
        return retVal

    def wait_for_acquisition(self):
        retVal = self._wait_for_acquisition(self.boardId,self.ch1Buffer_p, self.ch2Buffer_p)
        return retVal
        
    def generateTestPattern(self):
            
            #todo - this only will generate 1 round robin in averaging mode when using partial
            # buffers - 
                        
            numRecords = self.config['nbrWaveforms']*self.config['nbrSegments']*self.config['nbrRoundRobins']
            ch1 = np.zeros((self.config['recordLength'],numRecords),dtype=np.float32)
            ch2 = np.zeros((self.config['recordLength'],numRecords),dtype=np.float32)
            for r in range(numRecords):
                #apply the scaling and offset - recreate the pattern from alazarSim.c
                ch1[:,r] = (np.mod(r,256) - 128.)*self.config['verticalScale']/128.
                ch2[:,r] = (np.mod(r+1,256) - 128.)*self.config['verticalScale']/128.
                
            #average the data if ebabled
            if self.config['acquireMode'] == 'averager':
                newShape = (self.config['recordLength'],self.config['nbrWaveforms'],self.config['nbrSegments'],self.config['nbrRoundRobins'])
                ch1 = np.reshape(ch1,newShape,order='F')
                ch2 = np.reshape(ch2,newShape,order='F')
                                
                #average across waveforms and round robins
                ch1=np.average(ch1,axis=1)
                ch1=np.average(ch1,axis=2)
                
                ch2=np.average(ch2,axis=1)
                ch2=np.average(ch2,axis=2)                    
                    
            return ch1,ch2
                
            
            
            
            


def main():

        """
        Parse the input arguments into arg
        """
        parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
        parser.add_argument('--log', help='log file none - defaults to stdout',default=None)
        parser.add_argument('--acquireMode', help='digitizer or averager',default='averager')
        parser.add_argument('--bandwidth', help='Full or 20MHz',default='Full')
        parser.add_argument('--delay', help='trigger delay',type=float,default=0.0)
        parser.add_argument('--recordLength', help='record length',type=int,default=4096)
        parser.add_argument('--nbrSegments', help='number of segments',type=int,default=1)
        parser.add_argument('--nbrWaveforms', help='number of waveforms',type=int,default=1)
        parser.add_argument('--nbrRoundRobins', help='number of round robins',type=int,default=1)
        parser.add_argument('--samplingRate', help='number of round robins',type=float,default=500e6)
        parser.add_argument('--triggerCoupling', help='trigger coupling AC or DC',default='AC')
        parser.add_argument('--triggerLevel', help='trigger level in mV',type=int,default=1000)
        parser.add_argument('--triggerSlope', help='rising or falling',default='rising')
        parser.add_argument('--triggerSource', help='A, B, or Ext',default='Ext')
        parser.add_argument('--verticalCoupling', help='channel coupling AC or DC',default='AC')
        parser.add_argument('--verticalOffset', help='channel offset in volts',type=float,default=0.0)
        parser.add_argument('--verticalScale', help='channel input range, choose from [.04,.1,.2,.4,1.0,2.0,4.0]',type=float,default=1.0)
        parser.add_argument('--bufferSize', help='defaults to 81920',type=int,default=81920)
        parser.add_argument('--addr', help='board address',type=int,default=1)
        parser.add_argument('--plot', help=' enable plot', dest='plot', action='store_true',default=False)
        args=parser.parse_args()

        #todo - check return values
        #todo - validate config parameters

        config={
            'acquireMode':args.acquireMode,
            'bandwidth':args.bandwidth,
            'clockType':'ref',
            'delay':args.delay,
            'enabled':True,
            'label':'Alazar',
            'recordLength':args.recordLength,
            'nbrSegments':args.nbrSegments,
            'nbrWaveforms':args.nbrWaveforms,
            'nbrRoundRobins':args.nbrRoundRobins,
            'samplingRate':args.samplingRate,
            'triggerCoupling':args.triggerCoupling,
            'triggerLevel':args.triggerLevel,
            'triggerSlope':args.triggerSlope,
            'triggerSource':args.triggerSource,
            'verticalCoupling':args.verticalCoupling,
            'verticalOffset':args.verticalOffset,
            'verticalScale':args.verticalScale,
            'bufferSize':args.bufferSize,
        }

        alazar=LibAlazar()
        if alazar.connectBoard(args.addr,args.log) < 0:
            exit(-1)
            
        if alazar.setAll(config) < 0:
            exit(-1)
            
            
        t1,t2 = alazar.generateTestPattern()
        alazar.acquire()

        
        ch1 = np.array([],dtype=np.float32)
        ch2 = np.array([],dtype=np.float32)
        
        for count in range(alazar.numberAcquistions):
            while not alazar.wait_for_acquisition():
                time.sleep(.0001)
            ch1=np.append(ch1,alazar.ch1Buffer)
            ch2=np.append(ch2,alazar.ch2Buffer)

        alazar.disconnect()
        alazar.stop()

if __name__ == "__main__":
    main()
