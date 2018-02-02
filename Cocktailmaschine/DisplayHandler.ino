//LCD pins
const int LCD_RS = A10;
const int LCD_EN = A11;
const int LCD_D4 = A12;
const int LCD_D5 = A13;
const int LCD_D6 = A14;
const int LCD_D7 = A15;

//Pixel definitions for custom characters
const byte arrowRight[8] = {
  B10000,
  B11000,
  B11100,
  B11110,
  B11100,
  B11000,
  B10000,
};

const byte arrowLeft[8] = {
  B00001,
  B00011,
  B00111,
  B01111,
  B00111,
  B00011,
  B00001,
};

const byte empty[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

const byte ue[8] = {
  B01010,
  B00000,
  B10001,
  B10001,
  B10001,
  B10011,
  B01101,
};

const byte oe[8] = {
  B01010,
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
};

const byte ae[8] = {
  B01010,
  B00000,
  B01110,
  B00001,
  B01111,
  B10001,
  B01111,
};

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//Initializes the display, clears it and creates the custom characters
void initDisplay() {
  lcd.begin(20, 4);
  lcd.clear();
  lcd.createChar(0, arrowLeft);
  lcd.createChar(1, arrowRight);
  lcd.createChar(2, empty);
  lcd.createChar(3, ue);
  lcd.createChar(4, oe);
  lcd.createChar(5, ae);
}

//Prints all recipes to the display
void showRecipes() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.print(recipeNames[0]);
  int spaces = getRecipeSpaces(recipeNames[0], recipeNames[1]);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  lcd.print(recipeNames[1]);
  lcd.write(byte(1));

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.print(recipeNames[2]);
  spaces = getRecipeSpaces(recipeNames[2], recipeNames[3]);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  lcd.print(recipeNames[3]);
  lcd.write(byte(1));

  lcd.setCursor(0, 2);
  lcd.write(byte(0));
  lcd.print(recipeNames[4]);
  spaces = getRecipeSpaces(recipeNames[4], recipeNames[5]);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  lcd.print(recipeNames[5]);
  lcd.write(byte(1));

  lcd.setCursor(0, 3);
  lcd.write(byte(0));
  lcd.print(recipeNames[6]);
  spaces = getRecipeSpaces(recipeNames[6], recipeNames[7]);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  lcd.print(recipeNames[7]);
  lcd.write(byte(1));
}

//Prints the fill levels of the ingredient bottles to the display
void showFillLevels() {
  lcd.setCursor(0, 0);
  printCString(" Fuellstandsanzeige: ", false);
  
  lcd.setCursor(0, 1);
  char* s = ingredientNames[0];
  lcd.print(s);
  lcd.print(":");
  int spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  int level = fillLevels[0];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");

  s = ingredientNames[1];
  lcd.print(s);
  lcd.print(":");
  spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  level = fillLevels[1];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");

  lcd.setCursor(0, 2);
  s = ingredientNames[2];
  lcd.print(s);
  lcd.print(":");
  spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  level = fillLevels[2];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");

  s = ingredientNames[3];
  lcd.print(s);
  lcd.print(":");
  spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  level = fillLevels[3];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");
  
  lcd.setCursor(0, 3);
  s = ingredientNames[4];
  lcd.print(s);
  lcd.print(":");
  spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  level = fillLevels[4];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");

  s = ingredientNames[5];
  lcd.print(s);
  lcd.print(":");
  spaces = 5 - strlen(s);
  for(int i = 0; i < spaces; i++)
  {
    lcd.print(" ");
  }
  level = fillLevels[5];
  if(level < 100) { lcd.print(" "); }
  if(level <  10) { lcd.print(" "); }
  lcd.print(level);
  lcd.print("%");
}

//Prints an error message to the display
void printError(int code) {
  char* lineOne = "INTERNAL ERROR";
  char* line = code < 10 ? "CODE: 0x0" : "CODE: 0x";
  String lineTwo = String(line) + String(code, HEX);
  printMessage(lineOne, lineTwo.c_str());
}

//Prints the two lines of text passed to it to the display
void printMessage(const char* lineOne, const char* lineTwo) {
  if(strlen(lineOne) > 18 || strlen(lineTwo) > 18)
  {
    printError(ERROR_STRING_TOO_LONG);
    return;
  }
  
  int spacesOne = 18 - strlen(lineOne);
  int spacesTwo = 18 - strlen(lineTwo);
  
  lcd.setCursor(0, 0);
  lcd.print("+------------------+");

  lcd.setCursor(0, 1);
  lcd.print("|");
  int partOne = spacesOne / 2;
  for(int i = 0; i < partOne; i++)
  {
    lcd.print(" ");
  }
  printCString(lineOne, true);
  for(int i = 0; i < spacesOne - partOne; i++)
  {
    lcd.print(" ");
  }
  lcd.print("|");

  lcd.setCursor(0, 2);
  lcd.print("|");
  int partTwo = spacesTwo / 2;
  for(int i = 0; i < partTwo; i++)
  {
    lcd.print(" ");
  }
  printCString(lineTwo, true);
  for(int i = 0; i < spacesTwo - partTwo; i++)
  {
    lcd.print(" ");
  }
  lcd.print("|");

  lcd.setCursor(0, 3);
  lcd.print("+------------------+");
}

