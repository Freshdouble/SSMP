#include <inttypes.h>

typedef void (*DataReceivedCallback)(uint8_t* data, uint8_t length);

typedef struct
{
    uint16_t bufferlength;
    uint8_t* buffer;
    DataReceivedCallback Datareceived;
}ssmp_settings_t;

typedef struct
{
    uint8_t* writePtr;
    uint8_t decoderstate;
    uint8_t payloadlength;
    uint16_t receivedchecksum;
    uint8_t sum1;
    uint8_t sum2;
}ssmp_status_t;

typedef struct
{
    ssmp_settings_t settings;
    ssmp_status_t status;
}ssmp_t;

#ifdef ENABLE_OOPINTERFACE
ssmp_t* SSMP_CreateInstance(uint16_t bufferlength, DataReceivedCallback datareceived);
void SSMP_DestroyInstance(ssmp_t* instance);
#endif

int SSMP_Init(ssmp_t* ssmp, ssmp_settings_t* settings);
void SSMP_ResetDecoderState(ssmp_t* ssmp);
int SSMP_DecodeByte(ssmp_t* ssmp, uint8_t data);
uint16_t SSMP_CreatePacket(uint8_t* data, uint8_t length, uint8_t* packetbuffer, uint16_t packetbufferlength);

static inline uint8_t GetLastReceivedDataLength(ssmp_t* ssmp)
{
    return ssmp->status.writePtr - ssmp->settings.buffer;
}

static inline uint8_t* GetLastDataBuffer(ssmp_t* ssmp)
{
    return ssmp->settings.buffer;
}
