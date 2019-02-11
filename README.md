# RFIDEasy &middot; [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

Handles the RFID operations in MIFARE cards more easely and undertandable.
This library was only tested on MIFARE cards with 1K of memory. If you wanna expand this library capatibilities, feel free to make a pull request.

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

### IsNewCardPresent
```cpp
bool IsNewCardPresent()
```
> This method is a handleful access for the internal library with similar name. For more information read the [Attention](#attention) section.

### ReadCardSerial
```cpp
bool ReadCardSerial()
```
> This method is another a handleful access for the internal library with similar name. It calls the initial handleshaking between the card and the RFID sensor.

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

### readBlock

```cpp
String readBlock(int blockNumber)
```
> Reads a block from the card memory and returns the data as a string. The block only has 16 bytes long

### read

```cpp
String read(int startBlock, int blocksNumber)
```
> Reads data from the card, starting at a specified block, with a number of blocks long. The number of blocks read doesn't count the trailling blocks, which is automatically skipped. The blocks are concatenated and returns as a string

### close

```cpp
void close()
```
> Close the connection between the card and the RFID sensor, preventing for multiples read/write operations.

## Misc Methods

### clearBlock

```cpp
void clearBlock(int blockNumber, char character)
```
> Clear a specified block filling it with one character passed as argument

### clear

```cpp
void clear(int startBlock, int blocksNumber, char character)
```
> Clear a memory section starting with a specific block, and ending within a length, following the same logic as the write method. The spaces are filled with a character

### sizeBlocks

Calculate the number of blocks from the memory needed to store the data.

```cpp
int sizeBlocks(int buffer_length)
```
> Receives the buffer_length as argument

```cpp
int sizeBlocks(String text)
```
> Calculate the size with the string data

# Attention

Both the methods IsNewCardPresent() and close() prevents the sensor to perform multiples operations on the same card. IsNewCardPresent only returns trues if the sensor detects a card in range, and the connection was not closed. But to update the internal API, it needs to be called once with no card in the sensor's range.
You could call this method always on the main loop in the micro controller, but if you use in another location, keep in mind this behavior.

# Example

- [ReadWriteExample](https://github.com/caioaletroca/RFIDEasy/blob/master/Examples/ReadWriteExample/ReadWriteExample.ino);
