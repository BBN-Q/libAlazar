classdef AlazarATS9870 < hgsetget
    % Class driver file for Alazar Tech ATS9870 PCI digitizer
    %
    % Author(s): Rob McGurrin
    % Code started: 02 December 2015
    
    properties (Access = public)
        %Assume un-synced boards so that address = 1
        systemId = 1
        address = 1
        name = '';
        
        %The single-shot or averaged data (depending on the acquireMode)
        data
        
        %Acquire mode controls whether we return single-shot results or
        %averaged data
        acquireMode = 'averager';
        
        %How long to wait for a buffer to fill (seconds)
        timeOut = 30;
        lastBufferTimeStamp = 0;
        
        %All the settings for the device
        settings
        
        %Vertical scale
        verticalScale
    end
    
    properties (Access = private)
        %Handle to the board for the C API
        boardHandle
        
    end
    
    properties (Dependent = true)
        
        averager;  
        %ditherRange %needs to be implemented in software
    end
    
    properties (Constant = true)
        model_number = 'ATS9870';
                
        % The size of the memory of the card
        onBoardMemory = 256e6;
    end
    
    events
        DataReady
    end
    
    methods (Access = public)
        %Constuctor which loads definitions and dll
        function obj = AlazarATS9870()
        
        %todo - make paths configurable
        
        if( ismac )
            % the mac build os for testin only - it uses a simulated Alazar
            % shared library
            [~,~] = loadlibrary(fullfile(pwd,'../../build/lib/libAlazar.dylib'),fullfile(pwd,'../../build/lib/libAlazarAPI.h'));
        else
            %todo - fix for windows
            [~,~] = loadlibrary(fullfile(pwd,'../../build/lib/libAlazar.dll'),fullfile(pwd,'../../build/lib/libAlazarAPI.h'));
        end
        
        end
        
        %Destructor
        function delete(obj)
            obj.stop();
            obj.disconnect();
            unloadlibrary('libALazar');
        end
        
        
        function connect(obj)
           calllib('libAlazar','connect','foo.log');           
        end
        
        function disconnect(obj)
           calllib('libAlazar','disconnect');
        end
        
        %Helper function to make an API call and error check
        function retCode = call_API(obj, functionName, varargin)

        end
        
        %Function to flash the LED (at least then we know something works).
        function flash_LED(obj, numTimes, period)
           calllib('libAlazar','flash_led',numTimes,period);

        end
        
        %Instrument meta-setter that sets all parameters
        function setAll(obj, settings)
            %todo - create data structure and call API function
        end
        
        %Setup and start an acquisition
        function acquire(obj)
            
            calllib('libAlazar','acquire');

            
        end
        
        function stop(obj)
            
            calllib('libAlazar','stop');
        end
        
        
        %Wait for the acquisition to complete and average in software
        function status = wait_for_acquisition(obj, timeOut)
            
            calllib('libAlazar','wait_for_acquisition');
            pause(timeOut);
            fprintf(1,'HELLO\n');
            
        end
        
        % Dummy function for consistency with Acqiris card where average
        % data is stored on card
        function [avgWaveform, times] = transfer_waveform(obj, channel)
            %todo - TBD
        end
        
        
    
    end %methods
    
    %Getter/setters must be in an methods block without attributes
    methods
    
    end %methods
    
    methods (Static)
        
        function unit_test()
            scope = AlazarATS9870();
            scope.connect();

            %scope.horizontal = struct('samplingRate', 500e6, 'delayTime', 0);
            %scope.vertical = struct('verticalScale', 1.0, 'verticalCoupling', 'AC', 'bandwidth', 'Full');
            %scope.trigger = struct('triggerLevel', 100, 'triggerSource', 'ext', 'triggerCoupling', 'DC', 'triggerSlope', 'rising');
            %scope.averager = struct('recordLength', 4096, 'nbrSegments', 1, 'nbrWaveforms', 1, 'nbrRoundRobins', 1000, 'ditherRange', 0);
            
            scope.acquire();
            scope.wait_for_acquisition(1);
            scope.stop();  
            scope.flash_LED(1,1);
            scope.disconnect();
        end

    
    end
end %classdef
