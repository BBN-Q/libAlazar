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
               ]

class AcquisitionParams(Structure):
    _fields_ = [("samplesPerAcquisition", c_uint32),
                ("numberAcquisitions",     c_uint32)]

_connectBoard = lib.connectBoard
_connectBoard.argtypes = [c_uint32,c_char_p]
_connectBoard.restype = c_int32

_setAll = lib.setAll
_setAll.argtypes = [c_uint32,POINTER(ConfigData),POINTER(AcquisitionParams)]
_setAll.restype = c_int32

_disconnect = lib.disconnect
_disconnect.argtypes = [c_uint32]
_disconnect.restype = c_int32

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

class AlazarError(Exception):
    def __init__(self, msg):
        self.msg = msg
        #todo - log msg

    def __str__(self):
        return self.msg

class ATS9870():

    def __init__(self,logFile='alazar.log',bufferType='raw'):

        self.addr = None

        self.config={
            'acquireMode':'averager',
            'bandwidth':'Full',
            'clockType':'ref',
            'delay':0.0,
            'enabled':True,
            'label':'Alazar',
            'recordLength':4096,
            'nbrSegments':1,
            'nbrWaveforms':1,
            'nbrRoundRobins':1,
            'samplingRate':500e6,
            'triggerCoupling':'AC',
            'triggerLevel':1000,
            'triggerSlope':'rising',
            'triggerSource':'Ext',
            'verticalCoupling':'AC',
            'verticalOffset':0.0,
            'verticalScale':1.0,
        }

        self.logFile = logFile
        self.bufferType = bufferType

    def connect(self, name):
        self.name = name.split('/')[0]
        self.addr = np.uint32(name.split('/')[1])

        retVal = _connectBoard(self.addr,self.logFile.encode('ascii'));
        if retVal < 0:
            raise AlazarError('ERROR %s: connectBoard failed'%self.name)
        return retVal

    #todo - anyway to automate creating the get/set methods?
    def set_acqireMode(self,value):
        self.writeConfig('acquireMode',value)
    def get_acquireMode(self):
        return self.readConfig('acquireMode')
    acquireMode = property(get_acquireMode, set_acqireMode)

    def set_bandwidth(self,value):
        self.writeConfig('bandwidth',value)
    def get_bandwidth(self):
        return self.readConfig('bandwidth')
    bandwidth = property(get_bandwidth, set_bandwidth)

    def set_clockType(self,value):
        self.writeConfig('clockType',value)
    def get_clockType(self):
        return self.readConfig('clockType')
    clockType = property(get_clockType,set_clockType)

    def set_delay(self,value):
        self.writeConfig('delay',value)
    def get_delay(self):
        return self.readConfig('delay')
    delay = property(get_delay, set_delay)

    def set_enabled(self,value):
        self.writeConfig('enabled',value)
    def get_enabled(self):
        return self.readConfig('enabled')
    enabled = property(get_enabled, set_enabled)

    def set_label(self,value):
        self.writeConfig('label',value)
    def get_label(self):
        return self.readConfig('label')
    label = property(get_label, set_label)

    def set_recordLength(self,value):
        self.writeConfig('recordLength',value)
    def get_recordLength(self):
        return self.readConfig('recordLength')
    recordLength = property(get_recordLength, set_recordLength)

    def set_nbrSegments(self,value):
        self.writeConfig('nbrSegments',value)
    def get_nbrSegments(self):
        return self.readConfig('nbrSegments')
    nbrSegments = property(get_nbrSegments, set_nbrSegments)

    def set_nbrWaveforms(self,value):
        self.writeConfig('nbrWaveforms',value)
    def get_nbrWaveforms(self):
        return self.readConfig('nbrWaveforms')
    nbrWaveforms = property(get_nbrWaveforms, set_nbrWaveforms)

    def set_nbrRoundRobins(self,value):
        self.writeConfig('nbrRoundRobins',value)
    def get_nbrRoundRobins(self):
        return self.readConfig('nbrRoundRobins')
    nbrRoundRobins = property(get_nbrRoundRobins, set_nbrRoundRobins)

    def set_samplingRate(self,value):
        self.writeConfig('samplingRate',value)
    def get_samplingRate(self):
        return self.readConfig('samplingRate')
    samplingRate = property(get_samplingRate, set_samplingRate)

    def set_triggerCoupling(self,value):
        self.writeConfig('triggerCoupling',value)
    def get_triggerCoupling(self):
        return self.readConfig('triggerCoupling')
    triggerCoupling = property(get_triggerCoupling, set_triggerCoupling)

    def set_triggerLevel(self,value):
        self.writeConfig('triggerLevel',value)
    def get_triggerLevel(self):
        return self.readConfig('triggerLevel')
    triggerLevel = property(get_triggerLevel, set_triggerLevel)

    def set_triggerSlope(self,value):
        self.writeConfig('triggerSlope',value)
    def get_triggerSlope(self):
        return self.readConfig('triggerSlope')
    triggerSlope = property(get_triggerSlope, set_triggerSlope)

    def set_triggerSource(self,value):
        self.writeConfig('triggerSource',value)
    def get_triggerSource(self):
        return self.readConfig('triggerSource')
    triggerSource = property(get_triggerSource, set_triggerSource )

    def set_verticalCoupling(self,value):
        self.writeConfig('verticalCoupling',value)
    def get_verticalCoupling(self):
        return self.readConfig('verticalCoupling')
    verticalCoupling = property(get_verticalCoupling, set_verticalCoupling )

    def set_verticalOffset(self,value):
        self.writeConfig('verticalOffset',value)
    def get_verticalOffset(self):
        return self.readConfig('verticalOffset')
    verticalOffset = property(get_verticalOffset, set_verticalOffset )

    def set_verticalScale(self,value):
        self.writeConfig('verticalScale',value)
    def get_verticalScale(self):
        return self.readConfig('verticalScale')
    verticalScale = property(get_verticalScale, set_verticalScale )

    def set_bufferSize(self,value):
        self.writeConfig('bufferSize',value)
    def get_bufferSize(self):
        return self.readConfig('bufferSize')
    bufferSize = property(get_bufferSize, set_bufferSize )

    def writeConfig(self,param,value):
        self.config[param] = value

    def readConfig(self,param):
        return self.config[param]

    def setAll(self, config):

        for param in self.config.keys():
            if param not in config.keys():
                raise AlazarError('ERROR: config is missing %s'%param)

        for param in config.keys():
            if param in self.config.keys():
                self.writeConfig(param,config[param])
            else:
                raise AlazarError('ERROR: %s is not a config parameter'%param)

        self.configureBoard()

    def configureBoard(self):
        self.configData = ConfigData()
        fieldNames = [ name for name, ftype in ConfigData._fields_]

        for field in fieldNames:
            value = getattr(self,field)
            if isinstance(value,str):
                value = value.encode('ascii')
            setattr(self.configData,field,value)

        self.acquisitionParams = AcquisitionParams()

        retVal = _setAll(self.addr,byref(self.configData),byref(self.acquisitionParams))
        if retVal < 0:
            raise AlazarError('ERROR %s: setAll failed'%self.name)

        self.numberAcquisitions     = self.acquisitionParams.numberAcquisitions
        self.samplesPerAcquisition = self.acquisitionParams.samplesPerAcquisition

        self.ch1Buffer   = np.zeros(self.samplesPerAcquisition,dtype=np.float32)
        self.ch1Buffer_p = self.ch1Buffer.ctypes.data_as(POINTER(c_float))

        self.ch2Buffer   = np.zeros(self.samplesPerAcquisition,dtype=np.float32)
        self.ch2Buffer_p = self.ch2Buffer.ctypes.data_as(POINTER(c_float))

    def acquire(self):
        self.configureBoard()
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
