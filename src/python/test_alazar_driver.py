import sys
import os
import time
import unittest
import numpy as np

from libalazar import ATS9870,AlazarError

class AlazarDriverTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        pass

    @classmethod
    def tearDownClass(self):
        pass        

class TestAPI(AlazarDriverTest):
    
    def setUp(self):
        pass
        
    def tearDown(self):
        pass
    
    def verify_local_attribute(self,dlocal,param,value):
        setattr(dlocal,param,value)
        self.assertEqual(getattr(dlocal,param),value)   
        

    def test_local_config(self):
        #run a local digitizer
        dlocal=ATS9870('foo/1')
        dlocal.connectBoard()

        #test using values that are different than the defaults
        self.verify_local_attribute(dlocal,'acquireMode','digitizer')
        self.verify_local_attribute(dlocal,'bandwidth','Half')
        self.verify_local_attribute(dlocal,'clockType','ext')
        self.verify_local_attribute(dlocal,'delay',1e6)
        self.verify_local_attribute(dlocal,'enabled',0)
        self.verify_local_attribute(dlocal,'label','foo')
        self.verify_local_attribute(dlocal,'recordLength',256)
        self.verify_local_attribute(dlocal,'nbrSegments',9)
        self.verify_local_attribute(dlocal,'nbrWaveforms',10)
        self.verify_local_attribute(dlocal,'nbrRoundRobins',2456)
        self.verify_local_attribute(dlocal,'samplingRate',100e6)
        self.verify_local_attribute(dlocal,'triggerCoupling','DC')
        self.verify_local_attribute(dlocal,'triggerLevel',3000)
        self.verify_local_attribute(dlocal,'triggerSlope','falling')
        self.verify_local_attribute(dlocal,'triggerSource','Int')
        self.verify_local_attribute(dlocal,'verticalCoupling','DC')
        self.verify_local_attribute(dlocal,'verticalOffset',10.0)
        self.verify_local_attribute(dlocal,'verticalScale',3.0)
        self.verify_local_attribute(dlocal,'bufferSize',32768)
    
        dlocal.disconnect()

    def test_local_api(self):

        dlocal=ATS9870('foo/1')
        dlocal.connectBoard()
        dlocal.setAll()
        dlocal.acquire()
        dlocal.waitForAcquisition()
        dlocal.disconnect()


