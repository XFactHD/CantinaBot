String message = "";
String output = "";
boolean serialRunning = false;
boolean needsReboot = false;

//Waits for 2 seconds for a computer to attempt a connection with this device
void waitForSerialComm() {
  setTimestamp();
  while(!timeOver(2000) && Serial.available() == 0) { ; }
  
  if(Serial.available() > 0)
  {
    setState(STATE_SERIAL_COMM);
    message.reserve(200);
    output.reserve(400);
  }
}

//Processes the message received from the serial port
void receiveMessage() {
  delay(100);
  Serial.println(message);
  if((message == "START") && !serialRunning && !needsReboot)
  {
    serialRunning = true;
    printMessage("SERIAL ACTIVE", "WAITING FOR DATA");
    delay(100);
    Serial.println("ACK");
  }
  else if(message == "TERMINATE" && serialRunning)
  {
    serialRunning = false;
    needsReboot = true;
    Serial.println("ACK");
    printMessage("SERIAL CLOSED", "WRITING EEPROM");
    writeToEEPROM();
    printMessage("SERIAL CLOSED", "REBOOT ME!");
    delay(100);
  }
  else if(message == "GETINGREDIENTS")
  {
    delay(100);
    Serial.println("ACK");
    output += "INGREDIENTS;";
    for(int i = 0; i < 6; i++)
    {
      output += ingredientNames[i];
      if(i < 5)
      {
        output += ";";
      }
    }
    output += ";END";
    Serial.println(output);
    output = "";
  }
  else if(message == "GETRECIPES")
  {
    delay(100);
    Serial.println("ACK");
    output += "RECIPES;";
    for(int i = 0; i < 8; i++)
    {
      output += recipeNames[i];
      output += '|';
      int ingredientCount = recipeIngredientCounts[i];
      output += ingredientCount;
      if(ingredientCount > 0)
      {
        output += '|';
      }
      for(int j = 0; j < ingredientCount; j++)
      {
        int ingredient = recipeIngredients[i][j];
        output += ingredient;
        output += ',';
        output += recipeIngredientAmounts[i][j];
        if(j < ingredientCount - 1)
        {
          output += '|';
        }
      }
      if(i < 7)
      {
        output += ';';
      }
    }
    output += ";END";
    Serial.println(output);
    output = "";
  }
  else
  {
    if(message.startsWith("INGREDIENTS;") && message.endsWith(";END"))
    {
      Serial.println("ACK");
      
      message.replace("INGREDIENTS;", "");
      message.replace(";END", "");

      char* data = message.c_str();
      char* part = strtok(data, ";");
      int index = 0;
      while(part != NULL)
      {
        memcpy(ingredientNames[index], part, (strlen(part) + 1) * sizeof(char));
        index++;
        part = strtok(NULL, ";");
      }
      
      Serial.println("ACP");
    }
    else if(message.startsWith("RECIPES;") && message.endsWith(";END"))
    {
      Serial.println("ACK");
      delay(100);

      message.replace("RECIPES;", "");
      message.replace(";END", "");

      char* data = message.c_str();
      char* recipe = strtok(data, ";");
      char recipes[8][47];
      int index = 0;
      while(recipe != NULL && index < 8)
      {
        memcpy(recipes[index], recipe, (strlen(recipe) + 1) * sizeof(char));
        recipe = strtok(NULL, ";");
        index++;
      }

      int size = index;
      for(int i = 0; i < size; i++)
      {
        char* recipePart = strtok(recipes[i], "|");
        char recipeParts[8][9];
        int index = 0;
        while(recipePart != NULL && index < 8)
        {
          memcpy(recipeParts[index], recipePart, (strlen(recipePart) + 1) * sizeof(char));
          index++;
        }

        setName(i, recipeParts[0]);
        int ingredientCount = atoi(recipeParts[1]);
        for(int j = 0; j < ingredientCount; j++)
        {
          int stringIndex = j + 2;
          char* part = strtok(recipeParts[stringIndex], ",");
          int ingredient = atoi(part);
          part = strtok(NULL, ",");
          int amount = atoi(part);
          setIngredient(i, j, ingredient);
          setAmountOfIngredient(i, ingredient, amount);
        }
      }

      Serial.println("ACP");
    }
    else
    {
      Serial.println("NACK");
    }
  }
}

//Called by a software routine when a byte is received on the serial bus, assembles the bytes into a string and calls receiveMessage() when the string is complete
void serialEvent() {
  if(state != STATE_SERIAL_COMM || needsReboot) { return; }

  while(Serial.available())
  {
    char inChar = (char)Serial.read();
    if(inChar == '\n')
    {
      receiveMessage();
      message = "";
    }
    else
    {
      message += inChar;
    }
  }
}

