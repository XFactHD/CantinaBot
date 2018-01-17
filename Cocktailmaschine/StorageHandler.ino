const int EEPROM_CHECK_SUM = 100;

const int ADDRESS_CHECK_SUM = 0;
const int ADDRESS_RECIPE_1 = ADDRESS_CHECK_SUM + sizeof(int);
const int ADDRESS_RECIPE_2 = ADDRESS_RECIPE_1 + sizeof(Recipe);
const int ADDRESS_RECIPE_3 = ADDRESS_RECIPE_2 + sizeof(Recipe);
const int ADDRESS_RECIPE_4 = ADDRESS_RECIPE_3 + sizeof(Recipe);
const int ADDRESS_RECIPE_5 = ADDRESS_RECIPE_4 + sizeof(Recipe);
const int ADDRESS_RECIPE_6 = ADDRESS_RECIPE_5 + sizeof(Recipe);
const int ADDRESS_RECIPE_7 = ADDRESS_RECIPE_6 + sizeof(Recipe);
const int ADDRESS_RECIPE_8 = ADDRESS_RECIPE_7 + sizeof(Recipe);
const int ADDRESS_OFFSET_LOOKUP = ADDRESS_RECIPE_8 + sizeof(Recipe);

const int ADDRESSES_RECIPE[] { ADDRESS_RECIPE_1, ADDRESS_RECIPE_2, ADDRESS_RECIPE_3, ADDRESS_RECIPE_4, ADDRESS_RECIPE_5, ADDRESS_RECIPE_6, ADDRESS_RECIPE_7, ADDRESS_RECIPE_8 };

boolean checkSumWritten = false;

//Called when the MCU boots up
void readFromEEPROM() {
  int checkSum = 0;
  EEPROM.get(ADDRESS_CHECK_SUM, checkSum); //Read our checksum from the EEPROM to check if it contains data
  checkSumWritten = checkSum == EEPROM_CHECK_SUM;
  if(!checkSumWritten) { return; } //We never wrote anything to the EEPROM or a format change was made, so there is no need to read anything

  for(int i = 0; i < 8; i++)
  {
    EEPROM.get(ADDRESSES_RECIPE[i], recipes[i]); //Read recipes from the EEPROM
  }

  int offsets[14];
  EEPROM.get(ADDRESS_OFFSET_LOOKUP, offsets); //Read the table of string lengths from the EEPROM

  int currentAddress = ADDRESS_OFFSET_LOOKUP + sizeof(offsets); //Address of the first string entry
  for(int i = 0; i < 8; i++)
  {
    char s[offsets[i]];
    readCStringFromEEPROM(currentAddress, s, offsets[i]);
    memcpy(recipeNames[i], s, sizeof(s));
    currentAddress += offsets[i];
  }

  for(int i = 0; i < 6; i++)
  {
    char s[offsets[i + 8]]; //Shifted up by 8 to the actual position of the values in 'offsets' for the data read in this loop
    readCStringFromEEPROM(currentAddress, s, offsets[i + 8]);
    memcpy(ingredientNames[i], s, sizeof(s));
    currentAddress += offsets[i + 8];
  }
}

//Called everytime a recipe or ingredient is received over serial
void writeToEEPROM() {
  if(!checkSumWritten) { EEPROM.update(ADDRESS_CHECK_SUM, EEPROM_CHECK_SUM); }

  int offsets[14]
  {
    sizeof(recipeNames[0]),
    sizeof(recipeNames[1]),
    sizeof(recipeNames[2]),
    sizeof(recipeNames[3]),
    sizeof(recipeNames[4]),
    sizeof(recipeNames[5]),
    sizeof(recipeNames[6]),
    sizeof(recipeNames[7]),
    sizeof(ingredientNames[0]),
    sizeof(ingredientNames[1]),
    sizeof(ingredientNames[2]),
    sizeof(ingredientNames[3]),
    sizeof(ingredientNames[4]),
    sizeof(ingredientNames[5])
  };

  for(int i = 0; i < 8; i++)
  {
    EEPROM.put(ADDRESSES_RECIPE[i], recipes[i]); //Read recipes from the EEPROM
  }

  EEPROM.put(ADDRESS_OFFSET_LOOKUP, offsets);

  int currentAddress = ADDRESS_OFFSET_LOOKUP + sizeof(offsets);
  for(int i = 0; i < 8; i++)
  {
    EEPROM.put(currentAddress, recipeNames[i]);
    currentAddress += offsets[i];
  }

  for(int i = 0; i < 6; i++)
  {
    EEPROM.put(currentAddress, ingredientNames[i]);
    currentAddress += offsets[i + 8];
  }
}

void readCStringFromEEPROM(int address, char* s, int length) {
  for(int i = 0; i < length; i++)
  {
    s[i] = char(EEPROM.read(address + i));
  }
}

