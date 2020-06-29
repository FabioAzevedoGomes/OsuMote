using System;
using System.Runtime.InteropServices;
using System.Threading;
using Gtk;

public partial class MainWindow : Gtk.Window
{
    private int connection_status = -5;
    private static bool exit;
    static int drum_img_state;

    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int ConnectWiimotes();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int EnableMotionSensing();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int DisconnectWiimotes();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void CalibrateForce(int axis, int turn);
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void ToggleVibration();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern int GetLastReadMovement();
    [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern bool IsPressedB();

    public MainWindow() : base(Gtk.WindowType.Toplevel) => Build();

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        exit = true;
        Application.Quit();
        a.RetVal = true;
    }

    protected void ConnectHandler(object sender, EventArgs e)
    {
        // Show message to user with instructions on connecting the wiimote
        MessageDialog connect_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Question, ButtonsType.OkCancel,
            "Please put your wiimote in discovery mode.\n\nPress 1 + 2 and press OK, then wait for LED 1 to light up.\n\nThis action will take up to 10 seconds.\n\nPlease make sure that your bluetooth adapter is turned ON.");
        connect_md.Title = "Connect";
        connect_md.Image = new Image("./res/ConnectInstruction.png");
        connect_md.ShowAll();
        int cd_result = connect_md.Run(); // Get user selection
        connect_md.Destroy();

        if (cd_result == -5) // Dialog OK
        {
            // Show message informing user that the wiimote is being searched
            MessageDialog searching_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.None, "Searching...")
            {
                Title = "Searching..."
            };

            // Start thread to connect to wiimotes
            Thread connect_manager = new Thread(() => { connection_status = ConnectWiimotes(); });
            connect_manager.Start();

            // While still attempting to connect
            while (connection_status == -5)
            {
                searching_md.ShowAll();
            }
            searching_md.Destroy();

            // If connection sucessful
            if (connection_status > 0)
            {
                // Show message to user with instructions on calibrating the gyroscope
                MessageDialog gyro_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "Gyroscope calibration will now begin.\n\nPlease place your wiimote face up on a flat surface and press OK.\n\nThis action will take up to 3 seconds.\n\nPlease do not touch or move the wiimote during this process.");
                gyro_md.Title = "Gyroscope calibration";
                gyro_md.Image = new Gtk.Image("./res/CalibrateInstruction.png");

                gyro_md.ShowAll();
                gyro_md.Run();
                gyro_md.Destroy();

                // Enable motion sensing and calibrate the gyroscope
                int status = EnableMotionSensing();

