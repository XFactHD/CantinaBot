package cantcon;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

import java.io.*;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Enumeration;

public class Main extends Application implements SerialPortEventListener
{
    public static Main INSTANCE;

    private Stage primaryStage;

    //Serial port data
    private String portName = null;
    private SerialPort port;

    //Serial port access and details
    private BufferedReader input;
    private OutputStream output;
    private static final int TIME_OUT = 2000;
    private static final int DATA_RATE = 9600;

    //Serial message markers
    private static final String MESSAGE_START_TRANSMISSION = "START";
    private static final String MESSAGE_END_TRANSMISSION = "TERMINATE";
    private static final String MESSAGE_ACKNOWLEDGE_TRANSMISSION = "ACK";
    private static final String MESSAGE_DECLINE_TRANSMISSION = "NACK";
    private static final String MESSAGE_ACCEPT_TRANSMISSION = "ACP";
    private static final String MESSAGE_RECIPE_DATA_HEADER = "RECIPES;";
    private static final String MESSAGE_INGREDIENT_DATA_HEADER = "INGREDIENTS;";
    private static final String MESSAGE_DATA_END_MARKER = ";END";

    //App state
    private boolean running = true;
    private boolean connected = false;
    private EnumConnStatus status = EnumConnStatus.DISCONNECTED;

    //Thread used for connection timeout
    private TimerThread timer = new TimerThread();

    //Serial status indicators
    private boolean waitingForData = false;
    private boolean waitingForResponse = false;

    //Main method of the app
    public static void main(String[] args)
    {
        extractDLLs();
        launch(args);
    }

    //Starts the app and initializes the screen elements and other variables
    @Override
    public void start(Stage primaryStage) throws Exception
    {
        INSTANCE = this;
        this.primaryStage = primaryStage;
        GridPane root = new GridPane();
        primaryStage.setTitle("CantinaControl");
        primaryStage.setScene(new Scene(root, 1800, 960));
        primaryStage.setResizable(false);
        //Set what happens when the app is closed
        primaryStage.setOnCloseRequest(new EventHandler<WindowEvent>()
        {
            @Override
            public void handle(WindowEvent event)
            {
                close();
            }
        });

        primaryStage.show();

        DisplayManager.addScreenElements(primaryStage, root);

        timer.start(); //Start the timeout timer thread
    }

