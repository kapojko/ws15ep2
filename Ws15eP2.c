#include "MinUnit.h"
#include "Ws15eP2.h"

#define WS15EP2_CMD_DRIVER_OUTPUT 0x01
#define WS15EP2_CMD_GATE_DRIVING_VOLTAGE 0x03
#define WS15EP2_CMD_SOURCE_DRIVING_VOLTAGE 0x04
#define WS15EP2_CMD_DEEP_SLEEP_MODE 0x10
#define WS15EP2_CMD_DATA_ENTRY_MODE 0x11
#define WS15EP2_CMD_SW_RESET 0x12
#define WS15EP2_CMD_MASTER_ACTIVATION 0x20
#define WS15EP2_CMD_DISPLAY_UPDATE_CONTROL_1 0x21
#define WS15EP2_CMD_DISPLAY_UPDATE_CONTROL_2 0x22
#define WS15EP2_CMD_WRITE_RAM_BW 0x24
#define WS15EP2_CMD_WRITE_RAM_RED 0x26
#define WS15EP2_CMD_VCOM_SENSE 0x28
#define WS15EP2_CMD_PROGRAM_VCOM_OTP 0x2A
#define WS15EP2_CMD_VCOM_CONTROL 0x2B
#define WS15EP2_CMD_VCOM_WRITE 0x2C
#define WS15EP2_CMD_READ_DISPLAY_OPTION_OTP 0x2D
#define WS15EP2_CMD_READ_USER_ID 0x2E
#define WS15EP2_CMD_PROGRAM_WS_OTP 0x30
#define WS15EP2_CMD_LOAD_WS_OTP 0x31
#define WS15EP2_CMD_WRITE_LUT 0x32
#define WS15EP2_CMD_PROGRAM_OTP_SELECTION 0x36
#define WS15EP2_CMD_WRITE_USER_ID 0x38
#define WS15EP2_CMD_OTP_PROGRAM_MODE 0x39
#define WS15EP2_CMD_SET_RAM_X 0x44
#define WS15EP2_CMD_SET_RAM_Y 0x45
#define WS15EP2_CMD_SET_RAM_X_COUNTER 0x4E
#define WS15EP2_CMD_SET_RAM_Y_COUNTER 0x4F

#define CHECK_X(x8x, width8x) (x8x >= 0 && width8x > 0 && x8x + width8x <= WS15EP2_WIDTH / 8 && x8x % 8 == 0 && width8x % 8 == 0)
#define CHECK_Y(y, height) (y >= 0 && height > 0 && y + height <= WS15EP2_HEIGHT)
#define DATA_SIZE(width8x, height) (width8x / 8 * height)

static struct Ws15eP2_Platform *platform;

// see https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd1in54_V2/epd1in54_V2.cpp
// the same values also in https://github.com/waveshareteam/e-Paper/blob/master/STM32/STM32-F103ZET6/User/e-Paper/EPD_1in54_V2.c
// it could be not that since the voltage config is in last bytes
uint8_t Ws15eP2_WF_Full_1IN54[159] = {
    0x80,	0x48,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x40,	0x48,	0x80,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x80,	0x48,	0x40,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x40,	0x48,	0x80,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0xA,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x8,	0x1,	0x0,	0x8,	0x1,	0x0,	0x2,
    0xA,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	0x0,	0x0,	0x0,
    0x22,	0x17,	0x41,	0x0,	0x32,	0x20
};

// waveform partial refresh(fast)
// see https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd1in54_V2/epd1in54_V2.cpp
// the same values also in https://github.com/waveshareteam/e-Paper/blob/master/STM32/STM32-F103ZET6/User/e-Paper/EPD_1in54_V2.c
// it could be not that since the voltage config is in last bytes
uint8_t Ws15eP2_WF_PARTIAL_1IN54_0[159] = {
    0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0xF,0x0,0x0,0x0,0x0,0x0,0x0,
    0x1,0x1,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
    0x02,0x17,0x41,0xB0,0x32,0x28,
};

