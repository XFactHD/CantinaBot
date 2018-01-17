#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <A4988.h>

//PINS
const int BUTTON_INTERRUPT = 2;
const int SWITCH_INTERRUPT = 3;
const int BUTTON_START_OR_FILL_INFO = A0;
const int BUTTON_SELECT_1 = A2;
const int BUTTON_SELECT_2 = 5;
const int BUTTON_SELECT_3 = 6;
const int BUTTON_SELECT_4 = 7;
const int BUTTON_SELECT_5 = 8;
const int BUTTON_SELECT_6 = 9;
const int BUTTON_SELECT_7 = A1;
const int BUTTON_SELECT_8 = 11;
const int SWITCH_DISC_POS_ZERO = 0;
const int SWITCH_DISC_POS_COUNT = 0;
const int SWITCH_ARM_HOR_IN = 0;
const int SWITCH_ARM_HOR_OUT = 0;
const int SWITCH_ARM_VERT_TOP = 0;
const int SWITCH_ARM_VERT_BOTTOM = 0;

//ERROR CODES
const int ERROR_STRING_TOO_LONG = 0x00;

//STATES
const int STATE_INIT = 0;
const int STATE_SERIAL_COMM = 1;
const int STATE_FILL_INFO = 2;
const int STATE_MAIN_MENU = 3;
const int STATE_WAITING_GLASS = 4;
const int STATE_WAITING_START = 5;
const int STATE_WORKING = 6;
const int STATE_POST_PROCESS = 7;
const int STATE_ERRORED = 8;
const int STATE_READ_FILL_LEVEL = 9;

//MISC
const int STIR_TIME_MS = 6000;

char recipeNames[8][9]; //Names of the cocktails, has to be outside of the Recipe class to make writing to EEPROM easier, max string length is 8 (extra space is for null character)!

class Recipe {
private:
  int index;
  int count;
  int ingredients[6] {-1, -1, -1, -1, -1, -1};
  int amounts[6] {-1, -1, -1, -1, -1, -1};
public:
  Recipe(int index, char* name, int count) : index(index), count(count)
  {
    memcpy(recipeNames[index], name, strlen(name) * sizeof(char));
  }

  ~Recipe()
  {
    delete[] ingredients;
    delete[] amounts;
  }

  char* getName()
  {
    return recipeNames[index];
  }

  int getCount()
  {
    return count;
  }
  
  int* getIngredients()
  {
    return ingredients;
  }

  void setIngredient(int index, int ingredient)
  {
    ingredients[index] = ingredient;
  }

  int getIngredient(int index)
  {
    return ingredients[index];
  }

  void setAmountOf(int ingredient, int amount)
  {
    for(int i = 0; i < count; i++)
    {
      if(ingredients[i] == ingredient)
      {
        amounts[i] = amount;
        break;
      }
    }
  }

  int getAmountOf(int ingredient)
  {
    for(int i = 0; i < count; i++)
    {
      if(ingredients[i] == ingredient)
      {
        return amounts[i];
      }
    }
  }
};

char ingredientNames[6][6] { //max string length is 5 (extra space is for null character)!
  "Rum",
  "Cola",
  "Vodka",
  "Mango",
  "Gin",
  "Tonic"
};

Recipe recipes[8] {
  { 0, "Malaga", 0 },
  { 1, "RumCola", 0 },
  { 2, "GinTonic", 0 },
  { 3, "Malaga", 0 },
  { 4, "Zombie", 0 },
  { 5, "Jay-Dee", 0 },
  { 6, "Pacman", 0 },
  { 7, "Haven", 0 }
};

int fillLevels[6] { 0, 0, 0, 0, 0, 0 };
volatile int lastState = 0;
volatile int state = 0;
volatile boolean stateChanged = false;
volatile int selectedRecipe = -1;
unsigned long timestamp = 0; //Used by certain states for a timeout

void setup() {
  Serial.begin(9600);
  initDisplay();
  setState(STATE_INIT);
  printMessage("INITIALIZATION", "PLEASE WAIT");
  initFillLevelReader();
  initProcessHandler();
  pinMode(BUTTON_START_OR_FILL_INFO, INPUT);
  pinMode(BUTTON_SELECT_1, INPUT);
  pinMode(BUTTON_SELECT_2, INPUT);
  pinMode(BUTTON_SELECT_3, INPUT);
  pinMode(BUTTON_SELECT_4, INPUT);
  pinMode(BUTTON_SELECT_5, INPUT);
  pinMode(BUTTON_SELECT_6, INPUT);
  pinMode(BUTTON_SELECT_7, INPUT);
  pinMode(BUTTON_SELECT_8, INPUT);
  pinMode(SWITCH_DISC_POS_ZERO, INPUT);
  pinMode(SWITCH_DISC_POS_COUNT, INPUT);
  pinMode(SWITCH_ARM_HOR_IN, INPUT);
  pinMode(SWITCH_ARM_HOR_OUT, INPUT);
  pinMode(SWITCH_ARM_VERT_TOP, INPUT);
  pinMode(SWITCH_ARM_VERT_BOTTOM, INPUT);
  pinMode(13, INPUT);
  waitForSerialComm();
  if(state == STATE_SERIAL_COMM)
  {
    printMessage("SERIAL ACTIVE", "WAITING FOR START");
    return;
  }
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT), buttonISR, RISING);
  attachInterrupt(digitalPinToInterrupt(SWITCH_INTERRUPT), switchISR, RISING);
  moveArmToHome();
  rotateToPosZero();
  readAllFillLevels();
  setState(STATE_FILL_INFO);
}