    //Tries to initialize the serial port with the given name, returns if successful
    private boolean initializeSerialPort(String portName)
    {
        if (portName.equals("")) { return false; }

        CommPortIdentifier portId = null;
        Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();

        while (portEnum.hasMoreElements())
        {
            CommPortIdentifier currPortId = (CommPortIdentifier) portEnum.nextElement();
            if (currPortId.getName().equals(portName))
            {
                portId = currPortId;
                break;
            }
        }
        if (portId == null)
        {
            System.out.println("Could not find COM port.");
            return false;
        }

        try
        {
            port = (SerialPort) portId.open("Computer", TIME_OUT);
            port.setSerialPortParams(DATA_RATE, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

            input = new BufferedReader(new InputStreamReader(port.getInputStream()));
            output = port.getOutputStream();

            port.addEventListener(this);
            port.notifyOnDataAvailable(true);
        }
        catch (Exception e)
        {
            System.err.println(e.toString());
            return false;
        }

        DisplayManager.activateConnect();
        return true;
    }

    //Called when the app is closed, cleans up the serial port
    public synchronized void close()
    {
        if (port != null)
        {
            if (connected)
            {
                stopSerialComm();
            }
            port.removeEventListener();
            port.close();
        }
        running = false;
        primaryStage.close();
    }

    //Called by the select port button, opens the corresponding dialog and tries to initialize the port it returns
    public void selectPort()
    {
        boolean errored = false;
        String portName = "";
        while (portName.equals(""))
        {
            portName = DisplayManager.openPortDialog(errored);
            if (portName == null) { return; }
            if (portName.equals("")) { errored = true; continue; }
            if (initializeSerialPort(portName)) { break; }
            portName = "";
            errored = true;
        }

        this.portName = portName;
    }

    //Starts the serial communication by sending the START message
    public void startSerialComm()
    {
        if (port == null && portName != null)
        {
            initializeSerialPort(portName);
        }

        if (sendSerialMessage(MESSAGE_START_TRANSMISSION))
        {
            status = EnumConnStatus.CONNECTING;
            DisplayManager.setConnectionStatus(status);
            timer.activate();
            connected = true;
        }
    }

    //Ends the serial communication by sending the TERMINATE message
    public void stopSerialComm()
    {
        status = EnumConnStatus.DISCONNECTING;
        DisplayManager.setConnectionStatus(status);
        sendSerialMessage(MESSAGE_END_TRANSMISSION);
    }

    //Called by the rxtx library when serial data is received
    @Override
    public synchronized void serialEvent(SerialPortEvent event)
    {
        if (event.getEventType() == SerialPortEvent.DATA_AVAILABLE)
        {
            try
            {
                String inputLine = input.readLine();
                printInfo(inputLine);
                Platform.runLater(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        if (status == EnumConnStatus.CONNECTING)
                        {
                            if (inputLine.equals(MESSAGE_ACKNOWLEDGE_TRANSMISSION))
                            {
                                status = EnumConnStatus.CONNECTED;
                                DisplayManager.setConnectionStatus(status);

                                timer.deactivate();
                            }
                        }
                        else if (status == EnumConnStatus.DISCONNECTING)
                        {
                            if (inputLine.equals(MESSAGE_ACKNOWLEDGE_TRANSMISSION))
                            {
                                status = EnumConnStatus.DISCONNECTED;
                                DisplayManager.setConnectionStatus(status);
                                connected = false;
                            }
                        }
                        else if (status == EnumConnStatus.CONNECTED)
                        {
                            if (waitingForData)
                            {
                                if (inputLine.startsWith(MESSAGE_RECIPE_DATA_HEADER) && inputLine.endsWith(MESSAGE_DATA_END_MARKER))
                                {
                                    DisplayManager.receiveRecipes(inputLine);
                                    waitingForData = false;
                                }
                                else if (inputLine.startsWith(MESSAGE_INGREDIENT_DATA_HEADER) && inputLine.endsWith(MESSAGE_DATA_END_MARKER))
                                {
                                    DisplayManager.receiveIngredients(inputLine);
                                    waitingForData = false;
                                }
                            }
                            else if (waitingForResponse && inputLine.equals(MESSAGE_ACCEPT_TRANSMISSION))
                            {
                                DisplayManager.confirmSent();
                                waitingForResponse = false;
                            }
                            else if (inputLine.equals(MESSAGE_DECLINE_TRANSMISSION))
                            {
                                Alert alert = new Alert(Alert.AlertType.ERROR);
                                alert.setTitle("Error");
                                alert.setHeaderText("Communication failed!");
                                alert.setContentText("CantinaBot received an invalid message." +
                                        "\nPlease restart this program and your CantinaBot." +
                                        "\nIf this error occurs frequently, please consult an engineer from CantinaBot(R).");
                                alert.showAndWait();
                            }
                        }
                    }
                });
            }
            catch (Exception e)
            {
                if (e.getMessage().equals("Underlying input stream returned zero bytes"))
                {
                    System.err.println("Received empty stream!");
                }
                else
                {
                    System.err.println("An exception occured while reading data from the serial port!");
                    e.printStackTrace();
                }
            }
        }
    }

    //Sends a serial message to the arduino
    public boolean sendSerialMessage(String message)
    {
        message += "\n";
        try
        {
            output.write(message.getBytes());
            return true;
        }
        catch (IOException e)
        {
            printError("An error occurred while sending serial message!");
            printStacktrace(e);
            return false;
        }
    }

    public EnumConnStatus getStatus()
    {
        return status;
    }

    public void setWaitingForData()
    {
        waitingForData = true;
    }

    public void setWaitingForResponse()
    {
        this.waitingForResponse = true;
    }

    //Print a debug message to the internal console and to System.out
    public void printInfo(String line)
    {
        System.out.println(line);
        ConsoleManager.printToConsole("[INFO] " + line);
    }

    //Print an error message to the internal console and to System.error
    public void printError(String line)
    {
        System.err.println(line);
        ConsoleManager.printToConsole("[ERROR] " + line);
    }

    //Print a stacktrace to the internal console and to the system
    public void printStacktrace(Exception e)
    {
        e.printStackTrace();
        StringBuilder builder = new StringBuilder(e.getClass().getName() + System.lineSeparator());
        for (StackTraceElement element : e.getStackTrace())
        {
            builder.append("\t");
            builder.append("at ");
            builder.append(element.toString());
            builder.append(System.lineSeparator());
        }
        ConsoleManager.printToConsole(builder.toString());
    }

    //Extracts the DLLs from the jar if necessary
    private static void extractDLLs()
    {
        copyFile(Main.class.getResourceAsStream("/rxtxParallel.dll"), FileSystems.getDefault().getPath("rxtxParallel.dll").toString());
        copyFile(Main.class.getResourceAsStream("/rxtxSerial.dll"), FileSystems.getDefault().getPath("rxtxSerial.dll").toString());
    }

    //Handles the file copying of the DLLs
    public static void copyFile(InputStream source, String destination)
    {
        if (source == null) { return; } //Either we are in a dev environment or the desired files are not in the jar
        try
        {
            Files.copy(source, Paths.get(destination), StandardCopyOption.REPLACE_EXISTING);
        }
        catch (IOException ex)
        {
            //ignored, will crash moments later anyway
        }
    }

    //Thread used to handle connection timeout
    private static class TimerThread extends Thread
    {
        private long timestamp = -1;

        @Override
        public void run()
        {
            while(Main.INSTANCE.running)
            {
                if (timestamp != -1)
                {
                    if (Thread.interrupted())
                    {
                        timestamp = -1;
                    }
                    else if (System.currentTimeMillis() - timestamp > 1000)
                    {
                        Platform.runLater(new Runnable()
                        {
                            @Override
                            public void run()
                            {
                                Main.INSTANCE.status = EnumConnStatus.DISCONNECTED;
                                DisplayManager.setConnectionStatus(Main.INSTANCE.status);

                                Alert alert = new Alert(Alert.AlertType.ERROR);
                                alert.setTitle("Error");
                                alert.setHeaderText("Connection Error");
                                alert.setContentText("Couldn't connect to serial device (Timeout)!");
                                alert.showAndWait();
                            }
                        });
                        timestamp = -1;
                    }
                }
                delay();
            }
        }

        public void activate()
        {
            timestamp = System.currentTimeMillis();
        }

        public void deactivate()
        {
            timestamp = -1;
        }

        private void delay()
        {
            try
            {
                Thread.sleep(5);
            }
            catch (InterruptedException e)
            {
                System.err.println("Interrupted while sleeping!");
                e.printStackTrace();
            }
        }
    }
}