static bool sendCommand(uint8_t command) {
    int res;

    // Check busy
    if (platform->gpioGet(platform->pinBusy)) {
        platform->debugPrint("sendCommand failed, busy\r\n");
        return false;
    }

    // Select command using DC
    platform->gpioSet(platform->pinDc, 0);

    // Send data with CS
    res = platform->spiSendWithCs(&command, 1, false);

    if (res != 0) {
        platform->debugPrint("sendCommand failed, spiSendWithCs returned %d\r\n", res);
        return false;
    }

    return true;
}

static bool sendData(const uint8_t *data, int len) {
    int res;

    // Check busy
    if (platform->gpioGet(platform->pinBusy)) {
        platform->debugPrint("sendData failed, busy\r\n");
        return false;
    }

    // Select data using DC
    platform->gpioSet(platform->pinDc, 1);

    // Send data with CS
    res = platform->spiSendWithCs(data, len, false);

    if (res != 0) {
        platform->debugPrint("sendData failed, spiSendWithCs returned %d\r\n", res);
        return false;
    }

    return true;
}

static bool readCommandData(uint8_t command, uint8_t *data, int len) {
    int res = 0;

    // Check busy
    if (platform->gpioGet(platform->pinBusy)) {
        platform->debugPrint("readCommandData failed, busy\r\n");
        return false;
    }

    // Begin SPI transaction
    platform->spiBegin();

    // Select command using DC
    platform->gpioSet(platform->pinDc, 0);

    // Write command (keep CS active after that)
    res |= platform->spiSendWithCs(&command, 1, true);

    // Select data using DC
    platform->gpioSet(platform->pinDc, 1);

    // Receive data using CS (dont't keep CS active)
    res |= platform->spiRecvWithCs(data, len, false);

    // End SPI transaction
    platform->spiEnd();

    if (res != 0) {
        platform->debugPrint("readCommandData failed, spiRecvWithCs returned %d\r\n", res);
        return false;
    }

    return true;
}

static uint16_t convertTemperature(int tempDegC) {
    // Apply LSB
    int16_t tempSigned = tempDegC * 16;

    // Convert to 2's supplement (12 bits only)
    uint16_t temp = (*(uint16_t *)&tempSigned) & 0xFFF;

    return temp;
}

void Ws15eP2_Init(struct Ws15eP2_Platform *platformPtr) {
    platform = platformPtr;
}

bool Ws15eP2_SendGenericCommand(uint8_t command, const uint8_t *data, int len) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(command);

    // send data (if provided)
    if (data) {
        ok = ok & sendData(data, len);
    }

    return ok;
}

bool Ws15eP2_SetDriverOutput(int muxGateLines, int firstGate, int scanningOrder, int topBottom) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_DRIVER_OUTPUT);

    // send command data
    // NOTE: datasheet states that DC# bit should be 1 only for first data byte, this must be an error
    uint8_t data[3] = {
        (muxGateLines - 1) & 0xFF,
        ((muxGateLines - 1) >> 8) & 0b1,
        ((firstGate & 0b1) << 2) | ((scanningOrder & 0b1) << 1) | (topBottom & 0b1),
    };
    ok = ok & sendData(data, sizeof(data));

    return ok;
}

bool Ws15eP2_DeepSleep(void) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_DEEP_SLEEP_MODE);

    // send command data
    uint8_t data = 0x01; // Enter Deep Sleep Mode 1
    ok = ok & sendData(&data, sizeof(data));

    // NOTE from datasheet: After this command initiated, the chip will
    // enter Deep Sleep Mode, BUSY pad will keep
    // output high.
    // Remark:
    // To Exit Deep Sleep mode, User required to
    // send HWRESET to the driver

    // NOTE: the following is from Arduino example
    // platform->delayMs(200);
    // platform->gpioSet(platform->pinRst, 0);

    return ok;
}

