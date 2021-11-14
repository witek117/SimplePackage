#pragma once

#include "SimpleStream.h"
#include "cyclicBuffer.hpp"
#include <cstdint>
#include <cstring>


class SimplePackage : public SimpleStream {
    SimpleStream &stream;
    const uint8_t startWord = 0xc8;

    uint8_t rxBufferLen = 0;
    uint8_t crcIndex = 0;
    uint16_t receivedCrc = 0;
    uint8_t txPackageLength = 0;

    enum class State : uint8_t {
        START = 0,
        GET_LENGTH = 1,
        GET_DATA = 2,
        GET_CRC = 3,
    };

    State state = State::START;
protected:
    uint8_t length = 0;

public:
    SimplePackage(SimpleStream &stream) : stream(stream) {

    }

    void init() override {
        stream.flush();
    }

    uint16_t available() override {
        return availableData.getLength();
    }

    void write(uint8_t* data, uint16_t length) override {
        size_t packetLength = createPackage(reinterpret_cast<char *>(data), length);
        stream.write(packageTxBuffer, packetLength);
    }

    uint8_t read() override {
        return availableData.get();
    }

    void flush() override {
        availableData.flush();
    }

    CyclicBuffer_data <uint8_t, 100> availableData;
    uint8_t packageRxBuffer[100] = {0};
    uint8_t packageTxBuffer[100] = {0};

    uint8_t getLength() const {
        return length;
    }

    static uint16_t updateCRC(uint8_t byte, uint16_t crc) {
        // CRC-CCITT without LUT
        byte ^= (uint8_t)(crc & 0x00ffu);
        byte ^= (uint8_t)(byte << 4u);
        return (((uint16_t)((uint16_t)byte << 8u) | (uint16_t)((uint8_t)crc >> 8u)) ^ (uint8_t)(byte >> 4u) ^ ((uint16_t)byte << 3u));
    }

    static uint16_t getCRC(uint8_t* data, uint8_t startIndex, uint8_t length) {
        uint16_t crc = 0xFFFF;
        for(uint8_t i = startIndex; i < (startIndex + length); i++) {
            crc = updateCRC(data[i], crc);
        }
        return crc;
    }

    static inline uint8_t getFirstByte(uint16_t crc) {
        return crc & 0xffu;
    }

    static inline uint8_t getSecondByte(uint16_t crc) {
        return crc >> 8u;
    }

    static inline uint16_t toUint16(uint8_t firstByte, uint8_t secondByte) {
        return (uint16_t)((uint16_t)firstByte | (uint16_t)(secondByte << 8u));
    }

    // virtual void useValidData() = 0;

    void run() {
        size_t length = stream.available();
        for(size_t i = 0; i < length; i++) {
            putChar(stream.read());
        }
    }

private:
    bool putChar(uint8_t c) {
        switch (state) {
            case State::START:
                if (c == startWord) {
                    state = State::GET_LENGTH;
                    rxBufferLen = 0;
                }
                break;
            case State::GET_LENGTH:
                length = c;
                state = State::GET_DATA;
                rxBufferLen = 0;
                break;
            case State::GET_DATA:
                packageRxBuffer[rxBufferLen++] = c;
                if (rxBufferLen == length) {
                    state = State::GET_CRC;
                    crcIndex = 0;
                }
                break;
            case State::GET_CRC:
                if (crcIndex == 0) {
                    receivedCrc = c;
                    crcIndex = 1;
                } else if (crcIndex == 1) {
                    receivedCrc = SimplePackage::toUint16(receivedCrc, c);
                    uint16_t myCrc = SimplePackage::getCRC(packageRxBuffer, 0, length);
                    if (myCrc == receivedCrc) {
                        availableData.write(packageRxBuffer, length);
                    }
                    state = State::START;
                }
                break;
            default:
                length = 0;
        }

        return false;
    }

public:
    uint8_t createPackage(char* data, uint8_t length) {
        txPackageLength = 0;
        uint8_t offset = 0;
        packageTxBuffer[0] = startWord;
        packageTxBuffer[1] = length;
        offset = 2;
        memcpy(packageTxBuffer + offset, data,  length);
        uint16_t crc = SimplePackage::getCRC(packageTxBuffer, 2, length);
        offset += length;
        packageTxBuffer[offset] = SimplePackage::getFirstByte(crc);
        packageTxBuffer[offset + 1] = SimplePackage::getSecondByte(crc);
        txPackageLength = offset + 2;
        return txPackageLength;
    }
};