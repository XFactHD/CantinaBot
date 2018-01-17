const int GLASS_SENSOR = 13;

void waitForGlass() {
  printMessageMultiArg("Auswahl: ", recipeNames[selectedRecipe], "Warte auf Glass!", "");
  boolean success = true;
  while(digitalRead(GLASS_SENSOR) == LOW)
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

