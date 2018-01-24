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
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.GridPane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Line;
import javafx.scene.text.TextAlignment;
import javafx.stage.Stage;
import javafx.util.Callback;
import javafx.util.Pair;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Optional;

public class DisplayManager
{
    private static GridPane recipesPane;
    private static GridPane ingredientPane;
    private static Button selectButton;
    private static Button connectButton;
    private static Label spaceLabel;
    private static Label stateLabel;
    private static ArrayList<Pair<TextField, ArrayList<CheckBox>>> recipeList = new ArrayList<>();
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

    public static ArrayList<Node> addScreenElements(Stage primaryStage, GridPane root, ArrayList<Node> elementsToDisable) //TODO: add missing screen elements
    {
        elementsToDisable = addMainElements(primaryStage, root, elementsToDisable);
        elementsToDisable = addRecipeElements(elementsToDisable);
        elementsToDisable = addIngredientElements(elementsToDisable);
        return elementsToDisable;
    }

    private static ArrayList<Node> addMainElements(Stage primaryStage, GridPane root, ArrayList<Node> elementsToDisable)
    {
        Line lineHorOne = new Line(0, 0, primaryStage.getWidth(), 0);
        lineHorOne.setStrokeWidth(2);
        root.add(lineHorOne, 0, 0);

        GridPane main = new GridPane();
        main.setMaxHeight(primaryStage.getHeight() - 86D);

        Line lineVertOne = new Line(primaryStage.getWidth() / 2D, 0, primaryStage.getWidth() / 2D, primaryStage.getHeight() - 86D);
        lineVertOne.setStrokeWidth(2);
        main.add(lineVertOne, 0, 0);

        GridPane ingredients = new GridPane();
        ingredients.setMinWidth(primaryStage.getWidth() / 2D - 6D);
        ingredients.setMinHeight(primaryStage.getHeight() - 86D);
        ingredients.setPadding(new Insets(10, 10, 10, 10));
        ingredients.setHgap(8);
        ingredients.setVgap(5);
        main.add(ingredients, 1, 0);
        ingredientPane = ingredients;

        Line lineVertTwo = new Line(primaryStage.getWidth() / 2D, 0, primaryStage.getWidth() / 2D, primaryStage.getHeight() - 86D);
        lineVertTwo.setStrokeWidth(2);
        main.add(lineVertTwo, 2, 0);

        GridPane recipes = new GridPane();
        recipes.setMinWidth(primaryStage.getWidth() / 2D - 6D);
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

        Label spaceOne = new Label("   ");
        status.add(spaceOne, 1, 0);

        Button connect = new Button("Connect");
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
        status.add(connect, 2, 0);
        connectButton = connect;

        Label space = new Label("        ");
        status.add(space, 3, 0);
        spaceLabel = space;

        Label desc = new Label("Status: ");
        status.add(desc, 4, 0);

        Label state = new Label("Disconnected");
        state.setTextFill(Color.web("#FF0000"));
        status.add(state, 5, 0);
        stateLabel = state;

        root.add(status, 0, 3);

        return elementsToDisable;
    }

    private static ArrayList<Node> addRecipeElements(ArrayList<Node> elementsToDisable)
    {
        for (int i = 0; i < 8; i++)
        {
            GridPane recipePane = new GridPane();
            recipePane.setVgap(5);

            Label desc = new Label("Recipe #" + (i + 1));
            recipePane.add(desc, 0, 0);

            GridPane namePane = new GridPane();
            Label name = new Label("Name:   ");
            name.setTextAlignment(TextAlignment.RIGHT);
            recipePane.add(name, 0, 1);
            LimitedTextField nameField = new LimitedTextField(8);
            namePane.add(nameField, 0, 0);
            recipePane.add(namePane, 1, 1);

            GridPane checkBoxPane = new GridPane();
            Label ingredientLabel = new Label("Ingredients:   ");
            recipePane.add(ingredientLabel, 0, 2);
            ArrayList<CheckBox> boxes = new ArrayList<>();
            for (int j = 0; j < 6; j++)
            {
                CheckBox box = new CheckBox();
                box.setText(Integer.toString(j + 1) + "    ");
                boxes.add(box);
                checkBoxPane.add(box, j + 1, 0);
            }
            recipePane.add(checkBoxPane, 1, 2);

            Label space = new Label("   ");
            recipePane.add(space, 0, 3);

            recipesPane.add(recipePane, 0, i);

            recipeList.add(new Pair<>(nameField, boxes));
        }

        recipesPane.add(new Label("  "), 0, 8);
        recipesPane.add(new Label("  "), 0, 9);

        GridPane buttons = new GridPane();
        buttons.setHgap(10);
        Button load = new Button("Load recipes");
        load.setDisable(true);
        load.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                loadRecipes();
            }
        });
        buttons.add(load, 0, 0);
        loadRecipesButton = load;
        Button send = new Button("Send recipes");
        send.setDisable(true);
        send.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                sendRecipes();
            }
        });
        buttons.add(send, 1, 0);
        sendRecipesButton = send;
        recipesPane.add(buttons, 0, 10);

        return elementsToDisable;
    }

    private static ArrayList<Node> addIngredientElements(ArrayList<Node> elementsToDisable)
    {
        //TODO: add missing elements

        GridPane buttons = new GridPane();
        buttons.setHgap(10);
        Button load = new Button("Load ingredients");
        load.setDisable(true);
        load.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                loadIngredients();
            }
        });
        buttons.add(load, 0, 0);
        loadIngredientsButton = load;
        Button send = new Button("Send ingredients");
        send.setDisable(true);
        send.setOnAction(new EventHandler<ActionEvent>()
        {
            @Override
            public void handle(ActionEvent event)
            {
                sendIngredients();
            }
        });
        buttons.add(send, 1, 0);
        sendIngredientsButton = send;
        ingredientPane.add(buttons, 0, 10);
        return elementsToDisable;
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
            connectButton.setText("Disconnect");
            spaceLabel.setText("     ");
            loadRecipesButton.setDisable(false);
            sendRecipesButton.setDisable(false);
            loadIngredientsButton.setDisable(false);
            sendIngredientsButton.setDisable(false);
        }
        else if (status == EnumConnStatus.DISCONNECTED)
        {
            connectButton.setText("Connect");
            spaceLabel.setText("        ");
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
        //TODO: process data
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }

    private static void sendRecipes()
    {
        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);
        //TODO: send data
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
        //TODO: process data
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }

    private static void sendIngredients()
    {
        loadRecipesButton.setDisable(true);
        sendRecipesButton.setDisable(true);
        loadIngredientsButton.setDisable(true);
        sendIngredientsButton.setDisable(true);
        //TODO: send data
        Main.INSTANCE.setWaitingForResponse();
    }

    public static void confirmSent()
    {
        loadRecipesButton.setDisable(false);
        sendRecipesButton.setDisable(false);
        loadIngredientsButton.setDisable(false);
        sendIngredientsButton.setDisable(false);
    }
}