bool Ws15eP2_SetDataEntryMode(int dataEntryMode, int dataDirection) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_DATA_ENTRY_MODE);

    // send command data
    uint8_t data = (dataEntryMode & 0b11) | ((dataDirection & 0b1) << 2);
    ok = ok & sendData(&data, sizeof(data));

    return ok;
}

bool Ws15eP2_SoftwareReset(void) {
    bool ok = true;

    ok = ok & sendCommand(WS15EP2_CMD_SW_RESET);
    Ws15eP2_WaitBusy();

    return ok;
}

bool Ws15eP2_SetTemperature(int tempDegC) {
    bool ok = true;

    // Convert temperature
    uint16_t temp = convertTemperature(tempDegC);

    // Send command
    // NOTE: this command is mentioned in datasheet but not described in command reference table
    ok = ok & sendCommand(0x1A);

    // Send command data
    uint8_t data[2] = {
        (temp & 0xFF),
        ((temp >> 8) & 0xFF),
    };
    ok = ok & sendData(data, sizeof(data));

    return ok; 
}

bool Ws15eP2_MasterActivation(void) {
    bool ok = true;

    ok = ok & sendCommand(WS15EP2_CMD_MASTER_ACTIVATION);
    Ws15eP2_WaitBusy();

    return ok;
}

bool Ws15eP2_SetRamContentOption(int redRamOption, int bwRamOption) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_DISPLAY_UPDATE_CONTROL_1);

    // send command data
    uint8_t data[2] = {
        ((redRamOption & 0b1111) << 4) | ((bwRamOption & 0b1111) << 0),
        0
    };
    ok = ok & sendData(data, sizeof(data));

    return ok;
}

bool Ws15eP2_SetDisplayUpdateSequence(int displayUpdateSequence) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_DISPLAY_UPDATE_CONTROL_2);

    // send command data
    uint8_t data = displayUpdateSequence;
    ok = ok & sendData(&data, sizeof(data));

    return ok;
}

bool Ws15eP2_WriteRAMbw(const uint8_t *data, int len) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_WRITE_RAM_BW);

    // send data
    ok = ok & sendData(data, len);

    return ok;
}

bool Ws15eP2_WriteRAMred(const uint8_t *data, int len) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_WRITE_RAM_RED);

    // send data
    ok = ok & sendData(data, len);

    return ok;
}

bool Ws15eP2_ReadUserID(uint8_t *data10bytes) {
    bool ok = true;

    ok &= readCommandData(WS15EP2_CMD_READ_USER_ID, data10bytes, 10);

    return ok;
}

bool Ws15eP2_SetLutAndVoltage(const uint8_t *lut) {
    bool ok = true;

    // NOTE: taken from https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd1in54_V2/epd1in54_V2.cpp

    // Write LUT (153 bytes)
    ok = ok & sendCommand(WS15EP2_CMD_WRITE_LUT);
    ok = ok & sendData(lut, 153);

    Ws15eP2_WaitBusy();

    // NOTE: command 0x3f is not given in datasheet but persist in example
    ok = ok & sendCommand(0x3f);
    ok = ok & sendData(&lut[153], 1);

    // Setup gate driving voltage
    ok = ok & sendCommand(WS15EP2_CMD_GATE_DRIVING_VOLTAGE);
    ok = ok & sendData(&lut[154], 1);

    // Setup source driving voltage
    ok = ok & sendCommand(WS15EP2_CMD_SOURCE_DRIVING_VOLTAGE);
    ok = ok & sendData(&lut[155], 3);

    // Write VCOM
    ok = ok & sendCommand(WS15EP2_CMD_VCOM_WRITE);
    ok = ok & sendData(&lut[158], 1);

    return ok;
}

bool Ws15eP2_WriteUserID(const uint8_t *data10bytes) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_WRITE_USER_ID);

    // send data
    ok = ok & sendData(data10bytes, 10);

    return ok;
}

