#include "controllermanager.h"

namespace ControllerManager
{
    // Status variables
    static CWii wii;
    static int connected_wiimotes;
    static double recorded_rates[MAX_RECORDINGS][3];
    static int position_index; // Last place where a rate trio was stored
    static directions last_read, last_direction; // Last detected reading and actual last detected direction, Difference is last_direction won't be NONE
    static Display* display;
    static bool vibration; // Whether vibration is enabled or not
    static bool sound; // Whether sound is enabled or not
    static char* center_sound, sides_sound; // Path to the sound files to be played through the speakers

    // Calibration and detection variables
    static double angle_rate_threshold_down  = 900; // Angle rate limit for pitch to be detected as down
    static double angle_rate_threshold_sides = 500; // Angle rate limit for yaw to be detected as sides
    static double calibration_angles[MAX_CALIBRATION_RECORDINGS][3];
    static int    calibration_index = 0; // Position in the calibration array 
    static int    calibration_turn  = 0; // Turn of calibration (0 1 or 2)
    static int    calibration_axis  = 0; // Axis currently being calibrated (0 is down, 1/2 is sides)
    static int    calibrating      = 0; // Flag if calibration is currently ongoing

    // ===================================================================================
    // Non-exposed functions
    // ===================================================================================

    void set_index(int n)
    {
        position_index = n;
    }
    //                             double recorded_rates[MAX_RECORDINGS][3]
    void max_rates(double* result, double recorded_rates[][3], int size)
    {
	    for (int i=0; i < 3; i ++)
	    {
		    double max = 0;
		    for (int j=0;j<size;j++)
		    {
			    if (abs(recorded_rates[j][i]) > abs(max) )
			    {
				    max = recorded_rates[j][i];
			    }
		    }
		    result[i] = max;
	    }
    }

    // Simulates a key press based on the direction and if a double input must be sent
    void SendToGame(directions direction, int double_input)
    {
    
        int keycode;
    
        if (double_input)
        { // If the B key is held, moves should be interpreted as double input,
            // and thus mapped to DOUBLE_CENTER or RIGHT_AND_LEFT according to axis
            if (direction == DOWN)
            {
	            std::cout << "Movement was:  DOUBLE CENTER" << std::endl;
	            keycode = XKeysymToKeycode(display, XK_F);
	            // Press the key
                XTestFakeKeyEvent(display, keycode, 1, 0);
                // Release the key
                XTestFakeKeyEvent(display, keycode, 0, 0);
                
                keycode = XKeysymToKeycode(display, XK_J);
            }
            else
            {
	            std::cout << "Movement was: LEFT + RIGHT" << std::endl;
	            keycode = XKeysymToKeycode(display, XK_D);
	            // Press the key
                XTestFakeKeyEvent(display, keycode, 1, 0);
                // Release the key
                XTestFakeKeyEvent(display, keycode, 0, 0);
                // Prepare next key
                keycode = XKeysymToKeycode(display, XK_K);
            }	
        }
        else
        { // If the key is not held, moves should be interpreted as simple moves
            switch (direction){
	            case DOWN: 
		            std::cout << "Movement was: DOWN" << std::endl;
		            keycode = XKeysymToKeycode(display, XK_F);
		            break;
	            case LEFT:
		            std::cout << "Movement was: LEFT" << std::endl;
		            keycode = XKeysymToKeycode(display, XK_D);
		            break;
	            case RIGHT:
		            std::cout << "Movement was: RIGHT" << std::endl;
		            keycode = XKeysymToKeycode(display, XK_K);
		            break;
		        case UP:
		            keycode = XKeysymToKeycode(display, XK_Escape);
		            break;
		        default:
		            break;
            }
        }

        // Press the key
        XTestFakeKeyEvent(display, keycode, 1, 0);
        // Release the key
        XTestFakeKeyEvent(display, keycode, 0, 0);
        // Send
        XFlush(display);
        
    }

    void HandleDisconnect(CWiimote &wm) 
    {   // Pause game on controller disconnect
        SendToGame(UP, 0);
    }

