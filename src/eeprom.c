#include "global.h"
#include "eeprom.h"

extern const EEPROMConfig *gEEPROMConfig;
extern const EEPROMConfig gEEPROMConfig512;
extern const EEPROMConfig gEEPROMConfig8k;
extern const u16 gEepromMaxTimers[];

extern u16 timer_Count;
extern u8 timeoutFlag;
extern u8 timer_No;
extern vu16 *timerReg;
extern u16 shelt_ime;

u16 IdentifyEeprom(u16 sizeInKbit) {
    u16 ret;

    ret = 0;

    if (sizeInKbit == 4) {
        gEEPROMConfig = &gEEPROMConfig512;
    } else if (sizeInKbit == 0x40) {
        gEEPROMConfig = &gEEPROMConfig8k;
    } else {
        gEEPROMConfig = &gEEPROMConfig512;
        ret = 1;
    }

    return ret;
}

void EepromTimerIntr(void) {
    if (timer_Count != 0) {
        if (--timer_Count == 0) {
            timeoutFlag = 1;
        }
    }
}

u16 SetEepromTimerIntr(u8 timerNo, u32 *timerPtr) {
    u16 ret;

    if (timerNo <= 3) {
        timer_No = timerNo;
        timerReg = (vu16 *)&REG_TMCNT(timer_No);
        *timerPtr = (u32)EepromTimerIntr;
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
}

void StartEepromTimer(const u16 *maxTime) {
    shelt_ime = REG_IME;

    REG_IME = 0;
    REG_IE |= (8 << timer_No);
    REG_IME = 1;

    timeoutFlag = 0;
    timer_Count = *maxTime++;
    *timerReg++ = *maxTime++;
    *timerReg-- = *maxTime++;
}

void StopEepromTimer(void) {
    *timerReg++ = 0;
    *timerReg-- = 0;

    REG_IME = 0;
    REG_IE &= ~(8 << timer_No);
    REG_IME = shelt_ime;
}

void Dma3Transmit(void *src, void *dest, u16 size) {
    u16 ime = REG_IME;
    REG_IME = 0;
    REG_WAITCNT = (REG_WAITCNT & ~(WAITCNT_WS2_N_MASK | WAITCNT_WS2_S_MASK)) | gEEPROMConfig->waitcnt;
    REG_DMA3SAD = (u32)src;
    REG_DMA3DAD = (u32)dest;
    REG_DMA3CNT = (DMA_ENABLE << 0x10) | size;
    while (REG_DMA3CNT_H & DMA_ENABLE)
        ;
    REG_IME = ime;
}

u16 ReadEepromDword(u16 address, u16 *data) {
    u16 buffer[0x44];
    u16 *ptr;
    u8 *p;
    u8 t1;
    u8 t2;
    u16 value;

    if (address >= gEEPROMConfig->size)
        return EEPROM_OUT_OF_RANGE;

    ptr = buffer;
    // setup address
    p = (u8 *)ptr;
    p += (gEEPROMConfig->address_width << 1) + 1;
    ptr = (u16 *)p++;
    ptr = (u16 *)p++;
    for (t1 = 0; t1 < gEEPROMConfig->address_width; t1++) {
        *(ptr--) = address;
        address >>= 1;
    }

    // read request
    *(ptr--) = 1;
    *ptr = 1;
    Dma3Transmit(&buffer, REG_ADDR_EEPROM, (gEEPROMConfig->address_width + 3));
    Dma3Transmit(REG_ADDR_EEPROM, &buffer, 0x44);

    // 4 bit junk
    ptr = buffer + 4;
    data += 3;

    // copy data into output buffer
    for (t1 = 0; t1 < 4; t1++) {
        value = 0;
        for (t2 = 0; t2 < 0x10; t2++) {
            value <<= 1;
            value |= (*ptr++) & 1;
        }
        *(data--) = value;
    }

    return 0;
}

u16 ProgramEepromDword(u16 address, const u16 *data) {
    u16 buffer[0x52]; // this is one too large?
    u16 *ptr;
    u8 i;
    u8 j;
    u16 retval;

    if (address >= gEEPROMConfig->size)
        return EEPROM_OUT_OF_RANGE;

    // ugly ptr math required for OK
    ptr = (u16 *)(0x42 + (u32)&buffer + (u32)(gEEPROMConfig->address_width << 1) + 0x42);
    *(ptr)-- = 0;

    // copy data into buffer
    for (i = 0; i < 4; i++) {
        u16 r2 = *data++;
        for (j = 0; j < 16; j++) {
            *ptr-- = r2;
            r2 = r2 >> 1;
        }
    }

    // copy address to buffer
    for (i = 0; i < gEEPROMConfig->address_width; i++) {
        *ptr-- = address;
        address = address >> 1;
    }
    *ptr-- = 0;
    *ptr-- = 1;

    Dma3Transmit(&buffer, REG_ADDR_EEPROM, (gEEPROMConfig->address_width + 0x43));
    StartEepromTimer(gEepromMaxTimers);

    retval = 0;

    while (1) {
        if (REG_EEPROM & 1) {
            break;
        }

        if (timeoutFlag) {
            if (!(REG_EEPROM & 1)) {
                retval = EEPROM_WRITE_FAIL;
            }
            break;
        }
    }

    StopEepromTimer();
    return retval;
}

u16 VerifyEepromDword(u16 address, u16 *data) {
    u16 buffer[4];
    u8 i;
    u16 *ptr;
    u16 retvar = 0;

    if (address >= gEEPROMConfig->size)
        return EEPROM_OUT_OF_RANGE;

    ReadEepromDword(address, buffer);
    ptr = buffer;
    for (i = 0; i < 4; i++) {
        if (*data++ != *ptr++) {
            retvar = EEPROM_VERIFY_FAIL;
            break;
        }
    }

    return retvar;
}

u16 ProgramEepromDwordEx(u16 address, u16 *data) {
    u16 retvar;
    u8 i;

    for (i = 0; i < 3; i++) {
        retvar = ProgramEepromDword(address, data);
        if (retvar == 0) {
            retvar = VerifyEepromDword(address, data);
            if (retvar == 0)
                break;
        }
    }

    return retvar;
}
