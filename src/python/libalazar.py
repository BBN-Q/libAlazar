# Python wrapper for the libAlazar shared library.

# Original author: Rob McGurrin
# Copyright 2016-2017 Raytheon BBN Technologies

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
from ctypes import *
from ctypes.util import find_library
import numpy.ctypeslib as npct
import numpy as np

# load the shared library
# try with and without "lib" prefix
libpath = find_library("Alazar")
if libpath is None:
    libpath = find_library("libAlazar")
# if we still can't find it, then try in the python prefix (where conda stores binaries)
if libpath is None:
    libpath = sys.prefix + '/lib'
    lib = npct.load_library("libAlazar", libpath)
else:
    lib = CDLL(libpath)

class AcquisitionParams(Structure):
    _fields_ = [("samplesPerAcquisition", c_uint32),
                ("numberAcquisitions",     c_uint32)]

# _setAll = lib.setAll
# _setAll.argtypes = [c_uint32,POINTER(ConfigData),POINTER(AcquisitionParams)]
# _setAll.restype = c_int32

_Connect = lib.Connect
_Connect.argtypes = [c_uint32]
_Connect.restype = c_int32

_SetMode = lib.SetMode
_SetMode.argtypes = [c_uint32, c_char_p]
_SetMode.restype = c_int32

_SetSampleRate = lib.SetSampleRate
_SetSampleRate.argtypes = [c_uint32, c_uint32]
_SetSampleRate.restype = c_int32

_ConfigureVertical = lib.ConfigureVertical
_ConfigureVertical.argtypes = [c_uint32, c_float, c_float, c_char_p]
_ConfigureVertical.restype = c_int32    
                                       
_SetBandwidth = lib.SetBandwidth
_SetBandwidth.argtypes = [c_uint32, c_char_p]
_SetBandwidth.restype = c_int32

_ConfigureTrigger = lib.ConfigureTrigger
_ConfigureTrigger.argtypes = [c_uint32, c_float, c_char_p, c_char_p, c_char_p, c_float]
_ConfigureTrigger.restype = c_int32    
                         
_ConfigureAcquisition = lib.ConfigureAcquisition
_ConfigureAcquisition.argtypes = [c_uint32, c_uint32, c_uint32, c_uint32, c_uint32, POINTER(AcquisitionParams)]
_ConfigureAcquisition.restype = c_int32    

_disconnect = lib.disconnect
_disconnect.argtypes = [c_uint32]
_disconnect.restype = c_int32

_board_count = lib.boardCount
_board_count.argtypes = []
_board_count.restype = c_uint32

_board_info = lib.boardInfo
_board_info.argtypes = [c_uint32]
_board_info.restype = c_char_p

_stop = lib.stop
_stop.argtypes = [c_uint32]
_stop.restype = c_int32

_acquire = lib.acquire
_acquire.argtypes = [c_uint32]
_acquire.restype = c_int32

_wait_for_acquisition = lib.wait_for_acquisition
_wait_for_acquisition.argtypes = [c_uint32,POINTER(c_float),POINTER(c_float)]
_wait_for_acquisition.restype = c_int32

_force_trigger = lib.force_trigger
_force_trigger.argtypes = [c_uint32]
_force_trigger.restype = c_int32

_register_socket = lib.register_socket
_register_socket.argtypes = [c_uint32, c_uint32, c_int32]
_register_socket.restype = c_int32

_unregister_sockets = lib.unregister_sockets
_unregister_sockets.argtypes = [c_uint32]
_unregister_sockets.restype = c_int32

def enumerate_boards():
    return [_board_info(board_num+1).decode() for board_num in range(_board_count())]

class AlazarError(Exception):
    def __init__(self, msg):
        self.msg = msg
        #todo - log msg

    def __str__(self):
        return self.msg

