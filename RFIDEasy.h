#ifndef __RFID_EASY__
#define __RFID_EASY__

#include "Arduino.h"
#include "SPI.h"
#include "MFRC522.h"

/**
 * This library is to use with the MFRC522 library that controls the RFID sensor for writings and readings
 *
 * It was only tested with MIFARE 1K memory tag and it methods only works with that tags
 * Link to tag's manufacturer manual: https://www.jadaktech.com/skyetekfiles/docs/m2/mifareclassic.pdf
 */
class RFIDEasy
{
	public:
		// Properties
		MFRC522* mfrc522;

		// Methods
		RFIDEasy(int SS_PIN, int RST_PIN);

		// Dependency Methods
		bool IsNewCardPresent();
		bool ReadCardSerial();

		// Read/Write Operations
		String getUID();
		void writeBlock(int blockNumber, byte buffer[]);
		void writeBlock(int blockNumber, String text);
		String readBlock(int blockNumber);
		void write(int startBlock, byte buffer[], int buffer_length);
		void write(int startBlock, String text);
		String read(int startBlock, int blocksNumber);

		// Addons
		void endProcess();
		void clearBlock(int blockNumber, char character);
		void clear(int startBlock, int endBlock, char character);
		int sizeBlocks(int buffer_length);
		int sizeBlocks(String text);

	private:
		MFRC522::MIFARE_Key key;

		// Utilities
		bool checkTrailerBlock(int blockNumber);
		byte* normalize(int blocks, byte buffer[], int buffer_length);
		void subArray(int start, int length, byte array[], byte subarray[]);
};

#endif