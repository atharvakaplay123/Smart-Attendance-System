void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_R, HIGH);
    return;
  }

  else {
    Serial.println("Authentication success");
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, LOW);
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_R, HIGH);
    return;
  }

  else {
    Serial.println("Block was read successfully");
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, LOW);
  }
}