#pragma once

#include "SimpleStream.h"
#include <vector>
#include <string>
#include "iostream"

class Stream : public SimpleStream {
    std::vector<uint8_t> rxData;
    std::vector<uint8_t> txData;

public:
    uint16_t available() override {
        return rxData.size();
    }

    void write(uint8_t *data, uint16_t length) override {
        for (uint16_t i = 0; i < length; ++i) {
            txData.push_back(data[i]);
        }
    }

    uint8_t read() override {
        uint8_t data = rxData.front();
        rxData.erase(rxData.begin());
        return data;
    }

    void pushToRXBuffer(uint8_t data) {
        rxData.push_back(data);
    }

    std::string getTxBuffer() {
        std::string data;
        while (!txData.empty()) {
            data += (char) (txData.front());
            txData.erase(txData.begin());
        }
        return data;
    }

    void flush() override {
        rxData.clear();
        txData.clear();
    }
};