classdef RespondToDataReady < handle
    properties
        ListenerHandle % Property for listener handle
        data
    end
    
    methods
        function obj = RespondToDataReady(src)
            hl = addlistener(src,'DataReady',@RespondToDataReady.handleEvnt);
        end
    end
    
    methods (Static)
        function handleEvnt(src,~)
            obj.data = src.data;            
        end
    end
end