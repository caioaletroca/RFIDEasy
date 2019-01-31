#ifndef __RFID_EASY__
#define __RFID_EASY__

#include "Arduino.h"
#include "SPI.h"
#include "MFRC522.h"

class RFIDEasy
{
	public:
		// Properties
		MFRC522* mfrc522;

		// Methods
		RFIDEasy(int SS_PIN, int RST_PIN);

		void writeBlock(int blockNumber, byte buffer[]);
		void writeBlock(int blockNumber, String text);
		String readBlock(int blockNumber);
		void write(int startBlock, byte buffer[]);

	private:
		MFRC522::MIFARE_Key key;
};

#endif