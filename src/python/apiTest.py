#!/usr/bin/env python
import unittest
import libAlazar 
import os
import numpy as np
import time
import matplotlib.pyplot as plt

class MyUnitTest(unittest.TestCase):
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
        self.lib=libAlazar.LibAlazar()
        
        
    def connect(self,logFile):
        self.logFile = logFile
        if os.path.isfile(self.logFile):
            os.remove(self.logFile)
        ret =self.lib.connectBoard(self.logFile)
        
        
    @classmethod
    def tearDownClass(self):
        pass
        
    def parseLog(self,keyword):
        try:
            log = open(self.logFile, 'r')
        except:
            self.fail('Can\'t open log')
            return(-1)
            
        for n,line in enumerate(log):
            if keyword in line:
                return(line.split()[-1])
                #print(keyword,line.split()[-1])
                
    def checkLog(self,keyword,value,myType):
        
        test = self.parseLog(keyword)
        self.assertEqual(value,myType(test))
        
    def compareData(self):
        
        ch1 = np.array([],dtype=np.float32)
        ch2 = np.array([],dtype=np.float32)
        
        #wait for the acquisition to be complete
        for count in range(self.lib.numberAcquistions):
            while not self.lib.wait_for_acquisition():
                time.sleep(.0001)
            ch1=np.append(ch1,self.lib.ch1Buffer)
            ch2=np.append(ch2,self.lib.ch2Buffer)
        
        '''           
        plt.figure()
        plt.plot(ch1)
        plt.plot(ch2)
        plt.title('Diff')
        plt.show()
        '''
        
        
        #generate the test pattern to match
        t1,t2 = self.lib.generateTestPattern()
        
        '''
        plt.figure()
        plt.plot(t1.T.flat)
        plt.plot(t2.T.flat)
        plt.title('Diff')
        plt.show()
        '''
            
        maxErrorCh1 = np.max(ch1 - t1.T.flat)
        self.assertEqual(maxErrorCh1,0.0)
        
        maxErrorCh2 = np.max(ch2 - t2.T.flat)
        self.assertEqual(maxErrorCh2,0.0)

        return
    
