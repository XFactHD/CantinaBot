const int TRIGGER_PINS[] { 22, 24, 26, 28, 30, 32 }; //Trigger pins of the ultrasonic distance sensors
const int ECHO_PINS[] { 23, 25, 27, 29, 31, 33 }; //Echo pins of the ultrasonic distance sensors

const float CM_PER_MICROSEC = .03434; //Distance sound travels in air per microsecond

const float DIST_FULL = 0; //Sensor reading when bottle is full
const float DIST_EMPTY = 27; //Sensor reading when bottle is empty

//Configures pins for the ultrasonic distance sensors
void initFillLevelReader() {
  for(int i = 0; i < 6; i++)
  {
    pinMode(TRIGGER_PINS[i], OUTPUT);
    digitalWrite(TRIGGER_PINS[i], LOW);
    pinMode(ECHO_PINS[i], INPUT);
  }
}

//Checks the fill level of all containers after boot and if a certain button combination was pressed
void readAllFillLevels() {
  for(int i = 0; i < 6; i++)
  {
    fillLevels[i] = measureFillLevel(TRIGGER_PINS[i], ECHO_PINS[i]);
  }
}

//Only checks the fill level of ingredients used in the passed recipe
void readFillLevelsPostMixing(int recipe) {
  for(int i = 0; i < getNumberOfIngredients(recipe); i++)
  {
    int ingredient = getIngredient(recipe, i);
    fillLevels[ingredient] = measureFillLevel(TRIGGER_PINS[ingredient], ECHO_PINS[ingredient]);
  }
}

//Measures and calculates the fill level of the container belonging to the trigger and echo pins
int measureFillLevel(int trigger, int echo) {
  float dist = measureDistance(trigger, echo);
  if(dist > DIST_EMPTY) { return 0; }
  if(dist < DIST_FULL) { return 100; }

  int percentage = int(100.0 - mapFloat(dist, DIST_FULL, DIST_EMPTY, 0.0, 100.0));
  percentage = constrain(percentage, 0, 100);
  return percentage;
}

//Measures the distance on the ultrasonic distance sensor belonging to the trigger and echo pins
float measureDistance(int trigger, int echo) {
  digitalWrite(trigger, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigger, LOW);

  delayMicroseconds(250);

  float time = pulseIn(echo, HIGH, 300000);
  time /= 2.0;
  
  float distance = time * CM_PER_MICROSEC;

  return distance;
}

