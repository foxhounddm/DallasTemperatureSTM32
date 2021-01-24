//
// Created by dmitriy on 24.01.2021.
//

#include "DallasTemperature.h"

DallasTemperature::DallasTemperature(OneWire *oneWire) : _oneWire(oneWire) {}

uint8_t DallasTemperature::isTemperatureSensor(uint64_t *addr) {
    //((family_code == 0x28) || (family_code == 0x22) || (family_code == 0x10))
    uint64_t fCode = *addr & 0xFF;
    if (fCode == 0x28 || fCode == 0x22 || fCode == 0x10) {
        return 1;
    } else {
        return 0;
    }
}

void DallasTemperature::requestTemperatures() {
    uint8_t requestCmd[] = {0xCC, 0x44};
    _oneWire->sendCommand(requestCmd, 2);

}

void DallasTemperature::requestTemperaturesByAddress(const uint64_t *addr) {
    uint8_t requestCmd[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44};
    uint8_t* addrPtr = (uint8_t*)addr;
    for(uint8_t i=0; i<8; i++) requestCmd[i+1] = addrPtr[i];
    _oneWire->sendCommand(requestCmd, 10);
}

void DallasTemperature::requestTemperaturesByIndex(uint8_t index) {
    uint64_t addr = _oneWire->getDeviceAddress(index);
    requestTemperaturesByAddress(&addr);
}

float DallasTemperature::getTempC(uint64_t *addr) {
    uint8_t result[9];
    getScratchpad(addr, result);

    float temp = ((result[0] >> 4) & 0x0F) | ((result[1] & 0x00000111) << 4);
    temp = temp * ( ((result[1]&0b11110000)==0)? 1:-1 );
    if( result[0]&0b00001000) temp+=0.5;
    if( result[0]&0b00000100) temp+=0.25;
    if( result[0]&0b00000010) temp+=0.125;
    if( result[0]&0b00000001) temp+=0.0625;

    return temp;
}

float DallasTemperature::getTempCByIndex(uint8_t index) {
    uint64_t addr = _oneWire->getDeviceAddress(index);
    return getTempC(&addr);
}

void DallasTemperature::setResolution(uint8_t resolution) {
    uint8_t devCount = _oneWire->getDeviceCount();
    for( uint8_t i=0; i<devCount; i++)
        setResolution(resolution,i);

}

void DallasTemperature::setResolution(uint8_t resolution, uint8_t deviceIndex) {
    uint64_t  addr = _oneWire->getDeviceAddress(deviceIndex);
    if( isTemperatureSensor(&addr)) {
        setResolution( resolution, &addr);
    }

}

void DallasTemperature::setResolution(uint8_t resolution, uint64_t* deviceAddr) {
    uint8_t data[9];
    getScratchpad( deviceAddr, data);

    uint8_t writeCmd[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x00};
    uint8_t* addrPtr = (uint8_t*)deviceAddr;
    for(uint8_t i=0; i<8; i++) writeCmd[i+1] = addrPtr[i];
    writeCmd[10] = data[2];
    writeCmd[11] = data[3];
    writeCmd[12] = ((resolution-9)<<5)|0b00011111;

    _oneWire->sendCommand(writeCmd, 13);


}

uint8_t DallasTemperature::getResolution( uint8_t index) {
    uint64_t addr = _oneWire->getDeviceAddress(index);
    uint8_t  res =0;
    if( isTemperatureSensor(&addr)) res = getResolution(&addr);
    return res;
}

uint8_t DallasTemperature::getResolution( uint64_t *addr) {
    uint8_t data[9];
    getScratchpad(addr, data);

    return ((data[4]&0b01100000)>>5)+9;

}

void DallasTemperature::getScratchpad(uint64_t *addr, uint8_t* data) {
    uint8_t requestCmd[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBE};

    uint8_t* addrPtr = (uint8_t*)addr;
    for(uint8_t i=0; i<8; i++) requestCmd[i+1] = addrPtr[i];
    _oneWire->sendCommandAndReceiveData(requestCmd, 10,9);
    while ( _oneWire->isBusy()) {

    }
    _oneWire->getData(data,9);
}

void DallasTemperature::copyScratchpadToEE(uint64_t *addr) {
    uint8_t requestCmd[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48};
    uint8_t* addrPtr = (uint8_t*)addr;
    for(uint8_t i=0; i<8; i++) requestCmd[i+1] = addrPtr[i];
    _oneWire->sendCommand(requestCmd, 10);

}

void DallasTemperature::copyScratchpadFromEE(uint64_t *addr) {
    uint8_t requestCmd[] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8};
    uint8_t* addrPtr = (uint8_t*)addr;
    for(uint8_t i=0; i<8; i++) requestCmd[i+1] = addrPtr[i];
    _oneWire->sendCommand(requestCmd, 10);

}