bool Ws15eP2_SetRAMxAddress(int xStart, int xEnd) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_SET_RAM_X);

    // send command data
    uint8_t data[2] = {
        xStart & 077,
        xEnd & 077
        
    };
    ok = ok & sendData(data, sizeof(data));

    return ok;
}

bool Ws15eP2_SetRAMyAddress(int yStart, int yEnd) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_SET_RAM_Y);

    // send command data
    uint8_t data[4] = {
        yStart & 0xFF,
        (yStart >> 8) & 0b1,
        yEnd & 0xFF,
        (yEnd >> 8) & 0b1
    };
    ok = ok & sendData(data, sizeof(data));

    return ok;
}

bool Ws15eP2_SetRAMxCounter(int xCounter) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_SET_RAM_X_COUNTER);

    // send command data
    uint8_t data = xCounter & 077;
    ok = ok & sendData(&data, sizeof(data));

    return ok;
}

bool Ws15eP2_SetRAMyCounter(int yCounter) {
    bool ok = true;

    // send command
    ok = ok & sendCommand(WS15EP2_CMD_SET_RAM_Y_COUNTER);

    // send command data
    uint8_t data[2] = {
        yCounter & 0xFF,
        (yCounter >> 8) & 0b1
    };
    ok = ok & sendData(data, sizeof(data));

    return ok;
}

void Ws15eP2_HardwareReset(void) {
    // NOTE:  module reset. often used to awaken the module in deep sleep
    // NOTE: timings in example: 20, 5, 20
    // timings in datasheet: 10, 10, 10

    platform->gpioSet(platform->pinRst, 1);
    platform->delayMs(10);

    platform->gpioSet(platform->pinRst, 0); // module reset
    platform->delayMs(10);

    platform->gpioSet(platform->pinRst, 1);
    platform->delayMs(10);

    Ws15eP2_WaitBusy();
}

void Ws15eP2_WaitBusy(void) {
    // NOTE: timings taken from Arduino example

    while (platform->gpioGet(platform->pinBusy) != 0) {
        platform->delayMs(100);
    }

    platform->delayMs(200);
}

bool Ws15eP2_DefInit(int tempDegC, bool tempValid) {
    // NOTE: datasheet reference code
    
    bool ok = true;

    // Perform hardware reset
    Ws15eP2_HardwareReset();

    // Software reset
    ok = ok & Ws15eP2_SoftwareReset();

    // Driver output control
    ok = ok & Ws15eP2_SetDriverOutput(WS15EP2_MUX_GATE_LINES_DEF, WS15EP2_GATE_DEF, WS15EP2_SCAN_DEF,
        WS15EP2_TOP_BOTTOM);
    
    // Data entry mode
    ok = ok & Ws15eP2_SetDataEntryMode(WS15EP2_DATA_ENTRY_YDEC_XINC, WS15EP2_DATA_DIRECTION_X);

    // RAM x address start/end position
    ok = ok & Ws15eP2_SetRAMxAddress(0, (200 / 8) - 1);

    // RAM y address start/end position
    ok = ok & Ws15eP2_SetRAMyAddress(199, 0);

    // Set temperature
    if (tempValid) {
        ok = ok & Ws15eP2_SetTemperature(tempDegC);
    }

    // Load LUT from OTP
    // TODO: should we use WS15EP2_UPD_SEQ_EC_L1_DC if temperature not given?
    ok = ok & Ws15eP2_SetDisplayUpdateSequence(WS15EP2_UPD_SEQ_EC_LT_L1_DC);

    // Master activation
    ok = ok & Ws15eP2_MasterActivation();

    return ok;
}

