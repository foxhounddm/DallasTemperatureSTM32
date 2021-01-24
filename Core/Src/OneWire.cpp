//
// Created by dmitriy on 23.01.2021.
//

#include "OneWire.h"

uint8_t OneWire::reset() {
    uint8_t resetCmd = 0xF0;
    uint8_t result;
    initUART(ONE_WIRE_RESET_BAUD);
    uint32_t tickstart = HAL_GetTick();
    while (HAL_GetTick() - tickstart < ONE_WIRE_RESET_DELAY_TICK_COUNT) {

    }
    HAL_UART_Transmit(_huartPtr, &resetCmd, 1, 1);
    HAL_UART_Receive(_huartPtr, &result, 1, 1);
    initUART(ONE_WIRE_DEFAULT_BAUD);

    tickstart = HAL_GetTick();
    while (HAL_GetTick() - tickstart < ONE_WIRE_RESET_DELAY_TICK_COUNT) {

    }
    return (result != resetCmd) ? 1 : 0;
}

uint8_t OneWire::isBusy() const {
    return (_receiveFlag < 2) || (_sendFlag < 2) || (_uartFinishFlag < 1);
}

uint8_t OneWire::searchCommand() {
    while (isBusy()) {
    }

    uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    for (int i = 0; i < 10; i++) _sendBuffer[i] = tmp[i];

    reset();
    sendAndReceiveBits(10);

    while (isBusy()) {
    }

    return decodeBit(_receiveBuffer[8], _receiveBuffer[9]);
}

uint8_t OneWire::searchNextBit(uint8_t currentBit) {
    while (isBusy()) {
    }
    _sendBuffer[0] = (currentBit == 1) ? 0xFF : 0x00;
    _sendBuffer[1] = 0xFF;
    _sendBuffer[2] = 0xFF;

    sendAndReceiveBits(3);

    while (isBusy()) {
    }

    return decodeBit(_receiveBuffer[1], _receiveBuffer[2]);
}

uint8_t OneWire::searchDevices(uint64_t* address, int8_t index) {


    int8_t globalSelectPoint = -1;
    uint64_t prevAddr = 0;
    int8_t lastOneSelect;
    uint8_t deviceCount = 0;

    do {
        lastOneSelect = -1;
        uint64_t mask = 0x01;
        uint64_t currentAddr = 0;
        uint8_t bit = searchCommand();
        int8_t bitCount = 0;

        do {
            if (bit == 2) {
                if (bitCount > globalSelectPoint) {
                    currentAddr = currentAddr | 0x8000000000000000;
                    bit = searchNextBit(1);
                    lastOneSelect = bitCount;
                } else if (bitCount == globalSelectPoint) {
                    bit = searchNextBit(0);
                } else {
                    if ((prevAddr & mask) > 0) {
                        currentAddr = currentAddr | 0x8000000000000000;
                        bit = searchNextBit(1);
                        lastOneSelect = bitCount;
                    } else {
                        bit = searchNextBit(0);
                    }
                }

            } else {
                if (bit == 1) {
                    currentAddr = currentAddr | 0x8000000000000000;
                }
                bit = searchNextBit(bit);
            }
            if (bitCount < 63) {
                currentAddr = currentAddr >> 1;
                mask = mask << 1;
            }

            bitCount++;

        } while (bitCount < 64);

        prevAddr = currentAddr;
        if(index>=0&& deviceCount==index) {
            *address = currentAddr;
            return 0;
        }
        deviceCount++;
        globalSelectPoint = lastOneSelect;


    } while (lastOneSelect >= 0);

    return deviceCount;

}

OneWire::OneWire(UART_HandleTypeDef *huartPtr) {
    _huartPtr = huartPtr;
    _receiveFlag = 2;
    _sendFlag = 2;
}

void OneWire::receiveHalfBuffer() {
    _receiveFlag++;
}

void OneWire::sendHalfBuffer() {
    _sendFlag++;
}

void OneWire::sendAndReceiveBits(uint8_t size) {
    _receiveFlag = 0;
    _sendFlag = 0;
    HAL_UART_Transmit_DMA(_huartPtr, _sendBuffer, size);
    HAL_UART_Receive_DMA(_huartPtr, _receiveBuffer, size);

}

uint8_t OneWire::decodeBit(uint8_t bitA, uint8_t bitB) {
    uint8_t res;
    if (bitA == 0xFF && bitB != 0xFF) {
        res = 1;
    } else if (bitA != 0xFF && bitB == 0xFF) {
        res = 0;
    } else {
        res = 2;
    }
    return res;
}

void OneWire::initUART(uint32_t baud) {
    _huartPtr->Init.BaudRate = baud;
    if (HAL_UART_Init(_huartPtr) != HAL_OK) {
        Error_Handler();
    }
}

void OneWire::sendCommand(uint8_t *command, uint8_t size) {

    while (isBusy()) {
    }

    convertToOneWireCode(command, _sendBuffer, size);

    reset();
    _sendFlag = 0;
    _uartFinishFlag = 0;
    HAL_UART_Transmit_DMA(_huartPtr, _sendBuffer, size * 8);
}

void OneWire::sendCommandAndReceiveData(uint8_t *command, uint8_t cmdSize, uint8_t resultSize) {
    while (isBusy()) {
    }

    convertToOneWireCode(command, _sendBuffer, cmdSize);
    const uint8_t sendSize = (cmdSize + resultSize) * 8;
    for (uint8_t i = cmdSize * 8; i < sendSize; i++) _sendBuffer[i] = 0xFF;

    reset();
    _resultOffset = cmdSize * 8;
    sendAndReceiveBits(sendSize);

}

void OneWire::convertToOneWireCode(uint8_t *command, uint8_t *oneWireCmd, uint8_t size) {

    uint8_t *buf = oneWireCmd;
    for (uint8_t i = 0; i < size; i++) {
        uint8_t mask = 0x01;
        for (uint8_t j = 0; j < 8; j++) {
            *buf = (command[i] & mask) ? 0xFF : 0x00;
            buf++;
            mask = mask << 1;
        }
    }

}

void OneWire::getData(uint8_t *result, uint8_t size) {

    uint8_t *dataPrt = _receiveBuffer + _resultOffset;
    for (uint8_t i = 0; i < size; i++) {
        result[i] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            if (*dataPrt == 0xFF) {
                result[i] = result[i] | 0x80;
            }
            if (j < 7) result[i] = result[i] >> 1;
            dataPrt++;
        }
    }

}

void OneWire::uartFinish() {
    _uartFinishFlag = 1;

}

uint8_t OneWire::getDeviceCount()  {
    return searchDevices(nullptr,-1);
}


uint64_t OneWire::getDeviceAddress(uint8_t index) {
    uint64_t res;
    searchDevices(&res,index);
    return res;
}
