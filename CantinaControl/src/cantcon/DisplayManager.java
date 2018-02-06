/*  Copyright (C) <2017>  <XFactHD>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses. */

package cantcon;

import com.sun.javafx.collections.ObservableListWrapper;
import gnu.io.CommPortIdentifier;
import javafx.application.Platform;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.scene.control.*;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Line;
import javafx.stage.Stage;
import javafx.util.Callback;
import javafx.util.Pair;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Optional;

public class DisplayManager
{
    private static GridPane recipesPane;
    private static GridPane ingredientsPane;
    private static Button selectButton;
    private static Button connectButton;
    private static Label stateLabel;
    private static Button consoleButton;
    private static ArrayList<Pair<TextField, ArrayList<Spinner<Integer>>>> recipeList = new ArrayList<>();
    private static Button loadRecipesButton;
    private static Button sendRecipesButton;
    private static ArrayList<TextField> ingredientList = new ArrayList<>();
    private static Button loadIngredientsButton;
    private static Button sendIngredientsButton;

    public static String openPortDialog(boolean erroredBefore)
    {
        Dialog<String> dialog = new Dialog<>();
        dialog.setTitle("Choose port");
        dialog.getDialogPane().getButtonTypes().addAll(ButtonType.OK);

        dialog.getDialogPane().getScene().addEventHandler(KeyEvent.KEY_PRESSED, new EventHandler<KeyEvent>()
        {
            @Override
            public void handle(KeyEvent event)
            {
                if (event.getCode() == KeyCode.ESCAPE)
                {
                    Main.INSTANCE.close();
                }
            }
        });

        GridPane grid = new GridPane();
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(20, 150, 10, 10));

        ArrayList<String> items = new ArrayList<>();
        Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();
        while (portEnum.hasMoreElements())
        {
            CommPortIdentifier currPortId = (CommPortIdentifier) portEnum.nextElement();
            items.add(currPortId.getName());
        }
        items.remove("COM1");

        ObservableList<String> list = new ObservableListWrapper<>(items);
        ChoiceBox<String> port = new ChoiceBox<>(list);
        if (!items.isEmpty())
        {
            port.setValue(items.get(0));
        }

        Label message = new Label();
        if (erroredBefore)
        {
            message.setTextFill(Color.web("#FF0000"));
            message.setText("This com port could not be accessed!");
        }
        else if(items.isEmpty())
        {
            message.setTextFill(Color.web("#FF0000"));
            message.setText("No COM ports available, check your connections!");
        }
        else
        {
            message.setText("Select COM port");
        }

        grid.add(message, 0, 0);
        grid.add(port, 0, 1);

        dialog.getDialogPane().setContent(grid);

        dialog.setResultConverter(new Callback<ButtonType, String>()
        {
            @Override
            public String call(ButtonType param)
            {
                if (param == ButtonType.OK)
                {
                    return port.getValue();
                }
                return null;
            }
        });

