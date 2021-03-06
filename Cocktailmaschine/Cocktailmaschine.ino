#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <A4988.h>

//#define DEBUG

//BUTTON PINS
const int BUTTON_INTERRUPT = 2;
const int BUTTON_START_OR_FILL_INFO = 5;
const int BUTTON_SELECT_1 = 6;
const int BUTTON_SELECT_2 = 7;
const int BUTTON_SELECT_3 = 8;
const int BUTTON_SELECT_4 = 9;
const int BUTTON_SELECT_5 = 10;
const int BUTTON_SELECT_6 = 11;
const int BUTTON_SELECT_7 = 12;
const int BUTTON_SELECT_8 = 13;

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
const int STATE_DRAINING = 10;

char recipeNames[8][9] {"RumCola", "WodkaHB", "WhiskyHB", "Daiquiri", "WhiskySo", "WodkaSou", "Whisky", "Wodka"}; //Names of the cocktails, max string length is 8 (extra space is for null character)!
int recipeIngredientCounts[8] {2, 2, 2, 3, 3, 3, 1, 1}; //Number of ingredients of the cocktails
int recipeIngredients[8][6] { //Ingredients of the cocktails
  {0,  1, -1, -1, -1, -1},
  {1,  2, -1, -1, -1, -1},
  {1,  3, -1, -1, -1, -1},
  {0,  4,  5, -1, -1, -1},
  {3,  4,  5, -1, -1, -1},
  {2,  4,  5, -1, -1, -1},
  {3, -1, -1, -1, -1, -1},
  {2, -1, -1, -1, -1, -1} };
int recipeIngredientAmounts[8][6] { //Amount of each ingredient of a cocktail
  { 45, 135, -1, -1, -1, -1},
  { -1, 135, 45, -1, -1, -1},
  { -1, 135, -1, 45, -1, -1},
  { 50,  25, 25, -1, -1, -1},
  { 50,  25, 25, -1, -1, -1},
  { 50,  25, 25, -1, -1, -1},
  { 50,  -1, -1, -1, -1, -1},
  { 50,  -1, -1, -1, -1, -1} };

char ingredientNames[6][6] { //Names of the ingredients, max string length is 5 (extra space is for null character)!
  "Rum",
  "Cola",
  "Vodka",
  "Whisk",
  "Lime",
  "Sirup"
};

int fillLevels[6] { 0, 0, 0, 0, 0, 0 }; //Fill levels of the six containers
volatile int lastState = 0; //Last state of the state machine
volatile int state = 0; //Current state of the state machine
volatile boolean stateChanged = false; //Indicates if the state of the state machine changed
volatile int selectedRecipe = -1; //The recipe currently selected and being worked on
unsigned long timestamp = 0; //Used by certain states for a timeout

void setup() {
  Serial.begin(9600);
  initDisplay();
  setState(STATE_INIT);
  printMessage("INITIALIZATION", "PLEASE WAIT");
  initFillLevelReader();
  initGlassChecker();
  initProcessHandler();
  //Configure pins
  pinMode(BUTTON_START_OR_FILL_INFO, INPUT);
  pinMode(BUTTON_SELECT_1, INPUT);
  pinMode(BUTTON_SELECT_2, INPUT);
  pinMode(BUTTON_SELECT_3, INPUT);
  pinMode(BUTTON_SELECT_4, INPUT);
  pinMode(BUTTON_SELECT_5, INPUT);
  pinMode(BUTTON_SELECT_6, INPUT);
  pinMode(BUTTON_SELECT_7, INPUT);
  pinMode(BUTTON_SELECT_8, INPUT);
  readFromEEPROM();
  waitForSerialComm();
  if(state == STATE_SERIAL_COMM)
  {
    printMessage("SERIAL ACTIVE", "WAITING FOR START");
    return;
  }
  //Attach interrupts for user input buttons
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT), buttonISR, RISING);
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

//Sets the state of the state machine and marks it for checking in the loop method
void setState(int newState) {
  lastState = state;
  state = newState;
  stateChanged = true;
}

//Called from loop(), checks if the state of the state machine changed and takes actions according to the new state
void checkStateChange() {
  if(stateChanged)
  {
    stateChanged = false;
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
      if(checkIngredientsAvailable(selectedRecipe))
      {
        setTimestamp();
      }
      else
      {
        printMessage("Zutaten", "unzureichend!");
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
      readFillLevelsPostMixing(selectedRecipe);
      setState(STATE_MAIN_MENU);
    }
    else if(state == STATE_READ_FILL_LEVEL)
    {
      printMessage("Fuellstandsmessung!", "Bitte warten");
      readAllFillLevels();
      setState(STATE_MAIN_MENU);
    }
    else if(state == STATE_DRAINING)
    {
      printDrainInfo();
    }
  }
}

//Called when a button press by the user triggers the hardware interrupt
void buttonISR() {
  if(state == STATE_FILL_INFO || state == STATE_DRAINING)
  {
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

    if(digitalRead(BUTTON_SELECT_3) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(0);
    }
    else if(digitalRead(BUTTON_SELECT_4) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(1);
    }
    else if(digitalRead(BUTTON_SELECT_5) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(2);
    }
    else if(digitalRead(BUTTON_SELECT_6) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(3);
    }
    else if(digitalRead(BUTTON_SELECT_7) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(4);
    }
    else if(digitalRead(BUTTON_SELECT_8) == HIGH)
    {
      setState(STATE_DRAINING);
      switchValveToDrain(5);
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

//Sets a timestamp for a timeout
void setTimestamp() {
  timestamp = millis();
}

//Checks if the timeout has been reached
boolean timeOver(unsigned long timeout) {
  unsigned long current = millis();
  if(current < timestamp || current - timestamp >= timeout)
  {
    timestamp = 0;
    return true;
  }
  return false;
}

//Maps a float value from one range into another (map() only uses integer maths which causes extreme loss of precision)
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

