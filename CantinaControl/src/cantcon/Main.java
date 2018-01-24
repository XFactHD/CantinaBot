package cantcon;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Enumeration;

public class Main extends Application implements SerialPortEventListener
{
    public static Main INSTANCE;

    private Stage primaryStage;

    private String portName = null;
    private SerialPort port;

    private BufferedReader input;
    private OutputStream output;
    private static final int TIME_OUT = 2000;
    private static final int DATA_RATE = 9600;

    private boolean running = true;
    private EnumConnStatus status = EnumConnStatus.DISCONNECTED;

    private ArrayList<Node> elementsToDisable = new ArrayList<>();

    private TimerThread timer = new TimerThread();

    private boolean waitingForData = false;
    private boolean waitingForResponse = false;

    public static void main(String[] args)
    {
        launch(args);
    }

    @Override
    public void start(Stage primaryStage) throws Exception
    {
        INSTANCE = this;
        this.primaryStage = primaryStage;
        GridPane root = new GridPane();
        primaryStage.setTitle("CantinaControl");
        primaryStage.setScene(new Scene(root, 1400, 1050));
        primaryStage.setResizable(false);
        primaryStage.setOnCloseRequest(new EventHandler<WindowEvent>()
        {
            @Override
            public void handle(WindowEvent event)
            {
                close();
            }
        });

        primaryStage.show();

        elementsToDisable = DisplayManager.addScreenElements(primaryStage, root, elementsToDisable);

        for (Node n : elementsToDisable)
        {
            n.setDisable(true);
        }
        timer.start();
    }

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

    public synchronized void close()
    {
        if (port != null)
        {
            sendSerialMessage("TERMINATE");
            port.removeEventListener();
            port.close();
        }
        running = false;
        primaryStage.close();
    }

    public void selectPort()
    {
        boolean errored = false;
        String portName = "";
        while (portName.equals(""))
        {
            portName = DisplayManager.openPortDialog(errored);
            if (portName == null) { close(); return; }
            if (portName.equals("")) { errored = true; continue; }
            if (initializeSerialPort(portName)) { break; }
            portName = "";
            errored = true;
        }

        this.portName = portName;
    }

    public void startSerialComm()
    {
        if (port == null && portName != null)
        {
            initializeSerialPort(portName);
        }

        if (sendSerialMessage("START"))
        {
            status = EnumConnStatus.CONNECTING;
            DisplayManager.setConnectionStatus(status);
            timer.activate();
        }
    }

    public void stopSerialComm()
    {
        status = EnumConnStatus.DISCONNECTING;
        DisplayManager.setConnectionStatus(status);
        sendSerialMessage("TERMINATE");
    }

    @Override
    public synchronized void serialEvent(SerialPortEvent event)
    {
        if (event.getEventType() == SerialPortEvent.DATA_AVAILABLE)
        {
            try
            {
                String inputLine = input.readLine();
                System.out.println(inputLine);
                Platform.runLater(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        if (status == EnumConnStatus.CONNECTING)
                        {
                            if (inputLine.equals("ACK"))
                            {
                                status = EnumConnStatus.CONNECTED;
                                DisplayManager.setConnectionStatus(status);

                                for(Node n : elementsToDisable)
                                {
                                    n.setDisable(false);
                                }
                                timer.deactivate();
                                sleep(1000);
                            }
                        }
                        else if (status == EnumConnStatus.DISCONNECTING)
                        {
                            if (inputLine.equals("ACK"))
                            {
                                status = EnumConnStatus.DISCONNECTED;
                                DisplayManager.setConnectionStatus(status);
                            }
                        }
                        else if (status == EnumConnStatus.CONNECTED)
                        {
                            if (waitingForData)
                            {
                                if (inputLine.startsWith("RECIPES;"))
                                {
                                    DisplayManager.receiveRecipes(inputLine);
                                }
                                else if (inputLine.startsWith("INGREDIENTS;"))
                                {
                                    DisplayManager.receiveIngredients(inputLine);
                                }
                                waitingForData = false;
                            }
                            else if (waitingForResponse && inputLine.equals("ACK"))
                            {
                                DisplayManager.confirmSent();
                                waitingForResponse = false;
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
            System.err.println("An error occurred while sending serial message!");
            e.printStackTrace();
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

    private static void sleep(int millis)
    {
        try
        {
            Thread.sleep(millis);
        }
        catch (InterruptedException e)
        {
            System.err.println("Interrupted while sleeping!");
            e.printStackTrace();
        }
    }

    private static class TimerThread extends Thread
    {
        private long timestamp = 0;

        @Override
        public void run()
        {
            while(Main.INSTANCE.running)
            {
                if (timestamp != 0)
                {
                    if (Thread.interrupted())
                    {
                        timestamp = 0;
                    }
                    else if (System.currentTimeMillis() - timestamp > 1000) //FIXME: kills connection even if succesfull
                    {
                        //Platform.runLater(new Runnable()
                        //{
                        //    @Override
                        //    public void run()
                        //    {
                        //        Main.INSTANCE.status = EnumConnStatus.DISCONNECTED;
                        //        DisplayManager.setConnectionStatus(Main.INSTANCE.status);

                        //        Alert alert = new Alert(Alert.AlertType.ERROR);
                        //        alert.setTitle("Error");
                        //        alert.setHeaderText("Connection Error");
                        //        alert.setContentText("Couldn't connect to serial device (Timeout)!");
                        //        alert.showAndWait();
                        //    }
                        //});
                        timestamp = 0;
                    }
                }
                Main.sleep(5);
            }
        }

        public void activate()
        {
            timestamp = System.currentTimeMillis();
        }

        public void deactivate()
        {
            timestamp = 0;
        }
    }
}
