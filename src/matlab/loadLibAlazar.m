function [result] = loadLibAlazar()

% set default return code to indicate failure
result = false;

% Load driver library 
if ~libisloaded('libAlazar')    
    if strcmpi(computer('arch'), 'win64') 
        % Use protofile for 64-bit MATLAB
        % assumes lib is in Matlab's lib search path
        try
            [ret,~]=loadlibrary('libAlazar.dll',@libAlazar_pcwin64);
        catch
            error('Can''t find libAlazar.dll');
        end
    elseif strcmpi(computer('arch'), 'maci64') 
        % osx used for test and development
        try
            [~,~]=loadlibrary('libAlazar.dylib','libAlazarAPI.h');       
        catch
            error('Can''t find libAlazar.dylib');            
        end
    end
    if libisloaded('libAlazar')
        result = true;
    end
else
    % The driver is aready loaded
    result = true;
end

end