                // Show success message to user
                MessageDialog success_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok, 
                    "Wiimote connected!\n\nYou can now switch to game window and start playing!");
                success_md.ShowAll();
                success_md.Run();
                success_md.Destroy();

                // Enable the calibration and disconnect buttons, as well as the feedback frame
                // Disable the connect button
                calibrateButton.Sensitive = true;
                disconnectButton.Sensitive = true;
                feedbackFrame.Sensitive = true;
                connectButton.Sensitive = false;
                DisconnectAction.Sensitive = true;
                ConnectAction.Sensitive = false;

                // Spawn a thread to update the graphics for the taiko drum
                ThreadStart work2 = UpdateDrumGraphic;
                Thread drum_updater = new Thread(work2);
                drum_updater.Start();
            }
            else
            {
                // Show message to user informing that no wiimotes were detected / connected
                MessageDialog error_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Error, ButtonsType.Ok,
                    "No wiimotes were detected.\n\nPlease make sure that your bluetooth adapter is turned ON.");
                error_md.Title = "Connection error";
                error_md.Image = new Gtk.Image("./res/BluetoothError.png");

                error_md.ShowAll();
                error_md.Run();
                error_md.Destroy();

                // Reset connected wiimotes to searching
                connection_status = -5;
            }
        }
    }

    protected void CalibrateHandler(object sender, EventArgs e)
    {
        /* Calibrate the thresholds to user's movements */

        int md_result = 0;

        // Image with instructions on how to execute the movements
        Gtk.Image down_mvmnt_img = new Gtk.Image();
        Gtk.Image side_mvmnt_img = new Gtk.Image();
        down_mvmnt_img.PixbufAnimation = new Gdk.PixbufAnimation("./res/DownMvmnt.gif");
        side_mvmnt_img.PixbufAnimation = new Gdk.PixbufAnimation("./res/SideMvmnt.gif");

        // Calibrate downward movement

        // Show message with instructions for first move
        MessageDialog downfirst_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.OkCancel,
            "Calibration for down movements will now begin.\n\nPlease press OK and make the movement shown with your controller.")
        {
            Image = down_mvmnt_img
        };
        downfirst_md.ShowAll();
        md_result = downfirst_md.Run();
        downfirst_md.Destroy();

        if (md_result == -5) // Dialog OK
        {
            // Reset result
            md_result = 0;

            // Record user movements after OK is pressed
            CalibrateForce(0, 0);

            // Show message with instructions for second move
            MessageDialog downsecond_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                "Very good, now a second time.")
            {
                Image = down_mvmnt_img
            };
            downsecond_md.ShowAll();
            md_result = downsecond_md.Run();
            downsecond_md.Destroy();

            if (md_result == -5)
            {
                // Reset result
                md_result = 0;

                // Record user movements after OK is pressed 
                CalibrateForce(0, 1);

                // Show message with instructions for third move
                MessageDialog downthird_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                    "And lastly a third time.")
                {
                    Image = down_mvmnt_img
                };
                downthird_md.ShowAll();
                md_result = downthird_md.Run();
                downthird_md.Destroy();

                if (md_result == -5)
                {
                    // Reset result
                    md_result = 0;

                    // Record user movements after OK is pressed
                    CalibrateForce(0, 2);

                    // Calibrate sideways movement

                    // Show message with instructions for the first move
                    MessageDialog sidefirst_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                        "Calibration for sideways movements will now begin.\n\nPlease press OK and and make the movement shown with your controller.")
                    {
                        Image = side_mvmnt_img
                    };
                    sidefirst_md.ShowAll();
                    md_result = sidefirst_md.Run();
                    sidefirst_md.Destroy();

                    if (md_result == -5)
                    {
                        // Reset result
                        md_result = 0;

                        // Record user movements after OK is pressed
                        CalibrateForce(1, 0);

                        MessageDialog sidesecond_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                         "Very good, now a second time.")
                        {
                            Image = side_mvmnt_img
                        };
                        sidesecond_md.ShowAll();
                        md_result = sidesecond_md.Run();
                        sidesecond_md.Destroy();

                        if (md_result == -5)
                        {
                            // Record user movements affter OK is pressed
                            CalibrateForce(1, 1);

                            MessageDialog sidethird_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                             "And lastly a third time.")
                            {
                                Image = side_mvmnt_img
                            };
                            sidethird_md.ShowAll();
                            md_result = sidethird_md.Run();
                            sidethird_md.Destroy();

                            if (md_result == -5)
                            {
                                // Record user movements after OK is pressed
                                CalibrateForce(1, 2);
                            }
                        }
                    }
                }
            }
        }
    }

    protected void DisconnectHandler(object sender, EventArgs e)
    {
        /* Safely disconnect the wiimote */

        // Show message to user with instructions on disconnecting wiimote
        MessageDialog disconnect_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Question, ButtonsType.OkCancel,
             "Please hold the power button on your wiimote until LED 1 turns off and press OK.\n\nThis will disconnect the remote safely from your computer.");
        disconnect_md.Title = "Disconnect";
        disconnect_md.Image = new Gtk.Image("./res/DisconnectInstruction.png");

        disconnect_md.ShowAll();
        int dm_result = disconnect_md.Run();
        disconnect_md.Destroy();

        if (dm_result == -5) // Dialog OK
        {
            connection_status = DisconnectWiimotes();

            if (connection_status == 0)
            {
                // Show message to user informing disconnect was sucessful
                MessageDialog sucess_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
                    "Wiimote sucessfuly disconnected!\n\nIf you want to reconnect, simply press the connect button again.");
                sucess_md.Title = "Disconnection sucessful";
                sucess_md.Run();
                sucess_md.Destroy();

                // Set connection_status back to initial value
                connection_status = -5;

                // Disable the calibration and disconnect button, as well as feedback frame
                // Enable the connect button
                calibrateButton.Sensitive = false;
                disconnectButton.Sensitive = false;
                feedbackFrame.Sensitive = false;
                connectButton.Sensitive = true;
                DisconnectAction.Sensitive = false;
                ConnectAction.Sensitive = true;

                // Kill taiko updater thread and reset the shown image to the No Inputs
                exit = true;
                this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoNoInputs.png");
            }
            else
            {
                // Show message informing user the wiimote was unable to disconnect
                MessageDialog error_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Error, ButtonsType.Ok,
                    "No wiimotes were disconnected, please make sure that you held the power button until LED 1 was OFF.");
                error_md.Title = "Disconnection error";
                error_md.Run();
                error_md.Destroy();
            }
        }
    }

    protected void ToggleVibrationHandler(object sender, EventArgs e)
    {
        /* Toggle vibration on hit */
        ToggleVibration();
    }

    public void UpdateDrumGraphic()
    {
        int mvmnt = 0;
        bool b_pressed = false;
        while (!exit)
        {
            mvmnt = GetLastReadMovement();
            b_pressed = IsPressedB();
            switch (mvmnt)
            {
                case 1: // Left
                    if (drum_img_state != 1 && !b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoLeftSide.png");
                        drum_img_state = 1;
                    }
                    else if (drum_img_state != 5 && b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoDoubleSide.png");
                        drum_img_state = 4;
                    }
                    break;
                case 2: // Rightx'
                    if (drum_img_state != 2 && !b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoRightSide.png");
                        drum_img_state = 2;
                    }
                    else if (drum_img_state != 4 && b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoDoubleSide.png");
                        drum_img_state = 4;
                    }
                    break;
                case 3: // Center
                    if (drum_img_state != 3 && !b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoRightCenter.png");
                        drum_img_state = 3;
                    }
                    else if (drum_img_state != 5 && b_pressed)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoDoubleCenter.png");
                        drum_img_state = 5;
                    }
                    break;
                case 0:
                default: // None
                    if (drum_img_state != 0)
                    {
                        this.feedbackImage.Pixbuf = Gdk.Pixbuf.LoadFromResource("TRAB_IHC.res.TaikoNoInputs.png");
                        drum_img_state = 0;
                    }
                    break;
            }
        }
    }

    protected void OnConnectActionActibvated(object sender, EventArgs e)
    {
        ConnectHandler(sender, e);
    }

    protected void OnDisconnectActionActivated(object sender, EventArgs e)
    {
        DisconnectHandler(sender, e);
    }

    protected void OnControlsActionActivated(object sender, EventArgs e)
    {
        MessageDialog controls_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok, ""); ;
        controls_md.Title = "Controls";
        controls_md.Image = new Image("res/ControllerInstructions.png");

        controls_md.ShowAll();
        controls_md.Run();
        controls_md.Destroy();
    }

    protected void OnAboutActionActivated(object sender, EventArgs e)
    {
        MessageDialog about_md = new MessageDialog(this, DialogFlags.DestroyWithParent, MessageType.Info, ButtonsType.Ok,
            "OsuMote Version 1.0\nhttps://github.com/FabioAzevedoGomes/OsuMote\n" +
            "Made by:\n\n" +
            "Fábio A. Gomes\n" +
            "Caetano J. Stradolini\n");
        about_md.ShowAll();
        about_md.Run();
        about_md.Destroy();
    }
}
