void setName(int index, const char* name) {
  memcpy(recipeNames[index], name, (strlen(name) + 1) * sizeof(char));
}

char* getName(int index) {
  return recipeNames[index];
}

int getNumberOfIngredients(int index) {
  return recipeIngredientCounts[index];
}

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

int getIngredient(int index, int pos) {
  return recipeIngredients[index][pos];
}

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

int getAmountOfIngredient(int index, int ingredient) {
  for(int i = 0; i < getNumberOfIngredients(index); i++)
  {
    if(recipeIngredients[index][i] == ingredient)
    {
      return recipeIngredientAmounts[index][i];
    }
  }
}