        Optional<String> result = dialog.showAndWait();
        return result.orElse(null);
    }

    public static void addScreenElements(Stage primaryStage, GridPane root)
    {
        addMainElements(primaryStage, root);
        addIngredientElements();
        addRecipeElements();
    }

    private static void addMainElements(Stage primaryStage, GridPane root)
    {
        Line lineHorOne = new Line(0, 0, primaryStage.getWidth(), 0);
        lineHorOne.setStrokeWidth(2);
        root.add(lineHorOne, 0, 0);

        GridPane main = new GridPane();
        main.setMaxHeight(primaryStage.getHeight() - 86D);

        Line lineVertOne = new Line(primaryStage.getWidth() / 3D, 0, primaryStage.getWidth() / 3D, primaryStage.getHeight() - 86D);
        lineVertOne.setStrokeWidth(2);
        main.add(lineVertOne, 0, 0);

        GridPane ingredients = new GridPane();
        ingredients.setMinWidth(primaryStage.getWidth() / 3D - 6D);
        ingredients.setMinHeight(primaryStage.getHeight() - 86D);
        ingredients.setPadding(new Insets(10, 10, 10, 10));
        ingredients.setHgap(8);
        ingredients.setVgap(5);
        main.add(ingredients, 1, 0);
        ingredientsPane = ingredients;

        Line lineVertTwo = new Line(primaryStage.getWidth() / 3D, 0, primaryStage.getWidth() / 3D, primaryStage.getHeight() - 86D);
        lineVertTwo.setStrokeWidth(2);
        main.add(lineVertTwo, 2, 0);

        GridPane recipes = new GridPane();
        recipes.setMinWidth((primaryStage.getWidth() / 3D) * 2D - 6D);
        recipes.setMinHeight(primaryStage.getHeight() - 86D);
        recipes.setPadding(new Insets(10, 10, 10, 10));
        recipes.setHgap(8);
        recipes.setVgap(5);
        main.add(recipes, 3, 0);
        recipesPane = recipes;

        Line lineVertThree = new Line(primaryStage.getWidth() / 2D, 0, primaryStage.getWidth() / 2D, primaryStage.getHeight() - 86D);
        lineVertThree.setStrokeWidth(2);
        main.add(lineVertThree, 4, 0);

        root.add(main, 0, 1);

        Line lineHorTwo = new Line(0, primaryStage.getHeight() - 84D, primaryStage.getWidth(), primaryStage.getHeight() - 84D);
        lineHorTwo.setStrokeWidth(2);
        root.add(lineHorTwo, 0, 2);

        GridPane status = new GridPane();
        status.setHgap(5);
        status.setMinWidth(primaryStage.getWidth());
        status.setPadding(new Insets(7));
        status.setMinHeight(80);

        Button select = new Button("Select port");
        select.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                Platform.runLater(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        Main.INSTANCE.selectPort();
                    }
                });
            }
        });
        status.add(select, 0, 0);
        selectButton = select;

        Button connect = new Button("    Connect    ");
        connect.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                Platform.runLater(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        if (Main.INSTANCE.getStatus() == EnumConnStatus.CONNECTED)
                        {
                            Main.INSTANCE.stopSerialComm();
                        }
                        else if (Main.INSTANCE.getStatus() == EnumConnStatus.DISCONNECTED)
                        {
                            selectButton.setDisable(true);
                            Main.INSTANCE.startSerialComm();
                        }
                    }
                });
            }
        });
        connect.setDisable(true);
        status.add(connect, 1, 0);
        connectButton = connect;

        Label desc = new Label(" Status:");
        status.add(desc, 2, 0);

        Label state = new Label(EnumConnStatus.DISCONNECTED.getName());
        state.setTextFill(Color.web("#FF0000"));
        status.add(state, 3, 0);
        stateLabel = state;

        Button console = new Button("Open Console");
        console.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                ConsoleManager.openConsole();
                consoleButton.setDisable(true);
            }
        });
        status.add(console, 4, 0);
        consoleButton = console;

        root.add(status, 0, 3);
        state.setPrefWidth(111);
        connect.setPrefWidth(118);
    }

    private static void addIngredientElements()
    {
        int yIndex = 0;
        for (int i = 0; i < 6; i++)
        {
            GridPane ingredientPane = new GridPane();
            ingredientPane.setVgap(5);
            ingredientPane.setHgap(10);

            Label description = new Label("Ingredient #" + Integer.toString(i + 1));
            ingredientPane.add(description, 0, 0);

            Label name = new Label("Name: ");
            ingredientPane.add(name, 0, 1);

            LimitedTextField nameField = new LimitedTextField(5);
            ingredientPane.add(nameField, 1, 1);
            ingredientList.add(nameField);

            ingredientsPane.add(ingredientPane, 0, yIndex);
            yIndex++;

            Separator separatorOne = new Separator(Orientation.HORIZONTAL);
            ingredientsPane.add(separatorOne, 0, yIndex);
            yIndex++;
        }

        GridPane buttonPane = new GridPane();
        buttonPane.setHgap(10);

        Button loadIngredients = new Button("Load ingredients");
        loadIngredients.setDisable(true);
        loadIngredients.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                loadIngredients();
            }
        });
        buttonPane.add(loadIngredients, 0, 0);
        loadIngredientsButton = loadIngredients;
        Button sendIngredients = new Button("Send ingredients");
        sendIngredients.setDisable(true);
        sendIngredients.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                sendIngredients();
            }
        });
        buttonPane.add(sendIngredients, 1, 0);
        sendIngredientsButton = sendIngredients;

        ingredientsPane.add(buttonPane, 0, yIndex);
    }

    private static void addRecipeElements()
    {
        for (int i = 0; i < 8; i++)
        {
            GridPane recipePane = new GridPane();
            recipePane.setVgap(5);

            Label description = new Label("Recipe #" + Integer.toString(i + 1));
            recipePane.add(description, 0, 0);

            GridPane namePane = new GridPane();
            Label name = new Label("Name:   ");
            recipePane.add(name, 0, 1);
            LimitedTextField nameField = new LimitedTextField(8);
            namePane.add(nameField, 0, 1);
            recipePane.add(namePane, 1, 1);

            Label ingredientLabel = new Label("Ingredients:   ");
            recipePane.add(ingredientLabel, 0, 2);
            GridPane spinnerPane = new GridPane();
            spinnerPane.setHgap(7);
            ArrayList<Spinner<Integer>> spinners = new ArrayList<>();
            int xCoord = 0;
            for (int j = 0; j < 6; j++)
            {
                Label ingredientIndex = new Label(Integer.toString(j + 1) + ": ");
                spinnerPane.add(ingredientIndex, xCoord, 0);
                xCoord++;
                Spinner<Integer> amount = new Spinner<>(0, 200, 0);
                amount.setMaxWidth(100);
                spinners.add(amount);
                spinnerPane.add(amount, xCoord, 0);
                xCoord++;
                if (j < 5)
                {
                    Separator separator = new Separator(Orientation.VERTICAL);
                    spinnerPane.add(separator, xCoord, 0);
                    xCoord++;
                }
            }
            recipePane.add(spinnerPane, 1, 2);

            Separator separatorOne = new Separator(Orientation.HORIZONTAL);
            recipePane.add(separatorOne, 0, 3);
            Separator separatorTwo = new Separator(Orientation.HORIZONTAL);
            recipePane.add(separatorTwo, 1, 3);

            recipesPane.add(recipePane, 0, i);

            recipeList.add(new Pair<>(nameField, spinners));
        }

        GridPane buttonPane = new GridPane();
        buttonPane.setHgap(10);

        Button loadRecipes = new Button("Load recipes");
        loadRecipes.setDisable(true);
        loadRecipes.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                loadRecipes();
            }
        });
        buttonPane.add(loadRecipes, 0, 0);
        loadRecipesButton = loadRecipes;
        Button sendRecipes = new Button("Send recipes");
        sendRecipes.setDisable(true);
        sendRecipes.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                sendRecipes();
            }
        });
        buttonPane.add(sendRecipes, 1, 0);
        sendRecipesButton = sendRecipes;

        recipesPane.add(buttonPane, 0, 8);
    }

    public static void setConnectionStatus(EnumConnStatus status)
    {
        if (status == EnumConnStatus.CONNECTING || status == EnumConnStatus.DISCONNECTING)
        {
            connectButton.setDisable(true);
        }
        else
        {
            connectButton.setDisable(false);
        }

        if (status == EnumConnStatus.DISCONNECTING)
        {
            loadRecipesButton.setDisable(true);
            sendRecipesButton.setDisable(true);
            loadIngredientsButton.setDisable(true);
            sendIngredientsButton.setDisable(true);
        }

        if (status == EnumConnStatus.CONNECTED)
        {
            connectButton.setText("  Disconnect   ");
            loadRecipesButton.setDisable(false);
            sendRecipesButton.setDisable(false);
            loadIngredientsButton.setDisable(false);
            sendIngredientsButton.setDisable(false);
        }
        else if (status == EnumConnStatus.DISCONNECTED)
        {
            connectButton.setText("    Connect    ");
            selectButton.setDisable(false);
        }
        stateLabel.setText(status.getName());
        stateLabel.setTextFill(Color.web(status.getColor()));
    }

    public static void activateConnect()
    {
        connectButton.setDisable(false);
    }

    private static void loadRecipes()
    {
        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);
        Main.INSTANCE.sendSerialMessage("GETRECIPES");
        Main.INSTANCE.setWaitingForData();
    }

    public static void receiveRecipes(String input)
    {
        input = input.replace("RECIPES;", "");
        input = input.replace(";END", "");
        String[] recipes = input.split(";");
        for (int i = 0; i < recipes.length && i < 8; i++)
        {
            Pair<TextField, ArrayList<Spinner<Integer>>> recipeFields = recipeList.get(i);
            String[] data = recipes[i].split("\\|");
            recipeFields.getKey().setText(data[0]);
            ArrayList<Spinner<Integer>> ingredientFields = recipeFields.getValue();
            for (int j = 2; j < data.length && j < 8; j++)
            {
                String[] details = data[j].split(",");
                int index = Integer.valueOf(details[0]);
                ingredientFields.get(index).getValueFactory().setValue(Integer.valueOf(details[1]));
            }
        }
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }

    private static void sendRecipes()
    {
        int[] ingredientCounts = new int[] {0, 0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < 8; i++)
        {
            Pair<TextField, ArrayList<Spinner<Integer>>> recipe = recipeList.get(i);
            if (recipe.getKey().getText().equals(""))
            {
                Alert alert = new Alert(Alert.AlertType.ERROR);
                alert.setTitle("Error");
                alert.setHeaderText("Can't send recipes!");
                alert.setContentText("Can't send recipe with empty name to CantinaBot!");
                alert.showAndWait();
                return;
            }

            ArrayList<Spinner<Integer>> ingredients = recipe.getValue();
            for (int j = 0; j < 6; j++)
            {
                if (ingredients.get(j).getValue() != 0)
                {
                    ingredientCounts[i]++;// = ingredientCounts[i] + 1;
                }
            }

            if (ingredientCounts[i] == 0)
            {
                Alert alert = new Alert(Alert.AlertType.ERROR);
                alert.setTitle("Error");
                alert.setHeaderText("Can't send recipes!");
                alert.setContentText("Can't send recipe with no ingredients to CantinaBot!");
                alert.showAndWait();
                return;
            }
        }

        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);

        StringBuilder builder = new StringBuilder("RECIPES;");
        for (int i = 0; i < 8; i++)
        {
            Pair<TextField, ArrayList<Spinner<Integer>>> recipe = recipeList.get(i);
            builder.append(recipe.getKey().getText());
            builder.append("|");
            builder.append(ingredientCounts[i]);
            ArrayList<Spinner<Integer>> ingredients = recipe.getValue();
            for (int j = 0; j < 6; j++)
            {
                if (ingredients.get(j).getValue() != 0)
                {
                    builder.append("|");
                    builder.append(j);
                    builder.append(",");
                    builder.append(ingredients.get(j).getValue());
                }
            }
            builder.append(";");
        }
        builder.append("END");
        Main.INSTANCE.sendSerialMessage(builder.toString());

        Main.INSTANCE.setWaitingForResponse();
    }

    private static void loadIngredients()
    {
        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);
        Main.INSTANCE.sendSerialMessage("GETINGREDIENTS");
        Main.INSTANCE.setWaitingForData();
    }

    public static void receiveIngredients(String input)
    {
        input = input.replace("INGREDIENTS;", "");
        input = input.replace(";END", "");
        String[] names = input.split(";");
        for (int i = 0; i < names.length && i < 6; i++)
        {
            ingredientList.get(i).setText(names[i]);
        }
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }

    private static void sendIngredients()
    {
        for (TextField field : ingredientList)
        {
            if (field.getText().equals(""))
            {
                Alert alert = new Alert(Alert.AlertType.ERROR);
                alert.setTitle("Error");
                alert.setHeaderText("Can't send ingredients!");
                alert.setContentText("Can't send empty ingredient name to CantinaBot!");
                alert.showAndWait();
                return;
            }
        }

        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);

        StringBuilder builder = new StringBuilder("INGREDIENTS;");
        for (int i = 0; i < 6; i++)
        {
            builder.append(ingredientList.get(i).getText());
            builder.append(";");
        }
        builder.append("END");
        Main.INSTANCE.sendSerialMessage(builder.toString());

        Main.INSTANCE.setWaitingForResponse();
    }

    public static void confirmSent()
    {
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }

    public static void onConsoleClosed()
    {
        consoleButton.setDisable(false);
    }
}