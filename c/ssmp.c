#include "ssmp.h"
#include <string.h>

#ifdef ENABLE_OOPINTERFACE
#include <stdlib.h>
#endif

#ifdef ENABLE_OOPINTERFACE
ssmp_t* SSMP_CreateInstance(uint16_t bufferlength, DataReceivedCallback datareceived)
{
    uint8_t* buffer = malloc(sizeof(uint8_t)*bufferlength);
    if(buffer != 0)
    {
        ssmp_t* instance = malloc(sizeof(ssmp_t));
        if(instance != 0)
        {
            ssmp_settings_t settings;
            settings.buffer = buffer;
            settings.bufferlength = bufferlength;
            settings.Datareceived = datareceived;

            if(SSMP_Init(instance, &settings) >= 0)
            {
                return instance;
            }
            else
            {
                free(buffer);
                free(instance);
            }          
        }
        else
        {
            free(buffer);
        }
    }
    return 0;
}

void SSMP_DestroyInstance(ssmp_t* instance)
{
    free(instance->settings.buffer);
    free(instance);
}
#endif

int SSMP_Init(ssmp_t* ssmp, ssmp_settings_t* settings)
{
    if(settings == 0)
    {
        return -1;
    }
    memcpy(ssmp, settings, sizeof(ssmp_settings_t));
    SSMP_ResetDecoderState(ssmp);
    return 0;
}

void SSMP_ResetDecoderState(ssmp_t* ssmp)
{
    memset(&ssmp->status, 0, sizeof(ssmp_status_t));
    ssmp->status.writePtr = ssmp->settings.buffer;
}

int SSMP_DecodeByte(ssmp_t* ssmp, uint8_t data)
{
    switch(ssmp->status.decoderstate)
    {
        case 0:
            if(data == 255)
            {
                SSMP_ResetDecoderState(ssmp);
                ssmp->status.decoderstate = 1;
            }
        break;
        case 1:
            ssmp->status.payloadlength = data;
            ssmp->status.decoderstate = 2;
        break;
        case 2:
            if(data == ssmp->status.payloadlength)
            {
                ssmp->status.decoderstate = 3;
            }
            else
            {
                ssmp->status.decoderstate = 0;
            }
        break;
        case 3:
            *ssmp->status.writePtr = data;
            ssmp->status.writePtr++;
            if(ssmp->status.writePtr == ssmp->settings.buffer + ssmp->settings.bufferlength)
            {
                ssmp->status.decoderstate = 0;
            }
            ssmp->status.sum1 += data;
            ssmp->status.sum2 += ssmp->status.sum1;
            ssmp->status.payloadlength--;
            if(ssmp->status.payloadlength == 0)
            {
                ssmp->status.decoderstate = 4;
                ssmp->status.payloadlength = 1;
            }
        break;
        case 4:
            if(ssmp->status.payloadlength > 0)
            {
                ssmp->status.receivedchecksum = data;
                ssmp->status.payloadlength = 0;
            }
            else
            {
                if(ssmp->status.receivedchecksum == ssmp->status.sum2 && data == ssmp->status.sum1)
                {
                    if(ssmp->settings.Datareceived != 0)
                    {
                        ssmp->settings.Datareceived(ssmp->settings.buffer, GetLastReceivedDataLength(ssmp));
                    }
                }
                ssmp->status.decoderstate = 0;
                return 1; //Indicate that a packet was received
            }
        break;
        default:
            ssmp->status.decoderstate = 0;
        break;
    }
    return 0;
}

uint16_t SSMP_CreatePacket(uint8_t* data, uint8_t length, uint8_t* packetbuffer, uint16_t packetbufferlength)
{
    uint8_t sum1 = 0;
    uint8_t sum2 = 0;
    uint16_t i;
    if(packetbufferlength < length + 5)
    {
        return 0;
    }

    packetbuffer[0] = 255;
    packetbuffer[1] = length;
    packetbuffer[2] = length;
    for(i = 0; i < length; i++)
    {
        sum1 += data[i];
        sum2 += sum1;
        packetbuffer[i + 3] = data[i];
    }
    packetbuffer[i + 3] = sum2;
    packetbuffer[i + 4] = sum1;
    return i + 5;
}