bool Ws15eP2_DefInitFull() {
    // NOTE: from Arduino example

    bool ok = true;

    // Perform hardware reset
    Ws15eP2_HardwareReset();

    // Software reset
    ok = ok & Ws15eP2_SoftwareReset();

    // Driver output control
    ok = ok & Ws15eP2_SetDriverOutput(WS15EP2_MUX_GATE_LINES_DEF, WS15EP2_GATE_DEF, WS15EP2_SCAN_DEF, WS15EP2_TOP_BOTTOM_REVERSE);

    // Data entry mode
    ok = ok & Ws15eP2_SetDataEntryMode(WS15EP2_DATA_ENTRY_YDEC_XINC, WS15EP2_DATA_DIRECTION_X);

    // RAM x address start/end position (in address units of 8 bits)
    ok = ok & Ws15eP2_SetRAMxAddress(0, (200 / 8) - 1);
    
    // RAM y address start/end position
    ok = ok & Ws15eP2_SetRAMyAddress(199, 0);

    // Set BorderWavefrom
    // NOTE: the command missed in the datasheet but occurs in Arduino example
    uint8_t borderWaveformData = 0x01;
    ok = ok & Ws15eP2_SendGenericCommand(0x3C, &borderWaveformData, sizeof(borderWaveformData));

    // Command 0x18
    // NOTE: the command missed in the datasheet but occurs in Arduino example
    uint8_t command18Data = 0x80;
    ok = ok & Ws15eP2_SendGenericCommand(0x18, &command18Data, sizeof(command18Data));

    // Setup display update sequence - load Temperature and waveform setting
    // TODO: should we actually pass the temperature value? It is missed in the example
    ok = ok & Ws15eP2_SetDisplayUpdateSequence(WS15EP2_UPD_SEQ_EC_LT_L1_DC);

    // set RAM x address count to 0
    ok = ok & Ws15eP2_SetRAMxCounter(0);

    // set RAM y address count to 199
    ok = ok & Ws15eP2_SetRAMyCounter(199);

    Ws15eP2_WaitBusy();

    // Set LUT
    ok = ok & Ws15eP2_SetLutAndVoltage(Ws15eP2_WF_Full_1IN54);

    // Hardware initialized
    return ok;
}

bool Ws15eP2_DefInitFullPartial() {
    // NOTE: from Arduino example

    bool ok = true;

    // Perform hardware reset
    Ws15eP2_HardwareReset();

    // Software reset
    ok = ok & Ws15eP2_SoftwareReset();

    // Set LUT for partial display
    ok = ok & Ws15eP2_SetLutAndVoltage(Ws15eP2_WF_PARTIAL_1IN54_0);

    // Send command 0x37
    uint8_t command37Data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00};
    ok = ok & Ws15eP2_SendGenericCommand(0x37, command37Data, sizeof(command37Data));

    // Set BorderWavefrom
    // NOTE: the command missed in the datasheet but occurs in Arduino example
    uint8_t borderWaveformData = 0x80;
    ok = ok & Ws15eP2_SendGenericCommand(0x3C, &borderWaveformData, sizeof(borderWaveformData));

    // Setup display update sequence - just enable clock and analog
    ok = ok & Ws15eP2_SetDisplayUpdateSequence(WS15EP2_UPD_SEQ_EC_EA);

    // Master activation
    ok = ok & Ws15eP2_MasterActivation();

    return ok;
}

bool Ws15eP2_RefreshDisplay(void) {
    bool ok = true;

    // Setup display update sequence
    ok = ok & Ws15eP2_SetDisplayUpdateSequence(WS15EP2_UPD_SEQ_EC_EA_D1_DA_DO);

    // Master activation
    ok = ok & Ws15eP2_MasterActivation();

    return ok;
}

