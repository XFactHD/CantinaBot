String message = "";
String output = "";
boolean serialRunning = false;

void waitForSerialComm() {
  setTimestamp();
  while(!timeOver(5000) && Serial.available() == 0) { ; }
  
  if(Serial.available() > 0)
  {
    setState(STATE_SERIAL_COMM);
    message.reserve(200);
    output.reserve(200);
  }
}

void receiveMessage() {
  delay(100);
  Serial.println(message);
  if((message == "START") && !serialRunning)
  {
    serialRunning = true;
    printMessage("SERIAL ACTIVE", "WAITING FOR DATA");
    delay(100);
    Serial.println("ACK");
  }
  else if(message == "TERMINATE" && serialRunning)
  {
    serialRunning = false;
    printMessage("SERIAL CLOSED", "REBOOT ME!");
    delay(100);
    Serial.println("ACK");
  }
  else if(message == "GETINGREDIENTS")
  {
    delay(100);
    output += "INGREDIENTS;";
    for(int i = 0; i < 6; i++)
    {
      output += ingredientNames[i];
      if(i < 5)
      {
        output += ";";
      }
    }
    Serial.println(output);
    output = "";
  }
  else if(message == "GETRECIPES")
  {
    delay(100);
    output += "RECIPES;"; //FIXME: doesn't work correctly
    for(int i = 0; i < 8; i++)
    {
      Recipe r = recipes[i];
      output += r.getName();
      output += "|";
      for(int j = 0; j < 6; j++)
      {
        int ingredient = r.getIngredient(j);
        output += ingredient;
        output += ".";
        output += r.getAmountOf(ingredient);
        if(j < 5)
        {
          output += "|";
        }
      }
      if(i < 7)
      {
        output += ";";
      }
    }
    Serial.println(output);
    output = "";
  }
  else
  {
    
  }
}

void serialEvent() {
  if(state != STATE_SERIAL_COMM) { return; }

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

