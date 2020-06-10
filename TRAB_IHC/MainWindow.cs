using System;
using System.Runtime.InteropServices;
using Gtk;

public partial class MainWindow : Gtk.Window
{
    private int connection_status;

    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int connect_wiimotes();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int enable_motion_sensing();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int disconnect_wiimotes();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void calibrate_force(int axis, int turn);
 
    public MainWindow() : base(Gtk.WindowType.Toplevel) => Build();

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Application.Quit();
        a.RetVal = true;
    }
    
    protected void ConnectHandler(object sender, EventArgs e)
    {
        /* Connect to the wiimote */

        // Show message to user with instructions on connecting the wiimote
        MessageDialog connect_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Question, ButtonsType.OkCancel,
            "Please put your wiimote in discovery mode (Press 1 + 2) and press OK, then wait for LED 1 to light up");
        int cd_result = connect_md.Run();
        connect_md.Destroy();

        if (cd_result == -5) // Dialog OK
        {
            // Attempt to connect to wiimote
            connection_status = connect_wiimotes();

            // If connection sucessful
            if (connection_status > 0)
            {
                // Show message to user with instructions on calibrating the gyroscope
                MessageDialog gyro_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok, 
                    "Gyroscope calibration will now begin.\nPlease place your wiimote face up on a flat surface and press OK");
                gyro_md.Run();
                gyro_md.Destroy();

                // Enable motion sensing and calibrate the gyroscope
                int status = enable_motion_sensing();

                // Enable the calibration and disconnect buttons, as well as the feedback frame
                // Disable the connect button
                calibrateButton.Sensitive = true;
                disconnectButton.Sensitive = true;
                feedbackFrame.Sensitive = true;
                connectButton.Sensitive = false;
            }
            else
            {
                // Show message to user informing that no wiimotes were detected / connected
                MessageDialog error_md = new MessageDialog(this,DialogFlags.DestroyWithParent, MessageType.Error, ButtonsType.Ok, 
                    "No wiimotes were detected, please make sure that your bluetooth adapter is turned ON.");
                error_md.Run();
                error_md.Destroy();
            }
        }
    }

    protected void CalibrateHandler(object sender, EventArgs e)
    {
        /* Calibrate the thresholds to user's movements */

        // Image with instructions on how to execute the downward movement
        Gtk.Image down_mvmnt_img = new Gtk.Image("./Resources/wiimoteDownMovement.png"); // 200 x 150
        Gtk.Image side_mvmnt_img = new Gtk.Image("./Resources/wiimoteLeftMovement.png"); // 200 x 150


        // Calibrate downward movement

        // Show message with instructions for first move
        MessageDialog downfirst_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.OkCancel,
            "Calibration for down movements will now begin, please press OK and make the movement shown in the image")
        {
            Image = down_mvmnt_img
        };
        downfirst_md.ShowAll();
        downfirst_md.Run();
        downfirst_md.Destroy();

        // Record user movements after OK is pressed
        calibrate_force(0, 0);

        // Show message with instructions for second move
        MessageDialog downsecond_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "Very good, now a second time")
        {
            Image = down_mvmnt_img
        };
        downsecond_md.ShowAll();
        downsecond_md.Run();
        downsecond_md.Destroy();

        // Record user movements after OK is pressed 
        calibrate_force(0, 1);

        // Show message with instructions for third move
        MessageDialog downthird_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "And lastly a third time")
        {
            Image = down_mvmnt_img
        };
        downthird_md.ShowAll();
        downthird_md.Run();
        downthird_md.Destroy();

        // Record user movements after OK is pressed
        calibrate_force(0, 2);


        // Calibrate sideways movement

        // Show message with instructions for the first move
        MessageDialog sidefirst_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "Calibration for sideways movements will now begin, please press OK and and make the movement shown in the image")
        {
            Image = side_mvmnt_img
        };
        sidefirst_md.ShowAll();
        sidefirst_md.Run();
        sidefirst_md.Destroy();

        // Record user movements after OK is pressed
        calibrate_force(1, 0);

        MessageDialog sidesecond_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
         "Very good, now a second time")
        {
            Image = side_mvmnt_img
        };
        sidesecond_md.ShowAll();
        sidesecond_md.Run();
        sidesecond_md.Destroy();

        // Record user movements affter OK is pressed
        calibrate_force(1, 1);

        MessageDialog sidethird_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
         "And lastly a third time")
        {
            Image = side_mvmnt_img
        };
        sidethird_md.ShowAll();
        sidethird_md.Run();
        sidethird_md.Destroy();

        // Record user movements after OK is pressed
        calibrate_force(1, 2);

    }

    protected void DisconnectHandler(object sender, EventArgs e)
    {
        /* Safely disconnect the wiimote */

        // Show message to user with instructions on disconnecting wiimote
        MessageDialog disconnect_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Question, ButtonsType.OkCancel,
             "Please hold the power button on your wiimote until LED 1 turns off and press OK.");
        int dm_result = disconnect_md.Run();
        disconnect_md.Destroy();

        if (dm_result == -5) // Dialog OK
        {
            connection_status = disconnect_wiimotes();

            if (connection_status == 0)
            {
                // Show message to user informing disconnect was sucessful
                MessageDialog sucess_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                    "Wiimote sucessfuly disconnected! If you want to reconnect, simply press the connect button again");
                sucess_md.Run();
                sucess_md.Destroy();

                // Disable the calibration and disconnect button, as well as feedback frame
                // Enable the connect button
                calibrateButton.Sensitive = false;
                disconnectButton.Sensitive = false;
                feedbackFrame.Sensitive = false;
                connectButton.Sensitive = true;
            }
            else
            {
                // Show message informing user the wiimote was unable to disconnect
                MessageDialog error_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Error, ButtonsType.Ok,
                    "No wiimotes were disconnected, please make sure that you held the power button until LED 1 was OFF.");
                error_md.Run();
                error_md.Destroy();
            }
        }
    }

    protected void ToggleVibrationHandler(object sender, EventArgs e)
    {
        /* Toggle vibration on hit */
        // TODO
    }

    protected void ToggleSoundHandler(object sender, EventArgs e)
    { 
        /* Enable/Disable sound options frame*/
        if (!soundFrame.Sensitive) soundFrame.Sensitive = true;
        else soundFrame.Sensitive = false;

        /* Toggle sound playing on hit*/
        // TODO
    }

    protected void CenterListenHandler(object sender, EventArgs e)
    {
        /* Let user listen to sound for drum center hit*/
        // TODO
    }

    protected void CenterChooseHandler(object sender, EventArgs e)
    {
        /* Let user choose sound for drum center hit*/
        // TODO
    }

    protected void SiideListenHandler(object sender, EventArgs e)
    {
        /* Let user listen to sound for drum side hit*/
        // TODO
    }

    protected void SideChooseHandler(object sender, EventArgs e)
    {
        /* Let user choose sound for drum side hit*/
        // TODO
    }

    protected void OnControlsActionActivated(object sender, EventArgs e)
    {

    }

}
