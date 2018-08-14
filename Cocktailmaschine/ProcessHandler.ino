//Valve, stir motor and stepper driver pins
const int VALVE_INGREDIENT_1 = 34;
const int VALVE_INGREDIENT_2 = 35;
const int VALVE_INGREDIENT_3 = 36;
const int VALVE_INGREDIENT_4 = 37;
const int VALVE_INGREDIENT_5 = 38;
const int VALVE_INGREDIENT_6 = 39;

const int MOTOR_STIR = 4;

const int STEPPER_DISC_DIR = 40;
const int STEPPER_DISC_STEP = 41;
const int STEPPER_DISC_ENABLE = 42;
const int STEPPER_ARM_HOR_DIR = 43;
const int STEPPER_ARM_HOR_STEP = 44;
const int STEPPER_ARM_HOR_ENABLE = 45;
const int STEPPER_ARM_VERT_DIR = 46;
const int STEPPER_ARM_VERT_STEP = 47;
const int STEPPER_ARM_VERT_ENABLE = 48;
const int SWITCH_DISC_ZERO = A7;
const int SWITCH_DISC_POS = A2;
const int SWITCH_ARM_HOR_IN = A3;
const int SWITCH_ARM_HOR_OUT = A4;
const int SWITCH_ARM_VERT_TOP = A5;
const int SWITCH_ARM_VERT_BOTTOM = A6;

//Stepper objects
A4988 stepperDisc(200, STEPPER_DISC_DIR, STEPPER_DISC_STEP, STEPPER_DISC_ENABLE);
A4988 stepperArmHor(200, STEPPER_ARM_HOR_DIR, STEPPER_ARM_HOR_STEP, STEPPER_ARM_HOR_ENABLE);
A4988 stepperArmVert(200, STEPPER_ARM_VERT_DIR, STEPPER_ARM_VERT_STEP, STEPPER_ARM_VERT_ENABLE);

//Valve pins as array
const int VALVES[] { VALVE_INGREDIENT_1, VALVE_INGREDIENT_2, VALVE_INGREDIENT_3, VALVE_INGREDIENT_4, VALVE_INGREDIENT_5, VALVE_INGREDIENT_6 };

const int POS_STIR_ARM = 7; //Position of the stir arm above the table (pos 0 = start, pos 1-6 = ingredients, pos 7 = stir arm)
const float ML_PER_MS = 0.01; //Flow speed in milliliters per millisecond
const int STIR_TIME_MS = 6000; //Time to stir the cocktail
const int STIR_SPEED = 40; //PWM duty cycle for the stir motor

int openValve = -1; //Currently open valve, used to drain the system in a controlled manner

//Configures all pins, initializes the stepper drivers and disables them to conserve energy (and my sanity :D)
void initProcessHandler() {
  for(int i = 0; i < 6; i++)
  {
    pinMode(VALVES[i], OUTPUT);
    digitalWrite(VALVES[i], LOW);
  }
  
  pinMode(SWITCH_DISC_ZERO, INPUT_PULLUP);
  pinMode(SWITCH_DISC_POS, INPUT);
  pinMode(SWITCH_ARM_HOR_IN, INPUT_PULLUP);
  pinMode(SWITCH_ARM_HOR_OUT, INPUT_PULLUP);
  pinMode(SWITCH_ARM_VERT_TOP, INPUT_PULLUP);
  pinMode(SWITCH_ARM_VERT_BOTTOM, INPUT_PULLUP);
  pinMode(MOTOR_STIR, OUTPUT);
  digitalWrite(MOTOR_STIR, LOW);
  stepperDisc.begin();
  stepperDisc.disable();
  stepperArmHor.begin();
  stepperArmHor.disable();
  stepperArmVert.begin();
  stepperArmVert.disable();
}

//Checks if all ingredients for the recipe selected by the user are available in sufficient amounts
boolean checkIngredientsAvailable(int recipe) {
  //------------------------------------------------------------------------------------------------
  #ifdef DEBUG
  for(int i = 0; i < 6; i++) { //DEBUG CODE
    fillLevels[i] = 50;        //Allows operation without liquid
  }                            //remove '#define DEBUG' in Cocktailmaschine.ino for normal operation
  #endif
  //-------------------------------------------------------------------------------------------------
  
  for(int i = 0; i < getNumberOfIngredients(recipe); i++)
  {
    if(fillLevels[getIngredient(recipe, i)] < 10)
    {
      return false;
    }
  }
  return true;
}

//Handles the production of the selected cocktail
void process() {
  int lastIngredient = -1;

  for(int i = 0; i < getNumberOfIngredients(selectedRecipe); i++)
  {
    int ingredient = getIngredient(selectedRecipe, i);
    int amount = getAmountOfIngredient(selectedRecipe, ingredient);

    int positionDelta = ingredient - lastIngredient;
    rotate(positionDelta);

    pourIngredient(ingredient, amount);

    lastIngredient = ingredient;
  }

  rotate(POS_STIR_ARM - (lastIngredient + 1)); //Add 1 to the ingredient index because its internal index ranges from 0 to 5 and its index on the table ranges from  to 6
  
  moveArmAndStir();

  rotateToPosZero();

  setState(STATE_POST_PROCESS);
}