    // Detect direction of movement
    void detect_movement(int is_pressed_B)
    {

	    double solution[3];
	    // Get the max rates for all 3 axis
	    max_rates(solution, recorded_rates, MAX_RECORDINGS);
        
        int significantMovement = 0; // Flag if there was a significant move detected
        
	    // If pitch change was greater than yaw change in the last MAX_RECORDINGS readings
	    // and greater than the threshold
        if (abs(solution[0]) > abs(solution[2]) && solution[0] > angle_rate_threshold_down)
        {
            last_read = DOWN;
            significantMovement = 1;
        }
	    // Else if yaw change was greater than the treshold
	    else if (abs(solution[2]) > angle_rate_threshold_sides)
        {
            if (solution[2] > 0 && last_read != RIGHT)
            {
                last_read = LEFT;
            }
            else
            {
                if (last_read != LEFT) last_read = RIGHT;
            }
            significantMovement = 1;
        }


	    // If there was a significant movement
	    if (significantMovement)
        {
            last_direction = last_read;
        }
        else
        {
            if (last_read != NONE)
		    {
			    // If this is the case, some movement just finished and should be sent to the game 
			    // in the form of a kepress
			    SendToGame(last_read, is_pressed_B);
                
                /* TODO Check for sound and vibration*/

		    }
            // wether a move was
		    last_read = NONE;
        }
    }

