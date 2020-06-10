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
        MessageDialog first_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "Move 1 - Down");
        first_md.Run();
        first_md.Destroy();

        MessageDialog m1first_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 1 - Time 1");
        m1first_md.Run();
        m1first_md.Destroy();

        calibrate_force(0, 0);

        MessageDialog m1second_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 1 - Time 2");
        m1second_md.Run();
        m1second_md.Destroy();

        calibrate_force(0, 1);

        MessageDialog m1third_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 1 - Time 3");
        m1third_md.Run();
        m1third_md.Destroy();

        calibrate_force(0, 2);

        MessageDialog second_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "Move 2 - Left");
        second_md.Run();
        second_md.Destroy();

        MessageDialog m2first_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 2 - Time 1");
        m2first_md.Run();
        m2first_md.Destroy();

        calibrate_force(1, 0);

        MessageDialog m2second_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 2 - Time 2");
        m2second_md.Run();
        m2second_md.Destroy();

        calibrate_force(1, 1);

        MessageDialog m2third_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
    "Move 2 - Time 3");
        m2third_md.Run();
        m2third_md.Destroy();

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
}
