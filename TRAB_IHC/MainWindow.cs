using System;
using System.Runtime.InteropServices;
using Gtk;

public partial class MainWindow : Gtk.Window
{
    private int connection_status;

    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int connect_wiimotes();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int enable_motion_sensing();

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
                calibrateButton.Sensitive = true;
                disconnectButton.Sensitive = true;
                feedbackFrame.Sensitive = true;
            }
            else
            {
                // Show message to user informing that no wiimotes were detected / connected
                MessageDialog error_md = new MessageDialog(this,DialogFlags.DestroyWithParent, MessageType.Error, ButtonsType.Ok, 
                    "No wiimotes were detected, please make sure that your bluetooth adapter is turned ON.");
            }
        }
    }

    protected void CalibrateHandler(object sender, EventArgs e)
    {
        /* Calibrate the thresholds to user's movements */
        // TODO
    }

    protected void DisconnectHandler(object sender, EventArgs e)
    {
        /* Safely disconnect the wiimote */
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
