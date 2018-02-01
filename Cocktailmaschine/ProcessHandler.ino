const int VALVE_INGREDIENT_1 = 0;
const int VALVE_INGREDIENT_2 = 0;
const int VALVE_INGREDIENT_3 = 0;
const int VALVE_INGREDIENT_4 = 0;
const int VALVE_INGREDIENT_5 = 0;
const int VALVE_INGREDIENT_6 = 0;
const int MOTOR_STIR = 0;
const int STEPPER_DISC_DIR = 0;
const int STEPPER_DISC_STEP = 0;
const int STEPPER_DISC_ENABLE = 0;
const int STEPPER_ARM_HOR_DIR = 0;
const int STEPPER_ARM_HOR_STEP = 0;
const int STEPPER_ARM_HOR_ENABLE = 0;
const int STEPPER_ARM_VERT_DIR = 0;
const int STEPPER_ARM_VERT_STEP = 0;
const int STEPPER_ARM_VERT_ENABLE = 0;

A4988 stepperDisc(200, STEPPER_DISC_DIR, STEPPER_DISC_STEP, STEPPER_DISC_ENABLE); //TODO: place pullup resistors on the enable pins
A4988 stepperArmHor(200, STEPPER_ARM_HOR_DIR, STEPPER_ARM_HOR_STEP, STEPPER_ARM_HOR_ENABLE);
A4988 stepperArmVert(200, STEPPER_ARM_VERT_DIR, STEPPER_ARM_VERT_STEP, STEPPER_ARM_VERT_ENABLE);

const int VALVES[] { VALVE_INGREDIENT_1, VALVE_INGREDIENT_2, VALVE_INGREDIENT_3, VALVE_INGREDIENT_4, VALVE_INGREDIENT_5, VALVE_INGREDIENT_6 };

const int POS_STIR_ARM = 7;
const float ML_PER_MS_MIN = 1;
const float ML_PER_MS_MAX = 1; 
const int STIR_TIME_MS = 6000;
const int STIR_SPEED = 200;

volatile boolean homing = false;
volatile boolean moving = false;
volatile boolean home = false;
volatile int steps = 0;

void initProcessHandler() {
  for(int i = 0; i < 6; i++)
  {
    pinMode(VALVES[i], OUTPUT);
    digitalWrite(VALVES[i], LOW);
  }
  pinMode(MOTOR_STIR, OUTPUT);
  digitalWrite(MOTOR_STIR, LOW);
  stepperDisc.begin();
  stepperDisc.disable();
  stepperArmHor.begin();
  stepperArmHor.disable();
  stepperArmVert.begin();
  stepperArmVert.disable();
}

void initBaseRecipes() { //TODO: remove after testing
  for(int i = 0; i < 8; i++) {
    setIngredient(i, 0, 0);
    setAmountOfIngredient(i, 0, 25);
    setIngredient(i, 1, 1);
    setAmountOfIngredient(i, 1, 25);
  }
}

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

void rotateToPosZero() {
  if(digitalRead(SWITCH_DISC_POS_ZERO) == HIGH)
  {
    return;
  }
  homing = true;
  stepperDisc.enable(); //Switch stepper driver on
  while(!home)
  {
    stepperDisc.move(1);
    delay(4);
  }
  stepperDisc.disable(); //Switch stepper driver off
  homing = false;
  home = false;
  delay(100);
}

void rotate(int positions) { //TODO: rewrite to use a predefined amount of stepper steps
  moving = true;
  stepperDisc.enable(); //Switch stepper driver on
  while(steps < positions)
  {
    stepperDisc.move(1);
  }
  stepperDisc.disable(); //Switch stepper driver off
  moving = false;
  steps = 0;
}

void pourIngredient(int ingredient, int amount) {
  digitalWrite(VALVES[ingredient], HIGH);
  float mlPerMs = mapFloat(fillLevels[ingredient], 10, 100, ML_PER_MS_MIN, ML_PER_MS_MAX);
  int time = float(amount) / mlPerMs;
  delay(time);
  digitalWrite(VALVES[ingredient], LOW);
  delay(500);
}

void moveArmToHome() {
  if(digitalRead(SWITCH_ARM_HOR_OUT) == LOW)
  {
    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
    {
      stepperArmVert.move(1);
      delay(20);
    }
    stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
    
    delay(100);
    
    stepperArmHor.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_HOR_OUT) == LOW)
    {
      stepperArmHor.move(-1);
      delay(20);
    }
    stepperArmHor.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
    
    delay(100);

    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
    {
      stepperArmVert.move(-1);
      delay(20);
    }
    stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  }
  else
  {
    stepperArmVert.enable(); //Switch stepper driver on
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
    {
      stepperArmVert.move(-1);
      delay(20);
    }
    stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  }
  delay(100);
}

void moveArmAndStir() {
  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  
  delay(100);

  stepperArmHor.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_HOR_IN) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  stepperArmHor.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  
  delay(100);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
  stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  delay(100);

  analogWrite(MOTOR_STIR, STIR_SPEED);
  delay(STIR_TIME_MS);
  analogWrite(MOTOR_STIR, 0);
  delay(500);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  
  delay(100);

  stepperArmHor.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_HOR_OUT) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
  stepperArmHor.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  
  delay(100);

  stepperArmVert.enable(); //Switch stepper driver on
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
  stepperArmVert.disable(); //Switch stepper driver on //TODO: check if this is doable or if the arm would just fall down
  
  delay(100);

  //Cleaning in a glass of water
  digitalWrite(MOTOR_STIR, HIGH);
  delay(2000);
  digitalWrite(MOTOR_STIR, LOW);
}

void switchISR() {
  if(homing)
  {
    if(digitalRead(SWITCH_DISC_POS_ZERO) == HIGH)
    {
      home = true;
    }
  }
  else if(moving)
  {
    if(digitalRead(SWITCH_DISC_POS_COUNT) == HIGH)
    {
      steps++;
    }
  }
}

