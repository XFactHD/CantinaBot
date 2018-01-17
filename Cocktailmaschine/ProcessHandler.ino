const int VALVE_INGREDIENT_1 = 0;
const int VALVE_INGREDIENT_2 = 0;
const int VALVE_INGREDIENT_3 = 0;
const int VALVE_INGREDIENT_4 = 0;
const int VALVE_INGREDIENT_5 = 0;
const int VALVE_INGREDIENT_6 = 0;
const int MOTOR_STIR = 0;
const int STEPPER_DISC_DIR = 0;
const int STEPPER_DISC_STEP = 0;
const int STEPPER_ARM_HOR_DIR = 0;
const int STEPPER_ARM_HOR_STEP = 0;
const int STEPPER_ARM_VERT_DIR = 0;
const int STEPPER_ARM_VERT_STEP = 0;

A4988 stepperDisc(200, STEPPER_DISC_DIR, STEPPER_DISC_STEP);
A4988 stepperArmHor(200, STEPPER_ARM_HOR_DIR, STEPPER_ARM_HOR_STEP);
A4988 stepperArmVert(200, STEPPER_ARM_VERT_DIR, STEPPER_ARM_VERT_STEP);

const int VALVES[] { VALVE_INGREDIENT_1, VALVE_INGREDIENT_2, VALVE_INGREDIENT_3, VALVE_INGREDIENT_4, VALVE_INGREDIENT_5, VALVE_INGREDIENT_6 };

const int POS_STIR_ARM = 7;
const float ML_PER_MS_MIN = 0;
const float ML_PER_MS_MAX = 0; 

volatile boolean homing = false;
volatile boolean home = false;
volatile int steps = 0;

void initProcessHandler() {
  pinMode(VALVE_INGREDIENT_1, OUTPUT);
  pinMode(VALVE_INGREDIENT_2, OUTPUT);
  pinMode(VALVE_INGREDIENT_3, OUTPUT);
  pinMode(VALVE_INGREDIENT_4, OUTPUT);
  pinMode(VALVE_INGREDIENT_5, OUTPUT);
  pinMode(VALVE_INGREDIENT_6, OUTPUT);
  pinMode(MOTOR_STIR, OUTPUT);
  stepperDisc.begin();
  stepperArmHor.begin();
  stepperArmVert.begin();
}

boolean checkIngredientsAvailable(Recipe r) {
  for(int i = 0; i < r.getCount(); i++)
  {
    if(fillLevels[r.getIngredient(i)] < 10)
    {
      return false;
    }
  }
  return true;
}

void process() {
  Recipe r = recipes[selectedRecipe];
  
  int lastIngredient = -1;
  for(int i = 0; i < r.getCount(); i++)
  {
    int ingredient = r.getIngredient(i);
    int amount = r.getAmountOf(ingredient);

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
  while(!home)
  {
    stepperDisc.move(1);
    delay(20);
  }
  homing = false;
  home = false;
  delay(100);
}

void rotate(int positions) {
  int dir = positions > 0 ? 1 : -1;
  while(steps < abs(positions))
  {
    stepperDisc.move(dir);
  }
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
    while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
    {
      stepperArmVert.move(1);
      delay(20);
    }
    delay(100);
    while(digitalRead(SWITCH_ARM_HOR_OUT) == LOW)
    {
      stepperArmHor.move(-1);
      delay(20);
    }
    delay(100);
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
    {
      stepperArmVert.move(-1);
      delay(20);
    }
  }
  else
  {
    while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
    {
      stepperArmVert.move(-1);
      delay(20);
    }
  }
  delay(100);
}

void moveArmAndStir() {
  while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  delay(100);
  while(digitalRead(SWITCH_ARM_HOR_IN) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  delay(100);
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
  delay(100);

  digitalWrite(MOTOR_STIR, HIGH);
  delay(STIR_TIME_MS);
  digitalWrite(MOTOR_STIR, LOW);
  delay(500);
  
  while(digitalRead(SWITCH_ARM_VERT_TOP) == LOW)
  {
    stepperArmVert.move(1);
    delay(20);
  }
  delay(100);
  while(digitalRead(SWITCH_ARM_HOR_OUT) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
  delay(100);
  while(digitalRead(SWITCH_ARM_VERT_BOTTOM) == LOW)
  {
    stepperArmVert.move(-1);
    delay(20);
  }
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
  else
  {
    if(digitalRead(SWITCH_DISC_POS_COUNT) == HIGH)
    {
      steps++;
    }
  }
}

