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
const int SWITCH_DISC_ZERO = A1;
const int SWITCH_DISC_POS = A2;
const int SWITCH_ARM_HOR_IN = A3;
const int SWITCH_ARM_HOR_OUT = A4;
const int SWITCH_ARM_VERT_TOP = A5;
const int SWITCH_ARM_VERT_BOTTOM = A6;

A4988 stepperDisc(200, STEPPER_DISC_DIR, STEPPER_DISC_STEP, STEPPER_DISC_ENABLE);
A4988 stepperArmHor(200, STEPPER_ARM_HOR_DIR, STEPPER_ARM_HOR_STEP, STEPPER_ARM_HOR_ENABLE);
A4988 stepperArmVert(200, STEPPER_ARM_VERT_DIR, STEPPER_ARM_VERT_STEP, STEPPER_ARM_VERT_ENABLE);

const int VALVES[] { VALVE_INGREDIENT_1, VALVE_INGREDIENT_2, VALVE_INGREDIENT_3, VALVE_INGREDIENT_4, VALVE_INGREDIENT_5, VALVE_INGREDIENT_6 };

const int POS_STIR_ARM = 7; //Position of the stir arm above the table (pos 0 = start, pos 1-6 = ingredients, pos 7 = stir arm)
const float ML_PER_MS_MIN_SMALL = 1; //Flow speed through s small valve in milliliters per millisecond when the bottle is almost empty
const float ML_PER_MS_MAX_SMALL = 1; //Flow speed through a small valve in milliliters per millisecond when the bottle is full
const float ML_PER_MS_MIN_BIG = 1; //Flow speed through s big valve in milliliters per millisecond when the bottle is almost empty
const float ML_PER_MS_MAX_BIG = 1; //Flow speed through a big valve in milliliters per millisecond when the bottle is full
const int STIR_TIME_MS = 6000; //Time to stir the cocktail
const int STIR_SPEED = 200; //PWM duty cycle for the stir motor

volatile boolean moving = false;
volatile int steps = 0;

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
  for(int i = 0; i < 6; i++) { //TODO: remove after testing
    fillLevels[i] = 50;
  }
  
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

  rotate(POS_STIR_ARM - lastIngredient);

  moveArmAndStir();

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
  stepperDisc.disable(); //Switch stepper driver off
  delay(100);
}

//Rotates the turntable by the amount of stations passed in
void rotate(int positions) {
  moving = true;
  stepperDisc.enable(); //Switch stepper driver on
  while(steps < positions)
  {
    stepperDisc.move(-1);
  }
  stepperDisc.disable(); //Switch stepper driver off
  moving = false;
  steps = 0;
}

//Pours the ingredient at the defined amount into the glass
void pourIngredient(int ingredient, int amount) {
  float mlPerMs = 0;
  if(ingredient == 1 || ingredient == 5) //Ingredient 1 and 5 are using a valve with a bigger nozzle
  {
    mlPerMs = mapFloat(fillLevels[ingredient], 10, 100, ML_PER_MS_MIN_BIG, ML_PER_MS_MAX_BIG);
  }
  else
  {
    mlPerMs = mapFloat(fillLevels[ingredient], 10, 100, ML_PER_MS_MIN_SMALL, ML_PER_MS_MAX_SMALL);
  }
  
  digitalWrite(VALVES[ingredient], HIGH);
  int time = float(amount) / mlPerMs;
  delay(time);
  digitalWrite(VALVES[ingredient], LOW);
  delay(500);
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
    stepperArmVert.move(-1);
    delay(5);
  }
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
    stepperArmVert.move(1);
    delay(5);
  }
  stepperArmVert.disable(); //Switch stepper driver off
  
  delay(100);

  stepperArmHor.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_HOR_OUT) == HIGH)
  {
    stepperArmVert.move(-1);
    delay(5);
  }
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

  //Cleaning in a glass of water
  digitalWrite(MOTOR_STIR, HIGH);
  delay(2000);
  digitalWrite(MOTOR_STIR, LOW);
}

//Called when the hall effect sensor triggers the hardware interrupt
void switchISR() {
  if(moving)
  {
    if(digitalRead(SWITCH_DISC_POS) == HIGH)
    {
      steps++;
    }
  }
}

