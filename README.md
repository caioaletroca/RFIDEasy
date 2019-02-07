# RFIDEasy &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

Handles the RFID operations in MIFARE cards more easely and undertandable.
This library was only tested on MIFARE cars with 1K of memory. If you wanna expand this library capatibilities, feel free to make a pull request.

# Information
You can find more information about the memory on that tag on the manufactures manual [here](https://www.jadaktech.com/skyetekfiles/docs/m2/mifareclassic.pdf).

# Quick Start

## Installation
Download as a zip file and uses the Arduino IDE to install normally. It's requires the [MFRC522](https://github.com/miguelbalboa/rfid) library to work.

## Usage
Include the library on your sketch

```cpp
#include <RFIDEasy.h>
```
Initialize a global instance of the library

```cpp
RFIDEasy rfid;
```

# Documentation

### Constructors
```cpp
RFIDEasy(int SS_PIN, int RST_PIN);
```
> Default constructor. Receives the SS and RST pins as arguments.

## Read/Write Operations

### getUID
```cpp
String getUID();
```
> Reads the card UID and returns as a string

### writeBlock

Writes the data on the specified block. Keep in mind that the blocks only have 16 bytes long.

```cpp
void writeBlock(int blockNumber, byte buffer[])
```
> Receives the block number to write and the data as a byte array
```cpp
void writeBlock(int blockNumber, String text)
```
> Receives the block number to write and the data as a string

### write

The number of blocks is automatically calculated and the trailling blocks is jumped. The data will be splitted across the blocks and the last block, if does not fill the 16 bytes, the string end will be filled with black spaces.

```cpp
void write(int startBlock, byte buffer[], int buffer_length)
```
> Receives the block number to start the write operation, the data as a byte array and the length of the data
```cpp
void write(int startBlock, String text)
```
> Receives the block number to start the write operation and the data as a string