class ATS9870():

    def __init__(self,logFile='alazar.log',bufferType='raw'):
        self.addr = None
        self.logFile = logFile
        self.bufferType = bufferType

    def connect(self, name):
        self.name = name.split('/')[0]
        self.addr = np.uint32(name.split('/')[1])
        retVal = _Connect(self.addr)   
        if retVal < 0:
            raise AlazarError('ERROR %s: connectBoard failed'%self.name)
        return retVal

    def set_mode(self, acquireMode):
        _SetMode(self.addr, acquireMode.encode())
    
    def set_sample_rate(self, samplingRate):
        _SetSampleRate(self.addr, int(samplingRate))
    
    def configure_vertical(self, verticalScale, verticalOffset, verticalCoupling):
        if verticalScale not in [0.04, 0.1, 0.2, 0.4, 1.0, 2.0, 4.0]:
            raise ValueError("Vertical Scale must be one of 0.04, 0.1, 0.2, 0.4, 1.0, 2.0, 4.0 (volts)")
        _ConfigureVertical(self.addr, verticalScale, verticalOffset, verticalCoupling.encode())
    
    def set_bandwidth(self, bandwidth):
        _SetBandwidth(self.addr, bandwidth.encode())
    
    def configure_trigger(self, triggerLevel, triggerSource, triggerSlope, triggerCoupling, delay):
        _ConfigureTrigger(self.addr, triggerLevel, triggerSource.encode(), triggerSlope.encode(), triggerCoupling.encode(), delay)
    
    def configure_acquisition(self, recordLength, nbrSegments, nbrWaveforms, nbrRoundRobins):
        self.acquisitionParams = AcquisitionParams()
        _ConfigureAcquisition(self.addr, recordLength, nbrSegments,
            nbrWaveforms, nbrRoundRobins, byref(self.acquisitionParams))
        self.numberAcquisitions     = self.acquisitionParams.numberAcquisitions
        self.samplesPerAcquisition = self.acquisitionParams.samplesPerAcquisition

        if not hasattr(self, 'ch1Buffer'):
            self.ch1Buffer = np.empty(self.samplesPerAcquisition,dtype=np.float32)
        elif len(self.ch1Buffer) != self.samplesPerAcquisition:
            self.ch1Buffer = np.empty(self.samplesPerAcquisition,dtype=np.float32)
        if not hasattr(self, 'ch2Buffer'):
            self.ch2Buffer = np.empty(self.samplesPerAcquisition,dtype=np.float32)
        elif len(self.ch2Buffer) != self.samplesPerAcquisition:
            self.ch2Buffer = np.empty(self.samplesPerAcquisition,dtype=np.float32)

        self.ch1Buffer_p = self.ch1Buffer.ctypes.data_as(POINTER(c_float))
        self.ch2Buffer_p = self.ch2Buffer.ctypes.data_as(POINTER(c_float))

    def acquire(self):
        # self.configureBoard()
        if not hasattr(self.acquisition_params):
            raise AlazarError("ERROR %s: acquisition params have not been set. Run configure_acquisition."%self.name)
        retVal = _acquire(self.addr)
        if retVal < 0:
            raise AlazarError('ERROR %s: acquire failed'%self.name)

    def data_available(self):
        status = _wait_for_acquisition(self.addr, self.ch1Buffer_p, self.ch2Buffer_p)
        if status < 0:
            raise AlazarError('ERROR %s: data_available failed' % self.name)
        return status

    def stop(self):
        # Don't bother if we've never connected
        if self.addr is not None:
            retVal = _stop(self.addr)
            if retVal < 0:
                raise AlazarError('ERROR %s: stop failed' % self.name)

    def disconnect(self):
        retVal = _disconnect(self.addr)
        if retVal < 0:
            raise AlazarError('ERROR %s: disconnect failed'%self.name)

    def trigger(self):
        retVal = _force_trigger(self.addr)
        if retVal < 0:
            raise AlazarError('ERROR %s: trigger failed' % self.name)

    def register_socket(self, channel, socket):
        retVal = _register_socket(self.addr, channel, socket.fileno())
        if retVal < 0:
            raise AlazarError('ERROR %s: register_socket failed' % self.name)

    def unregister_sockets(self):
        retVal = _unregister_sockets(self.addr)
        if retVal < 0:
            raise AlazarError('ERROR %s: unregister_sockets failed' % self.name)

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
