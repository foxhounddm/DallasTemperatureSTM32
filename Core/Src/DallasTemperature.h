//
// Created by dmitriy on 24.01.2021.
//

#ifndef TESTF_DALLASTEMPERATURE_H
#define TESTF_DALLASTEMPERATURE_H


#include "OneWire.h"

class DallasTemperature {
private:
    OneWire *_oneWire;
    void getScratchpad(uint64_t* addr, uint8_t* data);
public:
    DallasTemperature(OneWire *oneWire);

    uint8_t isTemperatureSensor(uint64_t *addr);

    void requestTemperatures();

    void requestTemperaturesByAddress(const uint64_t *addr);

    void requestTemperaturesByIndex(uint8_t index);

    float getTempC( uint64_t *addr);

    float getTempCByIndex(uint8_t index);

    void setResolution(uint8_t resolution);

    void setResolution(uint8_t resolution, uint8_t deviceIndex);

    void setResolution(uint8_t resolution, uint64_t* deviceAddr);

    uint8_t getResolution( uint8_t index);

    uint8_t getResolution( uint64_t *addr);

    void copyScratchpadToEE(uint64_t *addr);
    void copyScratchpadFromEE(uint64_t *addr);


};


#endif //TESTF_DALLASTEMPERATURE_H
