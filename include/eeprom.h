#ifndef GUARD_EEPROM_H
#define GUARD_EEPROM_H

#include "global.h"

#define REG_ADDR_EEPROM     (void *)0xD000000
#define REG_EEPROM          (*(vu16 *)REG_ADDR_EEPROM)

// EEPROM Error codes
#define EEPROM_OUT_OF_RANGE 0x80FF
#define EEPROM_VERIFY_FAIL  0x8000
#define EEPROM_WRITE_FAIL   0xC001

typedef struct EEPROMConfig {
    u32 unk_00;
    u16 size;
    u16 waitcnt;
    u8 address_width;
} EEPROMConfig;

u16 IdentifyEeprom(u16 sizeInKbit);
void EepromTimerIntr(void);
u16 SetEepromTimerIntr(u8 timerNo, u32 *timerPtr);
void StartEepromTimer(const u16 *maxTime);
void StopEepromTimer(void);
void Dma3Transmit(void *src, void *dest, u16 size);
u16 ReadEepromDword(u16 address, u16 *data);
u16 ProgramEepromDword(u16 address, const u16 *data);
u16 VerifyEepromDword(u16 address, u16 *data);
u16 ProgramEepromDwordEx(u16 address, u16 *data);

#endif // GUARD_EEPROM_H
