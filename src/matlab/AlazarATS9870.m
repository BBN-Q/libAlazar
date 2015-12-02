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
        [~,~] = loadlibrary('../../build/libAlazar.dylib','../lib/libAlazarAPI.h');
            
        end
        
        %Destructor
        function delete(obj)
            obj.stop();
            obj.disconnect();
        end
        
        
        function connect(obj, address)
            %Get the handle to the board
            %If only one board is installed, address = 1
            if ~isnumeric(address)
                address = str2double(address);
            end
            obj.systemId = address;
        end
        
        function disconnect(obj)

        end
        
        %Helper function to make an API call and error check
        function retCode = call_API(obj, functionName, varargin)

        end
        
        %Function to flash the LED (at least then we know something works).
        function flash_LED(obj, numTimes, period)
            if nargin < 3
                period = 1;
            end
            if nargin < 2
                numTimes = 10;
            end
            for ct = 1:numTimes
                %todo - call API function
                pause(period/2);
                
                %todo - call API function
                pause(period/2);
            end
        end
        
        %Instrument meta-setter that sets all parameters
        function setAll(obj, settings)
            %todo - create data structure and call API function
        end
        
        %Setup and start an acquisition
        function acquire(obj)
            %todo - call API function
        end
        
        function stop(obj)
            %todo - call API function
        end
        
        
        %Wait for the acquisition to complete and average in software
        function status = wait_for_acquisition(obj, timeOut)
            if ~exist('timeOut','var')
                timeOut = obj.timeOut;
            end
            
            %Loop until all are processed
            while toc(obj.lastBufferTimeStamp) < timeOut
                if obj.done
                    status = 0;
                    return
                else
                    pause(0.2);
                end
            end
            status = -1;
            warning('AlazarATS9870:TIMEOUT', 'AlazarATS9870 timed out while waiting for acquisition');
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
    
    end
end %classdef
