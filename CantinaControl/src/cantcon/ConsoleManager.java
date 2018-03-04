package cantcon;

import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.TextArea;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

public class ConsoleManager
{
    //The text are containing the displayed lines
    private static TextArea console = null;

    //Opens and configures the console in a seperate window
    public static void openConsole()
    {
        GridPane grid = new GridPane();
        grid.setAlignment(Pos.CENTER);
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(10));

        TextArea output = new TextArea();
        console = output;
        output.setPrefWidth(700);
        output.setPrefHeight(400);
        output.setEditable(false);
        grid.add(output, 0, 0);

        Scene dialog = new Scene(grid);

        Stage stage = new Stage();
        stage.setScene(dialog);
        stage.setTitle("Console");

        stage.setOnCloseRequest(new EventHandler<WindowEvent>()
        {
            @Override
            public void handle(WindowEvent event)
            {
                closeConsole();
            }
        });

        stage.show();
        stage.toFront();
    }

    //Handles closing of the console
    public static void closeConsole()
    {
        console = null;
        DisplayManager.onConsoleClosed();
    }

    //Adds a new line to the console
    public static void printToConsole(String line)
    {
        if (console != null)
        {
            console.appendText(line);
            console.appendText(System.lineSeparator());
        }
    }
}