void loop() {
  if(state == STATE_SERIAL_COMM) { return; }
  
  checkStateChange();
  if(state == STATE_FILL_INFO && timeOver(10000))
  {
    setState(STATE_MAIN_MENU);
  }
  else if(state == STATE_WAITING_START && timeOver(5000))
  {
    setState(STATE_MAIN_MENU);
  }
  else if(state == STATE_WAITING_GLASS)
  {
    waitForGlass();
  }
}

void setState(int newState) {
  lastState = state;
  state = newState;
  stateChanged = true;
}

void checkStateChange() {
  if(stateChanged)
  {
    if(state == STATE_FILL_INFO)
    {
      setTimestamp();
      showFillLevels();
    }
    else if(state == STATE_MAIN_MENU)
    {
      selectedRecipe = -1;
      showRecipes();
    }
    else if(state == STATE_WAITING_GLASS)
    {
      if(checkIngredientsAvailable(recipes[selectedRecipe]))
      {
        setTimestamp();
      }
      else
      {
        printMessage("", "");
        delay(1500);
      }
    }
    else if(state == STATE_WAITING_START)
    {
      printMessageMultiArg("Auswahl: ", recipeNames[selectedRecipe], "Start druecken!", "");
      if(checkGlassStillThere())
      {
        setTimestamp();
      }
      else
      {
        printMessage("Glass entfernt!", "Vorgang gestoppt!");
        delay(1500);
        setState(STATE_MAIN_MENU);
      }
    }
    else if(state == STATE_WORKING)
    {
      printMessageMultiArg("Auswahl: ", recipeNames[selectedRecipe], "Prozess laeuft!", "");
      process();
    }
    else if(state == STATE_POST_PROCESS)
    {
      printMessageMultiArg("Auswahl: ", recipeNames[selectedRecipe], "Fertig! Prost!", "");
      readFillLevelsPostMixing(recipes[selectedRecipe]);
    }
    else if(state == STATE_READ_FILL_LEVEL)
    {
      printMessage("Fuellstandsmessung!", "Bitte warten");
      readAllFillLevels();
      setState(STATE_MAIN_MENU);
    }
    //TODO: do stuff needed for the new state
    stateChanged = false;
  }
}

void buttonISR() {
  if(state == STATE_FILL_INFO)
  {
    if(digitalRead(BUTTON_SELECT_1) == HIGH)
    {
      setState(STATE_READ_FILL_LEVEL);
    }
    else if(digitalRead(BUTTON_START_OR_FILL_INFO) == HIGH)
    {
      setState(STATE_MAIN_MENU);
    }
  }
  else if(state == STATE_MAIN_MENU)
  {
    if(digitalRead(BUTTON_START_OR_FILL_INFO) == HIGH)
    {
      setState(STATE_FILL_INFO);
    }
    else if(digitalRead(BUTTON_SELECT_1) == HIGH)
    {
      selectedRecipe = 0;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_2) == HIGH)
    {
      selectedRecipe = 1;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_3) == HIGH)
    {
      selectedRecipe = 2;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_4) == HIGH)
    {
      selectedRecipe = 3;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_5) == HIGH)
    {
      selectedRecipe = 4;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_6) == HIGH)
    {
      selectedRecipe = 5;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_7) == HIGH)
    {
      selectedRecipe = 6;
      setState(STATE_WAITING_GLASS);
    }
    else if(digitalRead(BUTTON_SELECT_8) == HIGH)
    {
      selectedRecipe = 7;
      setState(STATE_WAITING_GLASS);
    }
  }
  else if(state == STATE_WAITING_START && digitalRead(BUTTON_START_OR_FILL_INFO) == HIGH)
  {
    setState(STATE_WORKING);
  }
}

void setTimestamp() {
  timestamp = millis();
}

boolean timeOver(unsigned long timeout) {
  unsigned long current = millis();
  if(current < timestamp || current - timestamp >= timeout)
  {
    timestamp = 0;
    return true;
  }
  return false;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

