//
// Created by dmitriy on 23.01.2021.
//

#ifndef TESTH_ONEWIRE_H
#define TESTH_ONEWIRE_H
#include "usart.h"

#define ONE_WIRE_BUFFER_SIZE 255
#define ONE_WIRE_DEFAULT_BAUD 115200
#define ONE_WIRE_RESET_BAUD 9600
#define ONE_WIRE_RESET_DELAY_TICK_COUNT 1

class OneWire{
    uint8_t _sendBuffer[ONE_WIRE_BUFFER_SIZE];
    uint8_t _receiveBuffer[ONE_WIRE_BUFFER_SIZE];
    uint8_t _receiveFlag = 2;
    uint8_t _sendFlag = 2;
    uint8_t _uartFinishFlag = 1;
    uint8_t  _resultOffset=0;
    UART_HandleTypeDef* _huartPtr;

    void sendAndReceiveBits(uint8_t size);
    uint8_t searchCommand();
    uint8_t searchNextBit(uint8_t currentBit);
    uint8_t decodeBit( uint8_t bitA, uint8_t bitB);
    void initUART( uint32_t baud);
    uint8_t searchDevices( uint64_t* address, int8_t index );
public:

    explicit OneWire(UART_HandleTypeDef* huartPtr);

    void receiveHalfBuffer();
    void sendHalfBuffer();
    void uartFinish();

    uint8_t reset();
    uint8_t isBusy() const;


    void sendCommand(uint8_t *command, uint8_t size);
    void sendCommandAndReceiveData(uint8_t *command, uint8_t cmdSize, uint8_t resultSize);
    void convertToOneWireCode(uint8_t *command, uint8_t *oneWireCmd, uint8_t size);
    void getData(uint8_t *result, uint8_t size);
    uint8_t getDeviceCount();
    uint64_t getDeviceAddress(uint8_t index);
};


#endif //TESTH_ONEWIRE_H
