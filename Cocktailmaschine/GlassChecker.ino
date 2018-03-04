const int GLASS_SENSOR = A0; //Glass sensor pin

//Initializes the GlassChecker
void initGlassChecker() {
  pinMode(GLASS_SENSOR, INPUT);
}

//Called when a recipe was selected by the user, waits 10 seconds for the user to put a glass on the turntable
void waitForGlass() {
  printMessageMultiArg("Auswahl: ", recipeNames[selectedRecipe], "Warte auf Glass!", "");
  boolean success = true;
  while(digitalRead(GLASS_SENSOR) == HIGH)
  {
    if(timeOver(10000))
    {
      success = false;
      setState(STATE_MAIN_MENU);
      break;
    }
    delay(50);
  }
  
  if(success)
  {
    timestamp = 0;
    setState(STATE_WAITING_START);
  }
  else
  {
    printMessage("Glass fehlt!", "Vorgang gestoppt!");
    delay(1000);
  }
}

//Called when the start button was pressed, checks if the user left the glass on the turntable
boolean checkGlassStillThere() {
  boolean stillThere = false;
  setTimestamp();
  while(!timeOver(1000))
  {
    if(digitalRead(GLASS_SENSOR) == HIGH)
    {
      stillThere = true;
      break;
    }
    delay(50);
  }
  return stillThere;
}

