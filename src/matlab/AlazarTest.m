%test for the AlazarATS9870 matlab driver
classdef AlazarTest < matlab.unittest.TestCase
 
    properties
        a
        r
    end
 
    methods(TestMethodSetup)
        function setUp(testCase)
            testCase.a= AlazarATS9870();
            testCase.r = RespondToDataReady(testCase.a);
            testCase.a.connect(1,'unittest.log')
        end
    end
 
    methods(TestMethodTeardown)
        function tearDown(testCase)
            testCase.a.stop()
            testCase.a.disconnect();
            testCase.a.delete();
            testCase.r.delete();
        end
    end
 
    methods(Test)
 
        function testDigitizer(testCase)
            testCase.a.settings.acquireMode      = 'digitizer';
            testCase.a.settings.averager.recordLength     = 1024;
            testCase.a.settings.averager.nbrWaveforms     = 7;
            testCase.a.settings.averager.nbrSegments      = 9;
            testCase.a.settings.averager.nbrRoundRobins   = 10;
            testCase.a.settings.bufferSize       = 1024*7*9*2;

            testCase.a.setAll(testCase.a.settings);
            testCase.a.acquire();
            testCase.a.wait_for_acquisition(10);
            
            testCase.verifyEqual(testCase.a.settings.samplesPerAcquisition, 64512);
            testCase.verifyEqual(testCase.a.settings.numberAcquistions, 10);
            
            testCase.verifyEqual((size(testCase.a.data{1}) == [1024,7,9]),logical([1 1 1]));
            testCase.verifyEqual((size(testCase.a.data{2}) == [1024,7,9]),logical([1 1 1]));
           
        end
 
         function testDigitizerMultipleRR(testCase)
            testCase.a.settings.acquireMode      = 'digitizer';
            testCase.a.settings.averager.recordLength     = 1024;
            testCase.a.settings.averager.nbrWaveforms     = 2;
            testCase.a.settings.averager.nbrSegments      = 2;
            testCase.a.settings.averager.nbrRoundRobins   = 10;
            testCase.a.settings.bufferSize       = 1024*2*2*2*5;

            testCase.a.setAll(testCase.a.settings);
            testCase.a.acquire();
            testCase.a.wait_for_acquisition(10);
            
            testCase.verifyEqual(testCase.a.settings.samplesPerAcquisition, 20480);
            testCase.verifyEqual(testCase.a.settings.numberAcquistions, 2);
            
            testCase.verifyEqual((size(testCase.a.data{1}) == [1024,2,2,5]),logical([1 1 1 1]));
            testCase.verifyEqual((size(testCase.a.data{2}) == [1024,2,2,5]),logical([1 1 1 1]));
           
         end
        
        function testDigitizerPartial(testCase)
            testCase.a.settings.acquireMode      = 'digitizer';
            testCase.a.settings.averager.recordLength     = 1024;
            testCase.a.settings.averager.nbrWaveforms     = 7;
            testCase.a.settings.averager.nbrSegments      = 9;
            testCase.a.settings.averager.nbrRoundRobins   = 1;
            
            testCase.a.setAll(testCase.a.settings);
            testCase.a.acquire();
            testCase.a.wait_for_acquisition(10);
            
            testCase.verifyEqual(testCase.a.settings.samplesPerAcquisition, 64512);
            testCase.verifyEqual(testCase.a.settings.numberAcquistions, 1);
            
            testCase.verifyEqual((size(testCase.a.data{1}) == [1024,7,9]),logical([1 1 1]));
            testCase.verifyEqual((size(testCase.a.data{2}) == [1024,7,9]),logical([1 1 1]));
           
        end
        
        function testAverager(testCase)
            testCase.a.settings.acquireMode      = 'averager';
            testCase.a.settings.averager.recordLength     = 1024;
            testCase.a.settings.averager.nbrWaveforms     = 7;
            testCase.a.settings.averager.nbrSegments      = 9;
            testCase.a.settings.averager.nbrRoundRobins   = 10;
            testCase.a.settings.bufferSize                = 1024*2*7*9;
           
            testCase.a.setAll(testCase.a.settings);
            testCase.a.acquire();
            testCase.a.wait_for_acquisition(10);
            
            testCase.verifyEqual(testCase.a.settings.samplesPerAcquisition, 9216);
            testCase.verifyEqual(testCase.a.settings.numberAcquistions, 10);
            
            testCase.verifyEqual((size(testCase.a.data{1}) == [1024,9]),logical([1 1]));
            testCase.verifyEqual((size(testCase.a.data{2}) == [1024,9]),logical([1 1]));
           
            testCase.verifyEqual((size(testCase.a.sumDataA) == [1024,9]),logical([1 1]));
            testCase.verifyEqual((size(testCase.a.sumDataB) == [1024,9]),logical([1 1]));
            

        end
        
        function testAveragerPartial(testCase)
            
            testCase.a.settings.acquireMode                 = 'averager';
            testCase.a.settings.averager.recordLength       = 1024;
            testCase.a.settings.averager.nbrWaveforms       = 7;
            testCase.a.settings.averager.nbrSegments        = 9;
            testCase.a.settings.averager.nbrRoundRobins     = 1;
            testCase.a.settings.bufferSize                  = 1024*2*7;
            
            testCase.a.setAll(testCase.a.settings);
            testCase.a.acquire();
            testCase.a.wait_for_acquisition(10);
            
            testCase.verifyEqual(testCase.a.settings.samplesPerAcquisition, 9216);
            testCase.verifyEqual(testCase.a.settings.numberAcquistions, 1);
            
            testCase.verifyEqual((size(testCase.a.data{1}) == [1024,9]),logical([1 1]));
            testCase.verifyEqual((size(testCase.a.data{2}) == [1024,9]),logical([1 1]));
           
            testCase.verifyEqual((size(testCase.a.sumDataA) == [1024,9]),logical([1 1]));
            testCase.verifyEqual((size(testCase.a.sumDataB) == [1024,9]),logical([1 1]));
       end

    end
 
end