//Homes the turntable (glass holder at the start position)
void rotateToPosZero() {
  if(digitalRead(SWITCH_DISC_ZERO) == LOW) { return; }
  
  stepperDisc.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_DISC_ZERO) == HIGH)
  {
    stepperDisc.move(-1);
    delay(4);
  }
  delay(4);
  stepperDisc.move(-32);
  delay(500);
  stepperDisc.disable(); //Switch stepper driver off
  delay(100);
}

//Rotates the turntable by the amount of positions passed in
void rotate(int positions) {
  int steps = 0;
  int lastHallEffectVal = LOW;
  stepperDisc.enable(); //Switch stepper driver on
  stepperDisc.move(-150); //Get the magnet out of range of the hall effect sensor
  while(steps < positions)
  {
    stepperDisc.move(-1);
    delay(4);
    
    int val = digitalRead(SWITCH_DISC_POS);
    if(val == HIGH && lastHallEffectVal == LOW) {
      lastHallEffectVal = HIGH;
      steps++;
    }
    else if(val == LOW && lastHallEffectVal == HIGH) {
      lastHallEffectVal = LOW;
    }
  }
  stepperDisc.disable(); //Switch stepper driver off
}

//Pours the ingredient at the defined amount into the glass
void pourIngredient(int ingredient, int amount) {
  int time = float(amount) / ML_PER_MS;
  Serial.println(time);
  digitalWrite(VALVES[ingredient], HIGH);
  delay(time);
  digitalWrite(VALVES[ingredient], LOW);
  delay(500);
}

//Opens (or closes if already open) the selected valve to drain the system
void switchValveToDrain(int index) {
  if(openValve == index)
  {
    digitalWrite(VALVES[index], LOW);
    openValve = -1;
    setState(STATE_FILL_INFO);
  }
  else if(openValve == -1)
  {
    digitalWrite(VALVES[index], HIGH);
    openValve = index;
  }
  else
  {
    digitalWrite(VALVES[openValve], LOW);
    digitalWrite(VALVES[index], HIGH);
    openValve = index;
    setState(STATE_DRAINING); //Set the state to draining again to force a display update
  }
}

//Shows the currently open valve on the display
void printDrainInfo() {
  char val[2] = "";
  itoa(openValve + 1, val, 10);
  val[1] = '\0';
  printMessageMultiArg("Draining!", " ", "Valve: ", val);
}

//Homes the stir arm (outside the radius of the turntable, spoon in the washing container)
void moveArmToHome() {
  if(digitalRead(SWITCH_ARM_HOR_OUT) == HIGH)
  {
    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_TOP) == HIGH)
    {
      stepperArmVert.move(-1);
      delay(5);
    }
    stepperArmVert.disable(); //Switch stepper driver off
    
    delay(100);
    
    stepperArmHor.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_HOR_OUT) == HIGH)
    {
      stepperArmHor.move(1);
      delay(5);
    }
    stepperArmHor.move(10);
    stepperArmHor.disable(); //Switch stepper driver off
    
    delay(100);

    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == HIGH)
    {
      stepperArmVert.move(1);
      delay(5);
    }
    stepperArmVert.disable(); //Switch stepper driver off
  }
  else
  {
    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == HIGH)
    {
      stepperArmVert.move(1);
      delay(5);
    }
    stepperArmVert.disable(); //Switch stepper driver off
  }
  delay(100);
}

//Moves the arm into the glass, stirs the cocktail and moves the arm back to the washing container
void moveArmAndStir() {
  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_TOP) == HIGH)
  {
    stepperArmVert.move(-1);
    delay(5);
  }
  stepperArmVert.disable(); //Switch stepper driver off
  
  delay(100);

  stepperArmHor.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_HOR_IN) == HIGH)
  {
    stepperArmHor.move(-1);
    delay(5);
  }
  stepperArmHor.move(-5);
  stepperArmHor.disable(); //Switch stepper driver off
  
  delay(100);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == HIGH)
  {
    stepperArmVert.move(1);
    delay(5);
  }
  stepperArmVert.disable(); //Switch stepper driver off
  delay(100);

  analogWrite(MOTOR_STIR, STIR_SPEED);
  delay(STIR_TIME_MS);
  analogWrite(MOTOR_STIR, 0);
  delay(500);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_TOP) == HIGH)
  {
    stepperArmVert.move(-1);
    delay(5);
  }
  stepperArmVert.disable(); //Switch stepper driver off
  
  delay(100);

  stepperArmHor.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_HOR_OUT) == HIGH)
  {
    stepperArmHor.move(1);
    delay(5);
  }
  stepperArmHor.move(10);
  //Keep the horizontal stepper driver on to make sure the arm doesn't get moved out of the parking position by the vibrations from the stir motor
  
  delay(100);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == HIGH)
  {
    stepperArmVert.move(1);
    delay(5);
  }
  stepperArmVert.disable(); //Switch stepper driver off
  
  delay(100);

  //Cleaning in a glass of water
  analogWrite(MOTOR_STIR, STIR_SPEED);
  delay(2000);
  analogWrite(MOTOR_STIR, 0);

  stepperArmHor.disable(); //Switch stepper driver off
}

