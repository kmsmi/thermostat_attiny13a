#ifndef DS18B20_H_
#define DS18B20_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

// Device resolution
#define TEMP_9_BIT 0x1F  //  9 bit 0,5С
#define TEMP_10_BIT 0x3F // 10 bit 0,25С
#define TEMP_11_BIT 0x5F // 11 bit 0,125С
#define TEMP_12_BIT 0x7F // 12 bit 0,0625С

// --- Configuration for OneWire
#define ONEWIRE_PORTReg PORTB
#define ONEWIRE_DDRReg DDRB
#define ONEWIRE_PINReg PINB
#define ONEWIRE_PIN PB0

// - Set OneWire pin in Output mode
#define OneWire_setPinAsOutput ONEWIRE_DDRReg |= (1 << ONEWIRE_PIN)

// - Set OneWire pin in Input mode
#define OneWire_setPinAsInput ONEWIRE_DDRReg &= ~(1 << ONEWIRE_PIN)

// - Set LOW level on OneWire pin
#define OneWire_writePinLOW ONEWIRE_PORTReg &= ~(1 << ONEWIRE_PIN)

// - Set HIGH level on OneWire pin
#define OneWire_writePinHIGH ONEWIRE_PORTReg |= (1 << ONEWIRE_PIN)

// - Read level from OneWire pin
#define OneWire_readPin ((ONEWIRE_PINReg & (1 << ONEWIRE_PIN)) ? 1 : 0)


extern void setResolution(uint8_t resolution);

extern void requestTemp();

extern uint16_t readTemp(bool raw);

#endif // DS18B20_H_