//Same as above, "concatenates" the two parameters passed to it
void printMessageMultiArg(const char* lineOneOne, const char* lineOneTwo, const char* lineTwoOne, const char* lineTwoTwo) {
  if(strlen(lineOneOne) + strlen(lineOneTwo) > 18 || strlen(lineTwoOne) + strlen(lineTwoTwo) > 18)
  {
    printError(ERROR_STRING_TOO_LONG);
    return;
  }
  
  int spacesOne = 18 - strlen(lineOneOne) - strlen(lineOneTwo);
  int spacesTwo = 18 - strlen(lineTwoOne) - strlen(lineTwoTwo);

  lcd.setCursor(0, 0);
  lcd.print("+------------------+");

  lcd.setCursor(0, 1);
  lcd.print("|");
  int partOne = spacesOne / 2;
  for(int i = 0; i < partOne; i++)
  {
    lcd.print(" ");
  }
  printCStringMulti(lineOneOne, lineOneTwo, true);
  for(int i = 0; i < spacesOne - partOne; i++)
  {
    lcd.print(" ");
  }
  lcd.print("|");

  lcd.setCursor(0, 2);
  lcd.print("|");
  int partTwo = spacesTwo / 2;
  for(int i = 0; i < partTwo; i++)
  {
    lcd.print(" ");
  }
  printCStringMulti(lineTwoOne, lineTwoTwo, true);
  for(int i = 0; i < spacesTwo - partTwo; i++)
  {
    lcd.print(" ");
  }
  lcd.print("|");

  lcd.setCursor(0, 3);
  lcd.print("+------------------+");
}

//Prints the string passed to it to the display, centers the string if 'centered' is true, replaces certain character combinations with custom chars (ae --> ä; oe --> ö; ue --> ü)
void printCString(const char* s, boolean centered) {
  int i = 0;
  int skips = 0;
  while(s[i] != '\0')
  {
    char c = s[i];
    if((c == 'a' || c == 'o' || c == 'u') && s[i + 1] == 'e')
    {
      skips++;
    }
    i++;
  }
  i = 0;

  int startSpaces = centered ? skips / 2 : 0;
  if(centered && (strlen(s) - skips) % 2 == 0)
  {
    startSpaces = max(startSpaces, skips - startSpaces);
  }
  for(int j = 0; j < startSpaces; j++)
  {
    lcd.print(' ');
  }
  
  boolean skip = false;
  while(s[i] != '\0')
  {
    if(skip)
    {
      skip = false;
      skips++;
      i++;
      continue;
    }
    
    char c = s[i];
    if(c == 'u' && s[i + 1] == 'e')
    {
      lcd.write(byte(3));
      skip = true;
    }
    else if(c == 'o' && s[i + 1] == 'e')
    {
      lcd.write(byte(4));
      skip = true;
    }
    else if(c == 'a' && s[i + 1] == 'e')
    {
      lcd.write(byte(5));
      skip = true;
    }
    else
    {
      lcd.print(s[i]);
    }
    i++;
  }

  int endSpaces = skips - startSpaces;
  for(int j = 0; j < endSpaces; j++)
  {
    lcd.print(' ');
  }
}

//Same as above, "concatenates" the two parameters passed to it
void printCStringMulti(const char* s1, const char* s2, boolean centered) {
  int i = 0;
  int skips = 0;
  while(s1[i] != '\0')
  {
    char c = s1[i];
    if((c == 'a' || c == 'o' || c == 'u') && s1[i + 1] == 'e')
    {
      skips++;
    }
    i++;
  }
  i = 0;

  while(s2[i] != '\0')
  {
    char c = s2[i];
    if((c == 'a' || c == 'o' || c == 'u') && s2[i + 1] == 'e')
    {
      skips++;
    }
    i++;
  }
  i = 0;

  int startSpaces = centered ? skips / 2 : 0;
  if(centered && (strlen(s1) + strlen(s2) - skips) % 2 == 0)
  {
    startSpaces = max(startSpaces, skips - startSpaces);
  }
  for(int j = 0; j < startSpaces; j++)
  {
    lcd.print(' ');
  }
  
  boolean skip = false;
  while(s1[i] != '\0')
  {
    if(skip)
    {
      skip = false;
      i++;
      continue;
    }
    
    char c = s1[i];
    if(c == 'u' && s1[i + 1] == 'e')
    {
      lcd.write(byte(3));
      skip = true;
    }
    else if(c == 'o' && s1[i + 1] == 'e')
    {
      lcd.write(byte(4));
      skip = true;
    }
    else if(c == 'a' && s1[i + 1] == 'e')
    {
      lcd.write(byte(5));
      skip = true;
    }
    else
    {
      lcd.print(c);
    }
    i++;
  }
  i = 0;
  skip = false;
  
  while(s2[i] != '\0')
  {
    if(skip)
    {
      skip = false;
      i++;
      continue;
    }
    
    char c = s2[i];
    if(c == 'u' && s2[i + 1] == 'e')
    {
      lcd.write(byte(3));
      skip = true;
    }
    else if(c == 'o' && s2[i + 1] == 'e')
    {
      lcd.write(byte(4));
      skip = true;
    }
    else if(c == 'a' && s2[i + 1] == 'e')
    {
      lcd.write(byte(5));
      skip = true;
    }
    else
    {
      lcd.print(c);
    }
    i++;
  }

  int endSpaces = skips - startSpaces;
  for(int j = 0; j < endSpaces; j++)
  {
    lcd.print(' ');
  }
}

//Calculates the amount of spaces between two recipes in one line in the main menu
int getRecipeSpaces(const char* r1, const char* r2) {
  return 18 - (strlen(r1) + strlen(r2));
}

