#include "RFIDEasy.h"

/**
 * Default constructor
 */
RFIDEasy::RFIDEasy(int SS_PIN, int RST_PIN) {
	this->mfrc522 = new MFRC522(SS_PIN, RST_PIN);

	SPI.begin();        // Init SPI bus
  	this->mfrc522->PCD_Init(); // Init MFRC522 card

	// Clears key
	for (byte i = 0; i < 6; i++)
    	key.keyByte[i] = 0xFF;
}

/**
 * Write data to the RFID tag
 * @param block  The block to write
 * @param buffer [description]
 */
void RFIDEasy::writeBlock(int blockNumber, byte buffer[]) {

	//this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
	int largestModulo4Number = blockNumber / 4 * 4;
	int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector

	// Check for block number
	// block number is a trailer block (modulo 4); quit and send error code 2
	if (blockNumber > 2 && (blockNumber + 1) % 4 == 0) {
		Serial.println(String("[ERROR] Block ") + String(blockNumber) + String(" is a trailer block, cannot write."));
		return;
	}

	// Try to authenticate
	MFRC522::StatusCode status = this->mfrc522->PCD_Authenticate(
		MFRC522::PICC_CMD_MF_AUTH_KEY_A,
		trailerBlock, &(this->key), &(this->mfrc522->uid)
	);
	// byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
	// this method is used to authenticate a certain block for writing or reading
	// command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A  = 0x60 (=1100000),    // this command performs authentication with Key A
	// blockAddr is the number of the block from 0 to 15.
	// MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
	// Uid *uid is a pointer to the UID struct that contains the user ID of the card.
	
	// Check if successful
	if (status != MFRC522::STATUS_OK) {
		Serial.print("[ERROR] PCD_Authenticate() failed: ");
		Serial.println(mfrc522->GetStatusCodeName(status));
		return;
	}
	// it appears the authentication needs to be made before every block read/write within a specific sector.
	// If a different sector is being authenticated access to the previous one is lost.


	// Write on the block
	status = this->mfrc522->MIFARE_Write(blockNumber, buffer, 16);
	// MIFARE_Write(block number (0-15), byte array containing 16 values, number of bytes in block (=16))
	
	// Check if successful
	if (status != MFRC522::STATUS_OK) {
		Serial.print("[ERROR] MIFARE_Write() failed: ");
		Serial.println(mfrc522->GetStatusCodeName(status));
		return;
	}
}

/**
 * Writes data to the RFID tag
 * @param blockNumber [description]
 * @param text        [description]
 */
void RFIDEasy::writeBlock(int blockNumber, String text) {
	// Create buffer
	byte buffer[sizeof(text)];
	text.getBytes(buffer, sizeof(text));

	// Run method
	this->writeBlock(blockNumber, buffer);
}

/**
 * Reads a block from the RFID
 * @param  blockNumber The block to read
 * @return             [description]
 */
String RFIDEasy::readBlock(int blockNumber) {

	int largestModulo4Number = blockNumber / 4 * 4;
	int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector

	// Try to authenticate
	MFRC522::StatusCode status = this->mfrc522->PCD_Authenticate(
		MFRC522::PICC_CMD_MF_AUTH_KEY_A,
		trailerBlock, &(this->key), &(mfrc522->uid)
	);
	// byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
	// this method is used to authenticate a certain block for writing or reading
	// command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A = 0x60 (=1100000),    // this command performs authentication with Key A
	// blockAddr is the number of the block from 0 to 15.
	// MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
	// Uid *uid is a pointer to the UID struct that contains the user ID of the card.
	
	// Check if successful
	if (status != MFRC522::STATUS_OK) {
		Serial.print("[ERROR] PCD_Authenticate() failed: ");
		Serial.println(this->mfrc522->GetStatusCodeName(status));
		return String(0);
	}
	//it appears the authentication needs to be made before every block read/write within a specific sector.
	//If a different sector is being authenticated access to the previous one is lost.

	// Read the data
	byte buffersize = 18;
	byte buffer[18];

	// we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size... 
	status = this->mfrc522->MIFARE_Read(blockNumber, buffer, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
	
	// Check if successful
	if (status != MFRC522::STATUS_OK) {
		Serial.print("[ERROR] MIFARE_Write() failed: ");
		Serial.println(this->mfrc522->GetStatusCodeName(status));
		return String(0);
	}

	return String((char*) buffer);
}

/**
 * [RFIDEasy::write description]
 * @param startBlock [description]
 * @param buffer     [description]
 */
void RFIDEasy::write(int startBlock, byte buffer[]) {
	// Get the data size
	int buffer_size = sizeof(buffer);

	// Get the number of blocks to write the data
	int blocks = buffer_size / 16;
	if(buffer_size % 16 != 0) blocks++;
	
	// Start the current block with the argument
	int currentBlock = startBlock;

	int writedBlocks = 0;
	// Finish loop if all blocks are writed
	while(writedBlocks < blocks) {
		
		if(writedBlocks < blocks - 1) {
			byte subarray[16];

			for(int i = 0; i < 16; i++) {
				subarray[i] = buffer[i + (writedBlocks * 16)];
			}

			Serial.println((char*) subarray);
		}

		// Split array

		// Write
		//this->writeBlock(currentBlock, );

		// Update the current block
		currentBlock++;

		// Avoid trailer blocks
		if(currentBlock % 4 == 0)
			currentBlock++;

		// Update blocks writed count
		writedBlocks++;
	}
}