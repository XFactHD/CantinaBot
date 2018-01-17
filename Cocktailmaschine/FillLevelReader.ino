const int TRIGGER_PINS[] { 39, 0, 0, 0, 0, 0 };
const int ECHO_PINS[] { 37, 0, 0, 0, 0, 0 };

const float CM_PER_MICROSEC = .03434;

const float BOTTLE_HEIGHT = 30;
const int DIST_MIN = 0; //Sensor's minimum distance + safety distance to fluid

void initFillLevelReader() {
  for(int i = 0; i < 6; i++)
  {
    pinMode(TRIGGER_PINS[i], OUTPUT);
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
void readFillLevelsPostMixing(Recipe r) {
  for(int i = 0; i < r.getCount(); i++)
  {
    int ingredient = r.getIngredient(i);
    fillLevels[ingredient] = measureFillLevel(TRIGGER_PINS[ingredient], ECHO_PINS[ingredient]);
  }
  
  setState(STATE_MAIN_MENU);
}

int measureFillLevel(int trigger, int echo) {
  float dist = measureDistance(trigger, echo);
  if(dist > BOTTLE_HEIGHT)
  {
    return 0;
  }

  int percentage = 100 - ((dist / BOTTLE_HEIGHT) * 100.0);
  percentage = constrain(percentage, 0, 100);
  return percentage;
}

float measureDistance(int trigger, int echo) {
  digitalWrite(trigger, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigger, LOW);

  delayMicroseconds(250);

  float time = pulseIn(echo, HIGH, 300000);
  time /= 2.0;
  if(time == 0)
  {
    return BOTTLE_HEIGHT + 1;
  }
  
  float distance = time * CM_PER_MICROSEC;
  distance -= float(DIST_MIN);
  return distance;
}