bool Ws15eP2_ClearScreen(bool clearBW, bool clearRed) {
    bool ok = true;

    int widthBytes = WS15EP2_WIDTH / 8;
    int heightBytes = WS15EP2_HEIGHT;
    uint8_t data = 0xFF;

    // Set RAM start/end address
    ok = ok & Ws15eP2_SetRAMxAddress(0, widthBytes - 1);
    ok = ok & Ws15eP2_SetRAMyAddress(heightBytes - 1, 0);

    // Set RAM counter
    ok = ok & Ws15eP2_SetRAMxCounter(0);
    ok = ok & Ws15eP2_SetRAMyCounter(heightBytes - 1);

    // Clear BW RAM (write ones)
    if (clearBW) {
        ok = ok & sendCommand(WS15EP2_CMD_WRITE_RAM_BW);
        for (int y = 0; y < heightBytes; y++) {
            for (int x = 0; x < widthBytes; x++) {
                ok = ok & sendData(&data, sizeof(data));
            }
        }
    }

    // Clear RED RAM (write ones)
    if (clearRed) {
        ok = ok & sendCommand(WS15EP2_CMD_WRITE_RAM_RED);
        for (int y = 0; y < heightBytes; y++) {
            for (int x = 0; x < widthBytes; x++) {
                ok = ok & sendData(&data, sizeof(data));
            }
        }
    }

    // Refresh display
    ok = ok & Ws15eP2_MasterActivation();

    return ok;
}

int Ws15eP2_OutputBitmap(int x8x, int y, int width8x, int height, const uint8_t *data, int dataSize) {
    bool ok = true;

    // Check x and y
    if (!CHECK_X(x8x, width8x)) {
        return WS15EP2_ERR_INVALID_X;
    }

    if (!CHECK_Y(y, height)) {
        return WS15EP2_ERR_INVALID_Y;
    }

    // Check data size
    if (dataSize != DATA_SIZE(width8x, height)) {
        return WS15EP2_ERR_INVALID_DATA_SIZE;
    }

    // Set start/end position
    ok = ok & Ws15eP2_SetRAMxAddress(x8x / 8, (x8x + width8x) / 8 - 1);
    ok = ok & Ws15eP2_SetRAMyAddress(y + height - 1, y);

    // Set address counters
    ok = ok & Ws15eP2_SetRAMxCounter(x8x / 8);
    ok = ok & Ws15eP2_SetRAMyCounter(y + height - 1);

    // Write BW RAM
    ok = ok & Ws15eP2_WriteRAMbw(data, dataSize);

    return ok ? 0 : WS15EP2_ERR_CMD_ERROR;
}

int Ws15eP2_FillArea(int x8x, int y, int width8x, int height, uint8_t value) {
    bool ok = true;

    // Check x and y
    if (!CHECK_X(x8x, width8x)) {
        return WS15EP2_ERR_INVALID_X;
    }

    if (!CHECK_Y(y, height)) {
        return WS15EP2_ERR_INVALID_Y;
    }

    // Set start/end position
    ok = ok & Ws15eP2_SetRAMxAddress(x8x / 8, (x8x + width8x) / 8 - 1);
    ok = ok & Ws15eP2_SetRAMyAddress(y + height - 1, y);

    // Set address counters
    ok = ok & Ws15eP2_SetRAMxCounter(x8x / 8);
    ok = ok & Ws15eP2_SetRAMyCounter(y + height - 1);

    // Write BW RAM
    ok = ok & sendCommand(WS15EP2_CMD_WRITE_RAM_BW);

    int dataSize = DATA_SIZE(width8x, height);
    for (int i = 0; i < dataSize; i++) {
        ok = ok & sendData(&value, sizeof(value));
    }

    return ok ? 0 : WS15EP2_ERR_CMD_ERROR;
}

const char *Ws15eP2_UnitTest(void) {
    // Test temperature conversion
    mu_assert("convertTemperature(127) == 0x7F0", convertTemperature(127) == 0x7F0);
    mu_assert("convertTemperature(25) == 0x190", convertTemperature(25) == 0x190);
    mu_assert("convertTemperature(0) == 0x0", convertTemperature(0) == 0x0);
    mu_assert("convertTemperature(-25) == 0xE70", convertTemperature(-25) == 0xE70);
    mu_assert("convertTemperature(-55) == 0xC90", convertTemperature(-55) == 0xC90);

    return 0;
}
