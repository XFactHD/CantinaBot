const byte EEPROM_CHECK_SUM = 100; //"Checksum" used to make sure we do not read any garbage from an EEPROM we never wrote to

const int ADDRESS_CHECK_SUM        = 0; //EEPROM address of the "checksum", needed to make sure we do not read any garbage from an EEPROM we never wrote to
const int ADDRESS_RECIPE_NAMES     = ADDRESS_CHECK_SUM    + sizeof(byte); //EEPROM address of the recipe names
const int ADDRESS_COUNTS           = ADDRESS_RECIPE_NAMES + sizeof(recipeNames); //EEPROM address of the number of ingredients in each recipe
const int ADDRESS_INGREDIENTS      = ADDRESS_COUNTS       + sizeof(recipeIngredientCounts); //EEPROM address of the ingredient lists of each recipe
const int ADDRESS_AMOUNTS          = ADDRESS_INGREDIENTS  + sizeof(recipeIngredients); //EEPROM address of the amounts of the ingredients in each recipe
const int ADDRESS_INGREDIENT_NAMES = ADDRESS_AMOUNTS      + sizeof(recipeIngredientAmounts); //EEPROM address of the ingredient names

boolean checkSumWritten = false;

//Called when the MCU boots up, reads the recipes and ingredients from the EEPROM
void readFromEEPROM() {
  byte checkSum = 0;
  EEPROM.get(ADDRESS_CHECK_SUM, checkSum); //Read our checksum from the EEPROM to check if the EEPROM contains data
  checkSumWritten = checkSum == EEPROM_CHECK_SUM;
  if(!checkSumWritten) { return; } //We never wrote anything to the EEPROM or a format change was made, so there is no need to read anything

  EEPROM.get(ADDRESS_RECIPE_NAMES, recipeNames);
  EEPROM.get(ADDRESS_COUNTS, recipeIngredientCounts);
  EEPROM.get(ADDRESS_INGREDIENTS, recipeIngredients);
  EEPROM.get(ADDRESS_AMOUNTS, recipeIngredientAmounts);
  EEPROM.get(ADDRESS_INGREDIENT_NAMES, ingredientNames);
}

//Called when the serial connection is closed by the control software, writes the recipes and the ingredients to the EEPROM
void writeToEEPROM() {
  if(!checkSumWritten) { EEPROM.update(ADDRESS_CHECK_SUM, EEPROM_CHECK_SUM); }

  EEPROM.put(ADDRESS_RECIPE_NAMES, recipeNames);
  EEPROM.put(ADDRESS_COUNTS, recipeIngredientCounts);
  EEPROM.put(ADDRESS_INGREDIENTS, recipeIngredients);
  EEPROM.put(ADDRESS_AMOUNTS, recipeIngredientAmounts);
  EEPROM.put(ADDRESS_INGREDIENT_NAMES, ingredientNames);
}

