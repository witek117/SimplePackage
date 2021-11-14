# SimplePackage
Create simple package string with CRC

### How to use?
```cpp
// Create Stream object based on virtual SimpleStream class.
Stream stream;

// Create SimplePackage object which works with stream
SimplePackage package(stream);
package.init();

// buffer for received message
uint8_t rxBuffer[100] = {0};
// in loop get received message from SimplePackage
int receivedLength = package.available();
for(int i = 0; i < receivedLength; i++) {
    rxBuffer[i] = package.read(); // save received message
}

// send SimplePackage using SimpleStream
uint8_t txBuffer = "Hello world";
package.write(txBuffer, sizeof(txBuffer));
```

