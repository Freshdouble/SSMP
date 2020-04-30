classdef SSMP < handle
    %SSMP Small Simple Message protcol
    %   This class is a implementation of the small simple message protcol
    %   which is a smaller variant of the smp
    
    properties
        state
        payloadlength
        maxlength
        data        
        receivedchecksum
        %Bytes for the fletcher checksum
        sum1
        sum2
    end
    
    methods
        function obj = SSMP(maxlength)
            %SSMP Construct an instance of this class
            obj.state = 0;
            obj.maxlength = maxlength;
        end
        
        function data = GetLastMessage(obj)
            %GetLastMessage returns the payload of the last received valid packet
            data = obj.data;
        end
        
        function receivedData = ProcessByteArray(obj, data)
            receivedData = {};
            for i = 1:length(data)
                datareceived = ProcessByte(obj, data(i));
                if datareceived
                    receivedData{end + 1} = GetLastMessage(obj);
                end
            end
        end
        
        function datareceived = ProcessByte(obj, data)
            %ProcessByte Process one byte from the inputstream
            %   This function returns true when a message was decoded false
            %   otherwise
            data = double(data);
            datareceived = 0;
            switch obj.state
                case 0 %Wait for the start delimeter
                    if data == 255
                        obj.state = 1;
                    end
                case 1 %Get length parameter
                    if data > obj.maxlength
                        %The packet length is greater than the maximum
                        %allowed length so we discard the data
                        obj.state = 0;
                    else
                        obj.payloadlength = data;
                        obj.state = 2;
                        obj.data = zeros(data,1);
                        obj.sum1 = 0;
                        obj.sum2 = 0;
                    end
                case 2 %receive the header checksum
                    if data == bitand(uint32(obj.payloadlength) + 255, 255)
                        obj.state = 3;
                    else
                        %Bad header checksum
                        obj.state = 0;
                    end
                case 3 %Receive the data and store it in a array
                    obj.data(end - obj.payloadlength + 1) = data;
                    % Rolling fletcher 16 checksum
                    obj.sum1 = bitand(obj.sum1 + data, 255);
                    obj.sum2 = bitand(obj.sum2 + obj.sum1, 255);
                    
                    obj.payloadlength = obj.payloadlength - 1;
                    if obj.payloadlength == 0
                        obj.state = 4;
                        obj.payloadlength = 1;
                    end
                case 4 %Receive the checksum and compare
                    if obj.payloadlength > 0
                        obj.receivedchecksum = data; %First receive the lower byte
                        obj.payloadlength = 0;
                    else
                        if obj.receivedchecksum == obj.sum2 && data == obj.sum1%Correct message received
                            datareceived = 1;
                        end
                        obj.state = 0;
                    end
            end
        end
        
        function packet = GeneratePacket(obj, data)
            %GeneratePacket Generates a complete ssmp packet with data as
            %payload
            if length(data) > obj.maxlength
                error("Datalength is greater than the maximum payload length");
                return
            end
            packet = zeros(length(data) + 5, 1, 'uint8');
            sum1 = 0;
            sum2 = 0;
            packet(1) = 255;
            packet(2) = length(data);
            packet(3) = bitand(length(data) + 255, 255);
            for i = 1:length(data)
                packet(i + 3) = data(i);
                sum1 = bitand(sum1 + data(i),255);
                sum2 = bitand(sum2 + sum1, 255);
            end
            packet(length(data) + 4) = sum2;
            packet(length(data) + 5) = sum1;
        end
    end
end