    // Handle button presses if there are any
    void HandleButtons(CWiimote &wm)
    {
    
        int keycode;
    
        // If plus button is pressed, pause the game  (ESC)
        if (wm.Buttons.isJustPressed(CButtons::BUTTON_PLUS))
        {
            keycode = XKeysymToKeycode(display, XK_Escape);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        }
        if (wm.Buttons.isJustPressed(CButtons::BUTTON_A))
        {
            keycode = XKeysymToKeycode(display, XK_Return);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        }
        if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))
        {
            keycode = XKeysymToKeycode(display, XK_Up);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        }

        if(wm.Buttons.isJustPressed(CButtons::BUTTON_DOWN))
        {
            keycode = XKeysymToKeycode(display, XK_Down);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        }

        if(wm.Buttons.isJustPressed(CButtons::BUTTON_LEFT))
        {
            keycode = XKeysymToKeycode(display, XK_Left);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        }

        if(wm.Buttons.isJustPressed(CButtons::BUTTON_RIGHT))
        {
            keycode = XKeysymToKeycode(display, XK_Right);
            XTestFakeKeyEvent(display, keycode, 1, 0);
            XTestFakeKeyEvent(display, keycode, 0, 0);
            XFlush(display);
        } 
    }

    // Handle a sensor update
    void HandleEvent(CWiimote &wm)
    {

	    // Get pitch roll and yaw rates
        float pitch_rate,roll_rate,yaw_rate;
        wm.ExpansionDevice.MotionPlus.Gyroscope.GetRates(roll_rate,pitch_rate,yaw_rate);
        
        if (calibrating && calibration_index < MAX_CALIBRATION_RECORDINGS)
        {
            switch (calibration_axis){
                
                case 0:
                    calibration_angles[calibration_index][calibration_turn] = pitch_rate;
                    break;
                case 1:
                case 2:
                    calibration_angles[calibration_index][calibration_turn] = yaw_rate;
                default:
                    break;
            }
            calibration_index++;
        }
        
	    // Detect movement
	    detect_movement(wm.Buttons.isHeld(CButtons::BUTTON_B));

	    // Update rates history
        recorded_rates[position_index][0] = pitch_rate;
        recorded_rates[position_index][1] =  roll_rate;
        recorded_rates[position_index][2] =   yaw_rate;

        set_index(position_index + 1);
        if (position_index >= MAX_RECORDINGS) set_index(0);	// Write over the oldest reading

	    // Print debug stuff
	    
    }

    // ===================================================================================
    // Exposed functions
    // ===================================================================================
    extern "C" void init()
    {
        vibration = false;
        sound = false;
        set_connected_wiimotes(0);
        set_index(0);
        display = XOpenDisplay(NULL);
    }
    
    extern "C" void set_connected_wiimotes(int n)
    {
        connected_wiimotes = n;
    }

    extern "C" int connect_wiimotes()
    {
        int index = 0;
        std::vector<CWiimote>::iterator i;

        // LED Map for the wiimote
        int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4};

         std::vector<CWiimote>& wiimotes = wii.FindAndConnect();
        
        if (!wiimotes.size())
            return ERROR_BLUETOOTH_OFF; // No wiimotes found and/or bluetooth is turned OFF
        else
        {
            for (index = 0, i = wiimotes.begin(); i != wiimotes.end(); ++i , ++index)
            {
                CWiimote & wiimote = *i;
                // Set leds
                wiimote.SetLEDs(LED_MAP[index]);
            }

            connected_wiimotes = wiimotes.size(); // Update connected count so controller manager can start detecting events
            
            return wiimotes.size(); // Sucessfully connected to wiimotes.size() wiimotes
        }
    }

    extern "C" int disconnect_wiimotes()
    {
        // First we tell user to hold power button, then call this function
    
        std::vector<CWiimote>& wiimotes = wii.GetWiimotes();
        
        return wiimotes.size();
        
    }

    extern "C" int enable_motion_sensing()
    {
        int index = 0;
        std::vector<CWiimote>& wiimotes = wii.GetWiimotes();
        std::vector<CWiimote>::iterator i;
        
        
        for (index = 0, i = wiimotes.begin(); i != wiimotes.end(); ++i , ++index)
        {
            CWiimote & wiimote = *i;
            
            // Enable motion sensing and motion plus
            wiimote.SetMotionSensingMode(CWiimote::ON);
            wiimote.EnableMotionPlus(CWiimote::ON);
        }
        
        return 0;
    }

    extern "C" void controller_manager()
    {
    
        int reload_wiimotes = 0;
    
        // Wait for first wiimote connection
	    while(connected_wiimotes <= 0 && (connected_wiimotes != EXIT_SIGNAL)){}

	    // Get vector containing wiimotes
	    std::vector<CWiimote>& wiimotes = wii.GetWiimotes();
	    std::vector<CWiimote>::iterator i;

	    // Start main loop
        int exit = 0;
        while (!exit && connected_wiimotes != EXIT_SIGNAL)
        {   
            
            if (reload_wiimotes)
            {
                wiimotes = wii.GetWiimotes();
                reload_wiimotes = 0;
            }
            
		    // Poll the Wiimotes for updates
		    if ( wii.Poll() )
		    {
			    // For each wiimote present
			    for (i = wiimotes.begin(); i != wiimotes.end(); ++i)
			    {
				    CWiimote & wiimote = *i;
				    switch(wiimote.GetEvent())
				    {
					    case CWiimote::EVENT_EVENT:
					        // Handle button inputs from the user (Dpad and +)
					        HandleButtons(wiimote);
						    // Handle that event			
						    HandleEvent(wiimote);
						    break;
					    case CWiimote::EVENT_DISCONNECT:
					        // User disconnect (Power button)
					        reload_wiimotes = 1;
					        break;
					    case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
					        // Unexpected disconenct (Battery)
					    	HandleDisconnect(wiimote);
						    reload_wiimotes = 1;
						    break;
					    case CWiimote::EVENT_STATUS:
						    // Handle status event
						    break;
					    default:
						    break;
				    }
			    }
		    }
	    }
        
        // Disconnect from X
        XCloseDisplay(display);

        // return
    }
    
    extern "C" void calibrate_force(int axis, int turn)
    {
        /*  
        static int angle_rate_threshold_down  = 900; // Angle rate limit for pitch to be detected as down
        static int angle_rate_threshold_sides = 500; // Angle rate limit for yaw to be detected as sides

        static int calibration_angles[MAX_CALIBRATION_RECORDINGS][3];
        */
        if (turn == 0) // If it's the first step, clear the recording array
        {
            for (int i = 0 ; i < MAX_CALIBRATION_RECORDINGS; i++)
            {
                calibration_angles[i][0] = 0.0;
                calibration_angles[i][1] = 0.0;
                calibration_angles[i][2] = 0.0;
            }
        }
        
        /* Tell controller manager to record the rates */
        calibration_index = 0;
        calibration_turn = turn;
        calibration_axis = axis;
        calibrating = 1;
        
        // Wait until all rates are recorded
        while(calibration_index < MAX_CALIBRATION_RECORDINGS) { }
        
        calibrating = 0;
        
        if (turn == 1) // If it's the last step, calculate average max
        {
            double result[3];
            // Get max for each of the 3 recordings
            max_rates(result, calibration_angles, MAX_CALIBRATION_RECORDINGS);
            
            // Average these max values
            //             recording 0   recording 1  recording 2 
            double average = (result[0] + result[1] + result[2])/3;
            
            if (axis == 0) // If down axis
            {
                angle_rate_threshold_down = average;            
            }
            else // If side axis
            {
                angle_rate_threshold_sides = average;
            }
        }
        // return
    }

    extern "C" void toggle_vibration()
    {
        vibration = !vibration;
    }

    extern "C" void toggle_sound()
    {
        sound = !sound;
    }

    extern "C" int set_sound(char* filename, int type)
    {
        // TODO Check if file exists and is a valid sound file
        // IF valid: 
        //Save file to sides_sound / center_sound and play it once to signal sucess
        return 0;
    }

    extern "C" void play_sound(int type)
    {
        // TODO Play the set sound for that type through the wiimote speakers
    }
}
