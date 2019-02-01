#include "RFIDEasy.h"

/**
 * Default constructor
 * Receives the RFID sensor pins as arguments
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
 * Run the RFID method
 * @return [description]
 */
bool RFIDEasy::IsNewCardPresent() {
	return this->mfrc522->PICC_IsNewCardPresent();
}

/**
 * Run the RFID method
 * @return [description]
 */
bool RFIDEasy::ReadCardSerial() {
	return this->mfrc522->PICC_ReadCardSerial();
}

String RFIDEasy::getUID() {
	String uid;

	// Get the uid on the tag
	for (byte i = 0; i < this->mfrc522->uid.size; i++) 
	{
		uid.concat(String(this->mfrc522->uid.uidByte[i] < 0x10 ? " 0" : " "));
		uid.concat(String(this->mfrc522->uid.uidByte[i], HEX));
	}

	// Format data
	uid.toUpperCase();
	uid.replace(" ", "");

	return uid;
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
		Serial.println(this->mfrc522->GetStatusCodeName(status));
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
		Serial.println(this->mfrc522->GetStatusCodeName(status));
		return;
	}
}

/**
 * Writes data to the RFID tag
 * @param blockNumber the block to write the data
 * @param text        The data
 */
void RFIDEasy::writeBlock(int blockNumber, String text) {
	// Create buffer
	byte buffer[text.length()];
	text.getBytes(buffer, text.length());

	// Run method
	this->writeBlock(blockNumber, buffer);
}

/**
 * Reads a block from the RFID
 * @param  blockNumber The block to read
 * @return             The read data
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
	status = this->mfrc522->MIFARE_Read(blockNumber, buffer, &buffersize);
	// &buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
	
	// Check if successful
	if (status != MFRC522::STATUS_OK) {
		Serial.print("[ERROR] MIFARE_Read() failed: ");
		Serial.println(this->mfrc522->GetStatusCodeName(status));
		return String(0);
	}

	// Returns the data
	return String((char*) buffer);
}

/**
 * Writes a array of data continuously on the tag system, jumping the not allowed trailling blocks
 * @param startBlock 	The start block to write
 * @param buffer     	The buffer with data
 * @param buffer_length	The buffer size
 */
void RFIDEasy::write(int startBlock, byte buffer[], int buffer_length) {
	// Get the number of blocks to write the data
	int blocks = this->sizeBlocks(buffer_length);

	// Normalize the data, puts blank space until complete the last block
	byte* test = this->normalize(blocks, buffer, buffer_length);

	// Start the current block with the argument
	int currentBlock = startBlock;

	// Check if the data is too big
	if(startBlock + blocks >= 16) {
		Serial.println("[ERROR] The data length exceed the 16 blocks limit.");
		return;
	}

	// Finish loop if all blocks are writed
	byte buffer_block[16];
	int writedBlocks = 0;
	while(writedBlocks < blocks) {

		// Get a substring of the data to fit on the block
		this->subArray(writedBlocks * 16, 16, test, buffer_block);

		// Write to the tag
		this->writeBlock(currentBlock, buffer_block);

		// Update the current block
		currentBlock++;

		// Avoid trailer blocks
		if(this->checkTrailerBlock(currentBlock))
			currentBlock++;

		// Update blocks writed count
		writedBlocks++;
	}

	// Dispose memory
	delete test;
}

/**
 * Writes a array of data continuously on the tag system, jumping the not allowed trailling blocks
 * @param startBlock 	The start block to write
 * @param text     		The data
 */
void RFIDEasy::write(int startBlock, String text) {
	// Create buffer
	byte buffer[text.length() + 1];
	text.getBytes(buffer, text.length() + 1);

	// Run method
	this->write(startBlock, buffer, text.length() + 1);
}

/**
 * Reads a sequence of blocks from the tag and returns a concatenated string
 * @param  startBlock The start block to read
 * @param  endBlock   The end block to read
 * @return            The read data
 */
String RFIDEasy::read(int startBlock, int endBlock) {
	String response = "";

	// Reads data and trims the end
	int currentBlock = startBlock;
	for(int i = 0; i < endBlock - startBlock + 1; i++) {
		// Skip if hit the trailing block
		if(this->checkTrailerBlock(currentBlock))
			currentBlock++;

		// Read the data
		String test = this->readBlock(currentBlock);
		response += String(test).substring(0, 16);

		// Add to the current read block
		currentBlock++;
	}

	return response;
}

/**
 * End the communication between the RFID sensor and arduino
 */
void RFIDEasy::endProcess() {
	this->mfrc522->PICC_HaltA();
  	this->mfrc522->PCD_StopCrypto1();
}

/**
 * Clears a block in the tag filling with the specified character
 * @param blockNumber The block to clear
 * @param character   The character to fill
 */
void RFIDEasy::clearBlock(int blockNumber, char character) {
	// Create the data
	String sequence = "";
	for(int i = 0; i < 16; i++)
		sequence += character;

	// Clears the block
	this->writeBlock(blockNumber, sequence);
}

/**
 * Clears a area in the tag filling with the specified character
 * @param startBlock  The start block to clear
 * @param endBlock    The end block to clear
 * @param character   The character to fill
 */
void RFIDEasy::clear(int startBlock, int endBlock, char character) {

	// Reads data and trims the end
	int currentBlock = startBlock;
	for(int i = 0; i < endBlock - startBlock + 1; i++) {
		// Skip if hit the trailing block
		if(this->checkTrailerBlock(currentBlock))
			currentBlock++;

		// Clears the block
		this->clearBlock(currentBlock, character);	

		// Add to the current read block
		currentBlock++;
	}
}

/**
 * Gets the size of the data in blocks count
 * @param  buffer_length The buffer size
 * @return               The size in blocks
 */
int RFIDEasy::sizeBlocks(int buffer_length) {
	int blocks = buffer_length / 16;
	if(buffer_length % 16 != 0) blocks++;

	return blocks;
}

/**
 * Gets the size of the data in blocks count
 * @param  text The data
 * @return      The size in blocks	
 */
int RFIDEasy::sizeBlocks(String text) {
	return this->sizeBlocks(text.length());
}

/**
 * Checks if a block is a trailer or not
 * @param  blockNumber The block to check
 * @return             [description]
 */
bool RFIDEasy::checkTrailerBlock(int blockNumber) {
	// Check for block number
	// block number is a trailer block (modulo 4)
	return (blockNumber > 2 && (blockNumber + 1) % 4 == 0);
}

/**
 * Normalize the data, inserting blank space in the final of the buffer to complete a full number block size
 * @param  blocks        The correct number of blocks
 * @param  buffer        The data
 * @param  buffer_length The buffer size
 * @return               [description]
 */
byte* RFIDEasy::normalize(int blocks, byte buffer[], int buffer_length) {
	byte* normalized = new byte[blocks * 16];
	for(int i = 0; i < blocks * 16; i++) {
		if(i < buffer_length)
			normalized[i] = buffer[i];
		else
			normalized[i] = ' ';
	}
	return normalized;
}

/**
 * Creates a sub array derivated
 * @param start    The start for the subarray
 * @param length   The length to copy from the original array
 * @param array    The data array
 * @param subarray The coppied sub array
 */
void RFIDEasy::subArray(int start, int length, byte array[], byte subarray[]) {
	for(int i = 0; i < length; i++) {
		subarray[i] = array[i + start];
	}
}