class TestLib(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.config={
            'acquireMode':'averager',
            'bandwidth':'Full',
            'clockType':'ref',
            'delay':0.01,
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
            'bufferSize':8192,
        }
        self.ats9870=ATS9870('foo/1')
        
        
    def connect(self,logFile):
        ret =self.ats9870.connectBoard()
        
        
    @classmethod
    def tearDownClass(self):
        pass
        
    def parseLog(self,keyword):
        return 0
        #todo - need to get this working
        try:
            log = open(self.logFile, 'r')
        except:
            self.fail('Can\'t open log')
            return(-1)
            
        for n,line in enumerate(log):
            if keyword in line:
                log.close()
                return(line.split()[-1])

        log.close()
                
    def checkLog(self,keyword,value,myType):
        return 0
        #todo - need to get this working
        test = self.parseLog(keyword)
        self.assertEqual(value,myType(test))
        
    def compareData(self):
        
        ch1 = np.array([],dtype=np.float32)
        ch2 = np.array([],dtype=np.float32)
        
        #wait for the acquisition to be complete
        for count in range(self.ats9870.numberAcquistions):
            while not self.ats9870.waitForAcquisition():
                time.sleep(.0001)
            ch1=np.append(ch1,self.ats9870.ch1Buffer)
            ch2=np.append(ch2,self.ats9870.ch2Buffer)

        #generate the test pattern to match
        t1,t2 = self.ats9870.generateTestPattern()

        maxErrorCh1 = np.max(ch1 - t1.T.flat)
        self.assertEqual(maxErrorCh1,0.0)
        
        maxErrorCh2 = np.max(ch2 - t2.T.flat)
        self.assertEqual(maxErrorCh2,0.0)

        return

    def initConfig(self):
        #reset the config params
        self.ats9870.config={
            'acquireMode':'averager',
            'bandwidth':'Full',
            'clockType':'ref',
            'delay':0.01,
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
            'bufferSize':8192,
        }

    def setUp(self):
        self.initConfig()
        
    def tearDown(self):
        self.ats9870.disconnect()

    def test_config(self):
        self.connect(self.test_config.__name__+'.log')
        self.ats9870.setAll()
        
    
    def test_single_record_too_large(self):
        self.connect(self.test_single_record_too_large.__name__+'.log')
        self.ats9870.config['bufferSize'] = 4096
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()

    def test_max_buffer_size_exceeded(self):
        self.connect(self.test_max_buffer_size_exceeded.__name__+'.log')
        self.ats9870.config['bufferSize'] = 2**23
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_record_length_alignment(self):
        self.connect(self.test_record_length_alignment.__name__+'.log')
        self.ats9870.config['recordLength'] = 257
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_record_length_min(self):
        self.connect(self.test_record_length_min.__name__+'.log')
        self.ats9870.config['recordLength'] = 255
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_one_round_robin_per_buffer(self):
        logFile = self.test_one_round_robin_per_buffer.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['recordLength'] = 1024
        self.ats9870.config['nbrWaveforms'] = 4
        self.ats9870.config['bufferSize'] = 8192
        self.ats9870.setAll()
        self.checkLog('roundRobinsPerBuffer',1,int)
        self.ats9870.disconnect()

    def test_multiple_round_robins_per_buffer(self):
        logFile = self.test_multiple_round_robins_per_buffer.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['recordLength'] = 256
        self.ats9870.config['nbrWaveforms'] = 4
        self.ats9870.config['nbrRoundRobins'] = 32
        self.ats9870.config['bufferSize'] = 8192
        self.ats9870.setAll()
        self.checkLog('roundRobinsPerBuffer',4,int)
        
    def test_multiple_buffers_per_round_robin(self):
        logFile = self.test_multiple_buffers_per_round_robin.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['recordLength'] = 4096
        self.ats9870.config['nbrWaveforms'] = 3
        self.ats9870.config['nbrRoundRobins'] = 3
        self.ats9870.config['bufferSize'] = 8192
        self.ats9870.setAll()
        self.checkLog('buffersPerRoundRobin',3,int)
        
    def test_mode_config(self):
        logFile = self.test_mode_config.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['acquireMode'] = 'avverager'
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        self.initConfig()

    def test_channel_coupling(self):
        logFile = self.test_channel_coupling.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['verticalCoupling'] = 'ac'
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()

    def test_trigger_coupling(self):
        logFile = self.test_trigger_coupling.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['triggerCoupling'] = 'ac'
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_trigger_source(self):
        logFile = self.test_trigger_source.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['triggerSource'] = 'ext'
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_trigger_slope(self):
        logFile = self.test_trigger_slope.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['triggerSource'] = 'rising'
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_trigger_bandwidth(self):
        logFile = self.test_trigger_bandwidth.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['bandwidth'] = ''
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
    def test_channel_scale(self):
        logFile = self.test_channel_scale.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['verticalScale'] = 99.0
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
        self.ats9870.config['verticalScale'] = 2.0
        self.ats9870.setAll()
        
    def test_sample_rate(self):
        logFile = self.test_sample_rate.__name__+'.log'
        self.connect(logFile)
        self.ats9870.config['samplingRate'] = 59e6
        with self.assertRaises(AlazarError):
            self.ats9870.setAll()
        
        self.ats9870.config['samplingRate'] = 500e6
        self.ats9870.setAll()

    def test_digitizer(self):        
        logFile = self.test_digitizer.__name__+'.log'
        
        self.connect(logFile)
        
        self.ats9870.config['acquireMode']      = 'digitizer'
        self.ats9870.config['recordLength']     = 1024
        self.ats9870.config['nbrWaveforms']     = 3
        self.ats9870.config['nbrSegments']      = 5
        self.ats9870.config['nbrRoundRobins']   = 3
        
        #configure for using exactly 3 buffers
        self.ats9870.config['bufferSize']       = 1024*2*3*5
        
        self.ats9870.setAll()
        
        self.ats9870.acquire()
        
        self.compareData()
        self.ats9870.disconnect()
          
    def test_averager(self):        
        logFile = self.test_averager.__name__+'.log'
        
        self.connect(logFile)
        
        self.ats9870.config['acquireMode']      = 'averager'
        self.ats9870.config['recordLength']     = 1024
        self.ats9870.config['nbrWaveforms']     = 5
        self.ats9870.config['nbrSegments']      = 3
        
        #pattern generator currently only supports 1 round robin
        self.ats9870.config['nbrRoundRobins']   = 1
        
        #configure for using exactly 3 buffers
        self.ats9870.config['bufferSize']       = 1024*5*3*3*2
        
        self.ats9870.setAll()
        
        self.ats9870.acquire()
        
        self.compareData()
        self.ats9870.disconnect()
        
    def test_partial_buffer_digitizer(self):   
        logFile = self.test_partial_buffer_digitizer.__name__+'.log'
        
        self.connect(logFile)
        
        self.ats9870.config['acquireMode']      = 'digitizer'
        self.ats9870.config['recordLength']     = 1024
        self.ats9870.config['nbrWaveforms']     = 7
        self.ats9870.config['nbrSegments']      = 9
        self.ats9870.config['nbrRoundRobins']   = 1
        
        #configure for 21 buffers per round robin
        self.ats9870.config['bufferSize']       = 1024*2*3
        
        self.ats9870.setAll()
        
        self.ats9870.acquire()
        
        self.compareData()
        
        
    def test_partial_buffer_averager(self):   
        logFile = self.test_partial_buffer_averager.__name__+'.log'
        
        self.connect(logFile)
        
        self.ats9870.config['acquireMode']      = 'averager'
        self.ats9870.config['recordLength']     = 1024
        self.ats9870.config['nbrWaveforms']     = 7
        self.ats9870.config['nbrSegments']      = 9
        self.ats9870.config['nbrRoundRobins']   = 1
        
        #configure for 21 buffers per round robin
        self.ats9870.config['bufferSize']       = 1024*2*3
        
        self.ats9870.setAll()
        
        self.ats9870.acquire()
        
        self.compareData()

        
if __name__ == '__main__':
    unittest.main(verbosity=True)
