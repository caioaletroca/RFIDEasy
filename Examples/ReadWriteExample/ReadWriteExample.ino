#include <RFIDEasy.h>

/**
 * DEFINITIONS
 * Change the pins for your setup
 */
#define SS_PIN D4
#define RST_PIN D2

// Globals
RFIDEasy rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);

  // Log
  Serial.println("[INFO] Starting ReadWriteExample");
}

void loop() {
  // Look for new cards
  if ( ! rfid.IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! rfid.ReadCardSerial())
    return;

  // Get tag UID
  String uid = rfid.getUID();
  Serial.print("[INFO] Tag UID: "); Serial.println(uid);

  // Define the block to start
  // For more information, please consult the README file
  int startBlock = 4;

  // Define the data to write
  String data = "To be, or not to be, that is the question.";
  int blocksNumber = rfid.sizeBlocks(data);

  // Write the data on the tag
  rfid.write(startBlock, data);

  // Check for errors
  if(rfid.error) {
    Serial.println("[ERROR] An error occurred in the writing process.");
    return;
  }

  // Read the tag data
  String storedData = rfid.read(startBlock, blocksNumber);

  // Check for errors
  if(rfid.error) {
    Serial.println("[ERROR] An error occurred in the reading process.");
    return;
  }

  // Log
  Serial.println("[SUCCESS] The operation was successful");
  Serial.print("[INFO] The data read: "); Serial.println(storedData);
}
