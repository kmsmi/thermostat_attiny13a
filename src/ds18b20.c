#include <util/delay.h>
#include "ds18b20.h"

//! Reset function
static uint8_t OneWire_reset()
{

    // - Wait for line
    uint8_t Retries = 125;
    OneWire_setPinAsInput;
    do
    {
        if (--Retries == 0)
            return 0;
        _delay_us(2);
    } while (!OneWire_readPin);

    // - Drop line
    OneWire_writePinLOW;
    OneWire_setPinAsOutput;
    _delay_us(480);

    // - Listen for reply pulse
    OneWire_setPinAsInput;
    _delay_us(70);

    // - Read line state
    const uint8_t State = !OneWire_readPin;
    _delay_us(410);
    return State;
}

//! Write single bit
static void OneWire_writeBit(const uint8_t Bit)
{
    if (Bit & 1)
    {
        // - Drop line
        OneWire_writePinLOW;
        OneWire_setPinAsOutput;
        // - Write Bit-1
        _delay_us(10);
        OneWire_writePinHIGH;
        _delay_us(55);
    }
    else
    {
        // - Drop line
        OneWire_writePinLOW;
        OneWire_setPinAsOutput;
        // - Write Bit-0
        _delay_us(65);
        OneWire_writePinHIGH;
        _delay_us(5);
    }
}

//! Read single bit
static uint8_t OneWire_readBit()
{
    // - Drop line
    OneWire_setPinAsOutput;
    OneWire_writePinLOW;
    _delay_us(3);

    // - Wait for data
    OneWire_setPinAsInput;
    _delay_us(10);

    // - Read bit into byte
    const uint8_t Bit = OneWire_readPin;
    _delay_us(53);
    return Bit;
}

//! Write byte
static void OneWire_writeByte(const uint8_t Byte)
{

    // - Write each bit
    for (uint8_t BitMask = 0x01; BitMask; BitMask <<= 1)
        OneWire_writeBit(BitMask & Byte ? 1 : 0);
}
//! Read byte
static uint8_t OneWire_readByte()
{
    uint8_t Byte = 0;

    // - Read all bits
    for (uint8_t BitMask = 0x01; BitMask; BitMask <<= 1)
    {
        // - Read & store bit into byte
        if (OneWire_readBit())
            Byte |= BitMask;
    }
    return Byte;
}


void setResolution(const uint8_t resolution)
{
    OneWire_reset();
    OneWire_writeByte(0xCC);
    OneWire_writeByte(0x4E);       // запись в блокноте
    OneWire_writeByte(0x00);       // Байт пользователя 0 - неиспользуемый
    OneWire_writeByte(0x00);       // Байт пользователя 1 - неиспользуемый
    OneWire_writeByte(resolution); // настройка  12 бит (0x7F)
}

void requestTemp()
{
    OneWire_reset();
    OneWire_writeByte(0xCC);
    OneWire_writeByte(0x44);
}

uint16_t readTemp(const bool raw)
{
    uint16_t t = 0;
    OneWire_reset();
    OneWire_writeByte(0xCC);
    OneWire_writeByte(0xBE);

    t = OneWire_readByte();
    t = OneWire_readByte() << 8 | t;
    if (raw)
        return t;
    t = (t * 10) >> 4;
    return t;
}