class TestAPI(MyUnitTest):
        
    def initConfig(self):
        #reset the config params
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

    def setUp(self):
        self.initConfig()
        
    def tearDown(self):
        self.lib.disconnect()

    def test_config(self):
        self.connect(self.test_config.__name__+'.log')
        
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)    
        
        self.checkLog('recordLength',self.config['recordLength'],int)
        self.checkLog('bufferLen',self.config['bufferSize'],int)
        self.checkLog('nbrBuffers',1,int)
        self.checkLog('recordsPerBuffer',1,int)
        self.checkLog('recordsPerAcquisition',1,int)
        self.checkLog('partialBuffer',0,int)
        self.checkLog('samplesPerAcquisition',4096,int)
        self.checkLog('numberAcquistions',1,int)
        self.checkLog('Input Range',10,int)
        self.checkLog('Counts2Volts',0.0078125,float)
        self.checkLog('Trigger Level Code',153,int)
        self.checkLog('Trigger Delay',5e6,int)
    
    def test_single_record_too_large(self):
        self.connect(self.test_single_record_too_large.__name__+'.log')
        self.config['bufferSize'] = 4096
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertNotEqual(ret,0)   

    def test_max_buffer_size_exceeded(self):
        self.connect(self.test_max_buffer_size_exceeded.__name__+'.log')
        self.config['bufferSize'] = 2**23
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)    
        
    def test_record_length_alignment(self):
        self.connect(self.test_record_length_alignment.__name__+'.log')
        self.config['recordLength'] = 257
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertNotEqual(ret,0)    
        
    def test_record_length_min(self):
        self.connect(self.test_record_length_min.__name__+'.log')
        self.config['recordLength'] = 255
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertNotEqual(ret,0)    
        
    def test_one_round_robin_per_buffer(self):
        logFile = self.test_one_round_robin_per_buffer.__name__+'.log'
        self.connect(logFile)
        self.config['recordLength'] = 1024
        self.config['nbrWaveforms'] = 4
        self.config['bufferSize'] = 8192
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,0)    
        self.checkLog('roundRobinsPerBuffer',1,int)
        self.lib.disconnect()

    def test_multiple_round_robins_per_buffer(self):
        logFile = self.test_multiple_round_robins_per_buffer.__name__+'.log'
        self.connect(logFile)
        self.config['recordLength'] = 256
        self.config['nbrWaveforms'] = 4
        self.config['nbrRoundRobins'] = 32
        self.config['bufferSize'] = 8192
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,0)    
        self.checkLog('roundRobinsPerBuffer',4,int)
        
    def test_multiple_buffers_per_round_robin(self):
        logFile = self.test_multiple_buffers_per_round_robin.__name__+'.log'
        self.connect(logFile)
        self.config['recordLength'] = 4096
        self.config['nbrWaveforms'] = 3
        self.config['nbrRoundRobins'] = 3
        self.config['bufferSize'] = 8192
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,0)    
        self.checkLog('buffersPerRoundRobin',3,int)
        
    def test_mode_config(self):
        logFile = self.test_mode_config.__name__+'.log'
        self.connect(logFile)
        self.config['acquireMode'] = 'avverager'
        ret = self.lib.setAll(1,1,self.config)
        self.initConfig()
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   

    def test_channel_coupling(self):
        logFile = self.test_channel_coupling.__name__+'.log'
        self.connect(logFile)
        self.config['verticalCoupling'] = 'ac'
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   

    def test_trigger_coupling(self):
        logFile = self.test_trigger_coupling.__name__+'.log'
        self.connect(logFile)
        self.config['triggerCoupling'] = 'ac'
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
    def test_trigger_source(self):
        logFile = self.test_trigger_coupling.__name__+'.log'
        self.connect(logFile)
        self.config['triggerSource'] = 'ext'
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
    def test_trigger_slope(self):
        logFile = self.test_trigger_coupling.__name__+'.log'
        self.connect(logFile)
        self.config['triggerSource'] = 'rising'
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
    def test_trigger_bandwidth(self):
        logFile = self.test_trigger_coupling.__name__+'.log'
        self.connect(logFile)
        self.config['bandwidth'] = ''
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
    def test_channel_scale(self):
        logFile = self.test_channel_scale.__name__+'.log'
        self.connect(logFile)
        self.config['verticalScale'] = 99.0
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
        self.config['verticalScale'] = 2.0
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   
        
    def test_sample_rate(self):
        logFile = self.test_sample_rate.__name__+'.log'
        self.connect(logFile)
        self.config['samplingRate'] = 59e6
        ret = self.lib.setAll(1,1,self.config)
        #expecting a -1 here - testing that the call fails
        self.assertEqual(ret,-1)   
        
        self.config['samplingRate'] = 500e6
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   

    def test_digitizer(self):        
        logFile = self.test_digitizer.__name__+'.log'
        
        self.connect(logFile)
        
        self.config['acquireMode']      = 'digitizer'
        self.config['recordLength']     = 1024
        self.config['nbrWaveforms']     = 3
        self.config['nbrSegments']      = 5
        self.config['nbrRoundRobins']   = 3
        
        #configure for using exactly 3 buffers
        self.config['bufferSize']       = 1024*2*3*5
        
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   
        
        ret = self.lib.acquire()
        self.assertEqual(ret,0)   
        
        self.compareData()
        self.lib.disconnect()
          
    def test_averager(self):        
        logFile = self.test_averager.__name__+'.log'
        
        self.connect(logFile)
        
        self.config['acquireMode']      = 'averager'
        self.config['recordLength']     = 1024
        self.config['nbrWaveforms']     = 5
        self.config['nbrSegments']      = 3
        
        #pattern generator currently only supports 1 round robin
        self.config['nbrRoundRobins']   = 1
        
        #configure for using exactly 3 buffers
        self.config['bufferSize']       = 1024*5*3*3*2
        
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   
        
        ret = self.lib.acquire()
        self.assertEqual(ret,0)   
        
        self.compareData()
        self.lib.disconnect()
        
    def test_partial_buffer_digitizer(self):   
        logFile = self.test_partial_buffer_digitizer.__name__+'.log'
        
        self.connect(logFile)
        
        self.config['acquireMode']      = 'digitizer'
        self.config['recordLength']     = 1024
        self.config['nbrWaveforms']     = 7
        self.config['nbrSegments']      = 9
        self.config['nbrRoundRobins']   = 1
        
        #configure for 21 buffers per round robin
        self.config['bufferSize']       = 1024*2*3
        
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   
        
        ret = self.lib.acquire()
        self.assertEqual(ret,0)   
        
        self.compareData()
        
        
    def test_partial_buffer_averager(self):   
        logFile = self.test_partial_buffer_averager.__name__+'.log'
        
        self.connect(logFile)
        
        self.config['acquireMode']      = 'averager'
        self.config['recordLength']     = 1024
        self.config['nbrWaveforms']     = 7
        self.config['nbrSegments']      = 9
        self.config['nbrRoundRobins']   = 1
        
        #configure for 21 buffers per round robin
        self.config['bufferSize']       = 1024*2*3
        
        ret = self.lib.setAll(1,1,self.config)
        self.assertEqual(ret,0)   
        
        ret = self.lib.acquire()
        self.assertEqual(ret,0)   
        
        self.compareData()
        
if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAPI)
    unittest.TextTestRunner(verbosity=2).run(suite)
    #unittest.main()