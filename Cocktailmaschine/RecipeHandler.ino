//Sets the name of the recipe at the index passed in
void setName(int index, const char* name) {
  memcpy(recipeNames[index], name, (strlen(name) + 1) * sizeof(char));
}

//Returns the name of the recipe at the index passed in
char* getName(int index) {
  return recipeNames[index];
}

//Returns the number of ingredients the recipe at the index passed in requires
int getNumberOfIngredients(int index) {
  return recipeIngredientCounts[index];
}

//Sets the ingredient in the ingredient slot at pos in the recipe at the index passed in
void setIngredient(int index, int pos, int ingredient) {
  if(recipeIngredients[index][pos] == -1 && ingredient != -1)
  {
    recipeIngredientCounts[index] = recipeIngredientCounts[index] + 1;
  }
  else if(recipeIngredients[index][pos] != -1 && ingredient == -1)
  {
    recipeIngredientCounts[index] = recipeIngredientCounts[index] - 1;
  }
  recipeIngredients[index][pos] = ingredient;
}

//Returns the ingredient in the ingredient slot at pos of the recipe at the index passed in
int getIngredient(int index, int pos) {
  return recipeIngredients[index][pos];
}

//Sets the amount of the ingredient in recipe at the index passed in
void setAmountOfIngredient(int index, int ingredient, int amount) {
  for(int i = 0; i < getNumberOfIngredients(index); i++)
  {
    if(recipeIngredients[index][i] == ingredient)
    {
      recipeIngredientAmounts[index][i] = amount;
       break;
    }
  }
}

//Returns the amount of the ingredient in the recipe at the index passed in
int getAmountOfIngredient(int index, int ingredient) {
  for(int i = 0; i < getNumberOfIngredients(index); i++)
  {
    if(recipeIngredients[index][i] == ingredient)
    {
      return recipeIngredientAmounts[index][i];
    }
  }
  return -1;
}

