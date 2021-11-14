#include "SimplePackage.h"
#include "Stream.h"
#include <gmock/gmock.h>

static Stream stream;

TEST(PACKAGE, create_package) {
    SimplePackage package(stream);
    package.init();
    const char *data = "this is the test";
    uint8_t dataLength = sizeof(data);
    uint8_t packageLength = package.createPackage(const_cast<char *>(data), dataLength);
    EXPECT_EQ(dataLength + 4, packageLength);

    EXPECT_EQ(0xc8, package.packageTxBuffer[0]);
    EXPECT_EQ(dataLength, package.packageTxBuffer[1]);

    for(int i = 0; i < dataLength; i++) {
        EXPECT_EQ(data[i], package.packageTxBuffer[i + 2]);
    }

    uint16_t crc = SimplePackage::getCRC((uint8_t *) data, 0, dataLength);

    EXPECT_EQ(SimplePackage::getFirstByte(crc), package.packageTxBuffer[packageLength - 2]);
    EXPECT_EQ(SimplePackage::getSecondByte(crc), package.packageTxBuffer[packageLength - 1]);
}

TEST(PACKAGE, receive_package) {
    SimplePackage package(stream);
    package.init();
    uint8_t myData [] = "this is the test 1234\n";
    uint16_t crc = SimplePackage::getCRC(myData, 0, (uint8_t) sizeof(myData));

    stream.pushToRXBuffer(0xc8);
    stream.pushToRXBuffer((uint8_t) sizeof(myData));

    for (unsigned char i : myData) {
        stream.pushToRXBuffer(i);
    }
    stream.pushToRXBuffer(SimplePackage::getFirstByte(crc));
    stream.pushToRXBuffer(SimplePackage::getSecondByte(crc));

    for(int i = 0; i < 100; i++) {
        package.run();
    }
    EXPECT_EQ(package.available(), sizeof(myData));
}

TEST(PACKAGE, create_receive) {
    SimplePackage package(stream);
    package.init();
    const char *data = "this is the test";
    uint8_t dataLength = strlen(data);
    uint8_t packageLength = package.createPackage(const_cast<char *>(data), dataLength);

    for (uint8_t i = 0; i < packageLength; i++) {
        stream.pushToRXBuffer(package.packageTxBuffer[i]);
    }

    for(int i = 0; i < 100; i++) {
        package.run();
    }

    EXPECT_EQ(package.available(), strlen(data));

    package.flush();

    for (uint8_t i = 0; i < packageLength; i++) {
        stream.pushToRXBuffer(package.packageTxBuffer[i]);
    }

    for(int i = 0; i < 100; i++) {
        package.run();
    }

    EXPECT_EQ(package.available(), strlen(data));

    package.flush();

    uint8_t temp_byte = package.packageTxBuffer[packageLength - 1];
    package.packageTxBuffer[packageLength - 1] = 4; // Create CRC error

    for (uint8_t i = 0; i < packageLength; i++) {
        stream.pushToRXBuffer(package.packageTxBuffer[i]);
    }

    for(int i = 0; i < 100; i++) {
        package.run();
    }

    EXPECT_EQ(package.available(), 0);

    package.flush();

    package.packageTxBuffer[packageLength - 1] = temp_byte;

    for (uint8_t i = 0; i < packageLength; i++) {
        stream.pushToRXBuffer(package.packageTxBuffer[i]);
    }

    for(int i = 0; i < 100; i++) {
        package.run();
    }
    EXPECT_EQ(package.available(), strlen(data));
}

TEST(PACKAGE, create_receive_error) {
    SimplePackage package(stream);
    package.init();
    const char *data = "this is the test";
    uint8_t dataLength = strlen(data);
    uint8_t packageLength = package.createPackage(const_cast<char *>(data), dataLength);

    package.packageTxBuffer[packageLength - 1] = 4; // Create CRC error

    for (uint8_t i = 0; i < packageLength; i++) {
        stream.pushToRXBuffer(package.packageTxBuffer[i]);
    }

    for(int i = 0; i < 100; i++) {
        package.run();
    }
    EXPECT_EQ(package.available(), 0);
}


TEST(PACKAGE, CRC) {
    uint8_t byte = 117;
    uint16_t crc = 0xFFFF;

    // CRC-CCITT without LUT
    byte ^= (uint8_t)(crc & 0x00ffu);
    byte ^= (uint8_t)(byte << 4u);
    crc = (((uint16_t)((uint16_t)byte << 8u) | (uint16_t)((uint8_t)crc >> 8u)) ^ (uint8_t)(byte >> 4u) ^ ((uint16_t)byte << 3u));

    std::cout << crc << std::endl;
}