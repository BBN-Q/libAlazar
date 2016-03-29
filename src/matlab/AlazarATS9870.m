classdef AlazarATS9870 < hgsetget
    % Class driver file for Alazar Tech ATS9870 PCI digitizer
    %
    % Author(s): Rob McGurrin
    % Code started: 02 December 2015
    
    properties (Access = public)
        %Assume un-synced boards so that address = 1
        systemId = 1
        address = 1

        %The single-shot or averaged data (depending on the acquireMode)
        data
        
        %used to keep running average of the data
        sumDataA
        sumDataB
        
        %How long to wait for a buffer to fill (seconds)
        timeOut = 30;
        lastBufferTimeStamp = 0;
        
        %All the settings for the device
        settings
        
    end
    
    properties (Access = private)
        % c lib buffer pointers for the API
        ch1Ptr
        ch2Ptr
        
    end
        
    events
        DataReady
    end
    
    methods (Access = public)
        %Constuctor which loads definitions and dll
        function obj = AlazarATS9870()
        
           
            %todo - make lib paths configurable
            %todo - handle thunk file            
            if ~loadLibAlazar()
                error('Can''t load libAlazar');
            end
            
            %set default settings
            obj.settings.acquireMode                 = 'digitizer';
            obj.settings.vertical.bandwidth          = 'Full';
            obj.settings.clockType                   = 'ref';
            obj.settings.horizontal.delayTime        = 0.0;
            obj.settings.enabled                     = true;
            obj.settings.deviceName                  = 'ATS9870';
            obj.settings.averager.recordLength       = 4096;
            obj.settings.averager.nbrSegments        = 1;
            obj.settings.averager.nbrWaveforms       = 3;
            obj.settings.averager.nbrRoundRobins     = 1;
            obj.settings.horizontal.samplingRate     = 500e6;
            obj.settings.trigger.triggerCoupling     = 'DC';
            obj.settings.trigger.triggerLevel        = 1000;
            obj.settings.trigger.triggerSlope        = 'rising';
            obj.settings.trigger.triggerSource               = 'Ext';
            obj.settings.vertical.verticalCoupling   = 'AC';
            obj.settings.vertical.verticalOffset     = 0.0;
            obj.settings.vertical.verticalScale      = 4.0;
            obj.settings.bufferSize                  = 4194304;
            
        end
        
        %Destructor
        function delete(obj)
            obj.stop();
            obj.disconnect();
            unloadlibrary libAlazar;
        end
        
        function connect(obj,address,logname)
            
            if nargin < 3
                logname = 'libAlazar.log';
            end
            
            obj.address = address;
            
            ret = calllib('libAlazar','connectBoard',address,logname);     
            if ret < 0
                error('Can''t Connect to board')
            end
        end
        
        function disconnect(obj)
           calllib('libAlazar','disconnect',obj.address);
        end
        
        %Function to flash the LED (at least then we know something works).
        function flash_LED(~, numTimes, period)
           calllib('libAlazar','flash_led',numTimes,period);

        end
        
        %Instrument meta-setter that sets all parameters
        function setAll(obj, settings)
            
            obj.settings = settings;
            
            %todo - parse system address and id from settings
            %todo - parse settings value from settings struct
            
            %todo - create data structure and call API function
            conf.acquireMode        = settings.acquireMode;
            conf.bandwidth          = settings.vertical.bandwidth;
            conf.clockType          = settings.clockType;
            conf.delay              = settings.horizontal.delayTime;
            conf.enabled            = true;
            conf.label              = settings.deviceName;
            conf.recordLength       = settings.averager.recordLength;
            conf.nbrSegments        = settings.averager.nbrSegments;
            conf.nbrWaveforms       = settings.averager.nbrWaveforms;
            conf.nbrRoundRobins     = settings.averager.nbrRoundRobins;
            conf.samplingRate       = settings.horizontal.samplingRate;
            conf.triggerCoupling    = settings.trigger.triggerCoupling;
            conf.triggerLevel       = settings.trigger.triggerLevel;
            conf.triggerSlope       = settings.trigger.triggerSlope;
            conf.triggerSource      = settings.trigger.triggerSource;
            conf.verticalCoupling   = settings.vertical.verticalCoupling;
            conf.verticalOffset     = settings.vertical.verticalOffset;
            conf.verticalScale      = settings.vertical.verticalScale;
            
            if isfield(settings,'bufferSize')
                conf.bufferSize         = settings.bufferSize;
            else
                conf.bufferSize         = 4194304;
            end

            acq.samplesPerAcquisition = 0;
            acq.numberAcquistions = 0;
            
            [ret,~,acq] = calllib('libAlazar','setAll',obj.address,conf,acq);
                        
            if ret < 0
                error('Could not set config parameters')
            end
            
            obj.settings.samplesPerAcquisition = acq.samplesPerAcquisition;
            obj.settings.numberAcquistions = acq.numberAcquistions;
            
            %allocate the the data buffers
            obj.ch1Ptr = libpointer('singlePtr',single(zeros(1,obj.settings.samplesPerAcquisition)));
            obj.ch2Ptr = libpointer('singlePtr',single(zeros(1,obj.settings.samplesPerAcquisition)));

            if strcmp(obj.settings.acquireMode, 'averager')
                obj.sumDataA = zeros([obj.settings.averager.recordLength, obj.settings.averager.nbrSegments]);
                obj.sumDataB = zeros([obj.settings.averager.recordLength, obj.settings.averager.nbrSegments]);
            else
                obj.sumDataA = [];
                obj.sumDataB = [];
            end


        end
        
        %Setup and start an acquisition
        function acquire(obj)
            ret = calllib('libAlazar','acquire',obj.address);
            if ret < 0
                error('Acquire API call failed')
            end
         end
        
        function stop(obj)
            
            ret = calllib('libAlazar','stop',obj.address);            
            if ret < 0
                error('Stop Failed: %d',ret);
            end
            
            clear obj.ch1Ptr;
            clear obj.ch2Ptr
            
       end
        
        %Wait for the acquisition to complete 
        function wait_for_acquisition(obj, timeout)

            for i=1:obj.settings.numberAcquistions
                
                ret = 0;
                start=clock;
                while ~ret
                    
                    ret=calllib('libAlazar','wait_for_acquisition',obj.address,obj.ch1Ptr,obj.ch2Ptr);
                    
                    if ret < 0
                        error('Acquisition Failed - API Error')
                    end
                    
                    if ret
                        start=clock;
                    else
                        if etime(clock,start) > timeout
                            error('Acquisition Failed - Timeout')
                        end
                    end

                end

                
                if strcmp(obj.settings.acquireMode, 'averager')
                    
                    obj.data{1} = reshape(obj.ch1Ptr.value,[obj.settings.averager.recordLength,obj.settings.averager.nbrSegments]);
                    obj.data{2} = reshape(obj.ch2Ptr.value,[obj.settings.averager.recordLength,obj.settings.averager.nbrSegments]);
                    
                    obj.sumDataA  = obj.sumDataA + obj.data{1};
                    obj.sumDataB  = obj.sumDataB + obj.data{2};
                else
                    nbrRRPerBuffer = obj.settings.samplesPerAcquisition/(obj.settings.averager.recordLength*obj.settings.averager.nbrWaveforms*obj.settings.averager.nbrSegments);
                    dims = [obj.settings.averager.recordLength,obj.settings.averager.nbrWaveforms,obj.settings.averager.nbrSegments,nbrRRPerBuffer];
                    obj.data{1} = reshape(obj.ch1Ptr.value,dims);
                    obj.data{2} = reshape(obj.ch2Ptr.value,dims);
                end

                notify(obj,'DataReady')


            end
            
            if strcmp(obj.settings.acquireMode, 'averager')
                obj.sumDataA = obj.sumDataA / obj.settings.numberAcquistions;
                obj.sumDataB = obj.sumDataB / obj.settings.numberAcquistions;
            end
            
        end
        
        % Dummy function for consistency with Acqiris card where average
        % data is stored on card
        function [avgWaveform, times] = transfer_waveform(~, ~)
            %todo - TBD
        end
        
        
    
    end %methods
    
end %classdef

