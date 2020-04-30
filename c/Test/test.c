#include "../ssmp.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static uint8_t buffer[100];

static uint8_t receivedPayload[0xFF];
static uint8_t payloadlength;

static bool SequenceEqual(uint8_t* data1, uint8_t* data2, uint32_t length)
{
    for(uint32_t i = 0; i < length; i++)
    {
        if(data1[i] != data2[i])
            return false;
    }
    return true;
}

void datareceived(uint8_t* data, uint8_t length)
{
    memcpy(receivedPayload, data, length);
    payloadlength = length;
}

void main(void)
{
    uint8_t packetbuffer[210];
    ssmp_t instance;
    ssmp_settings_t settings;
    settings.Datareceived = 0;
    settings.bufferlength = sizeof(buffer);
    settings.buffer = buffer;
    memset(packetbuffer, 0, sizeof(packetbuffer));

    SSMP_Init(&instance, &settings);

    uint8_t payload[] = {0x01,0x02,0x03,4,5,10,0xFF};

    memset(receivedPayload, 0, sizeof(receivedPayload));
    payloadlength = 0;
    uint16_t toTransmit = SSMP_CreatePacket(payload, sizeof(payload), packetbuffer, sizeof(packetbuffer));
    if(toTransmit == 0)
    {
        printf("Couldn't create packet\n");
        return;
    }

    for(int i = 0; i < toTransmit; i++)
    {
        SSMP_DecodeByte(&instance, packetbuffer[i]);
    }

    if(GetLastReceivedDataLength(&instance) == 0)
    {
        printf("No data received\n");
        return;
    }

    if(!SequenceEqual(GetLastDataBuffer(&instance), payload, GetLastReceivedDataLength(&instance)))
    {
        printf("Wrong data received\n");
        return;
    }

    printf("Test passed\n");
}