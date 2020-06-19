#include "controllermanager.h"

namespace ControllerManager
{

    // Information about the current status of the application
    static struct status_t      application_status;
    static struct refs_t        application_references;
    static struct calibration_t calibration_status;

    // Xinput display for sending keypresses
    static Display* display;

    // ===================================================================================
    // Non-exposed functions
    // ===================================================================================

    // Updates last recorded angle change rates
    void UpdateRates(double pitch_rate, double roll_rate, double yaw_rate)
    {
	    // Update rates history
        application_references.recorded_rates[application_references.position_index][0] = pitch_rate;
        application_references.recorded_rates[application_references.position_index][1] =  roll_rate;
        application_references.recorded_rates[application_references.position_index][2] =   yaw_rate;

        application_references.position_index++;
        if (application_references.position_index >= MAX_RECORDINGS) 
            // Write over the oldest reading
            application_references.position_index = 0;

    }

    // Returns in result the maximum found angle change for each axis
    void MaxAngleChangeRates(double* result, double recorded_rates[][3], int size)
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

    // Handle an unexpected controller disconnect (Pause the game)
    void HandleDisconnect(CWiimote &wm) 
    {
        SendToGame(UP, 0);
    }

    // Detect direction of movement
    void DetectMovement(CWiimote &wm)
    {
        int is_pressed_B = wm.Buttons.isHeld(CButtons::BUTTON_B);
        int significantMovement = 0; // Flag if there was a significant move detected
	    double solution[3]; // Max angle change rates for each axis

	    // Get the max rates for all 3 axis
	    MaxAngleChangeRates(solution, application_references.recorded_rates, MAX_RECORDINGS);
        
	    // If pitch change was greater than yaw change in the last MAX_RECORDINGS readings
	    // and greater than the threshold
        if (abs(solution[0]) > abs(solution[2]) && solution[0] > calibration_status.angle_rate_threshold_down)
        {
            application_references.last_read = DOWN;
            significantMovement = 1;
        }
	    // Else if yaw change was greater than the treshold
	    else if (abs(solution[2]) > calibration_status.angle_rate_threshold_sides)
        {
            if (solution[2] > 0 && application_references.last_read != RIGHT)
            {
                application_references.last_read = LEFT;
            }
            else
            {
                if (application_references.last_read != LEFT) application_references.last_read = RIGHT;
            }
            significantMovement = 1;
        }

	    // If there was a significant movement
	    if (significantMovement)
        {
            application_references.last_direction = application_references.last_read;
        }
        else
        {
            if (application_references.last_read != NONE)
		    {

			    // If this is the case, some movement just finished and should be sent to the game 
			    // in the form of a kepress
			    SendToGame(application_references.last_read, is_pressed_B);
                
                /* TODO Check for sound and vibration*/
                if (application_status.vibration)
                {
                    wm.ToggleRumble();
                    usleep(100000);
                    wm.ToggleRumble();
                }
		    }

            // Reset last read after movement was processed
		    application_references.last_read = NONE;
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
        
        if (application_status.calibrating && calibration_status.calibration_index < MAX_CALIBRATION_RECORDINGS)
        {
            switch (calibration_status.calibration_axis){
                
                case 0:
                    calibration_status.calibration_angles[calibration_status.calibration_index][calibration_status.calibration_turn] = pitch_rate;
                    break;
                case 1:
                case 2:
                    calibration_status.calibration_angles[calibration_status.calibration_index][calibration_status.calibration_turn] = yaw_rate;
                default:
                    break;
            }
            calibration_status.calibration_index++;
        }
        
	    // Detect movement
	    DetectMovement(wm);

        // Update latest readings
	    UpdateRates(pitch_rate, roll_rate, yaw_rate);
    }

    // ===================================================================================
    // Exposed functions
    // ===================================================================================

    extern "C" void InitLibrary()
    {
        // Start sound, vibration and calirbation flags as false
        application_status.vibration   = false;
        application_status.sound       = false;
        application_status.calibrating = false;

        // Set connected wiimote count and reading index to 0
        application_references.connected_wiimotes = 0;
        application_references.position_index = 0;

        // Set angle thresholds to default
        calibration_status.angle_rate_threshold_down  = 900;
        calibration_status.angle_rate_threshold_sides = 500;
        
        // Set calibration data as 0 (Not started)
        calibration_status.calibration_index = 0;
        calibration_status.calibration_turn  = 0;
        calibration_status.calibration_axis  = 0;

        display = XOpenDisplay(NULL);
    }
    
    extern "C" void Finish()
    {
        application_references.connected_wiimotes = EXIT_SIGNAL;
    }

    extern "C" int  ConnectWiimotes()
    {
        int index = 0;
        std::vector<CWiimote>::iterator i;

        // LED Map for the wiimote
        int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4};

        std::vector<CWiimote>& wiimotes = application_references.wii.FindAndConnect();
        
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

            application_references.connected_wiimotes = wiimotes.size(); // Update connected count so controller manager can start detecting events
            
            return wiimotes.size(); // Sucessfully connected to wiimotes.size() wiimotes
        }
    }

    extern "C" int  DisconnectWiimotes()
    {
        // First we tell user to hold power button, then call this function
    
        std::vector<CWiimote>& wiimotes = application_references.wii.GetWiimotes();
        
        return wiimotes.size();
        
    }

    extern "C" int  EnableMotionSensing()
    {
        int index = 0;
        std::vector<CWiimote>& wiimotes = application_references.wii.GetWiimotes();
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

    extern "C" void ControllerManager()
    {
    
        int reload_wiimotes = 0;
    
        // Wait for first wiimote connection
	    while(application_references.connected_wiimotes <= 0 && (application_references.connected_wiimotes != EXIT_SIGNAL)){}

	    // Get vector containing wiimotes
	    std::vector<CWiimote>& wiimotes = application_references.wii.GetWiimotes();
	    std::vector<CWiimote>::iterator i;

	    // Start main loop
        int exit = 0;
        while (!exit && application_references.connected_wiimotes != EXIT_SIGNAL)
        {   
            
            if (reload_wiimotes)
            {
                wiimotes = application_references.wii.GetWiimotes();
                reload_wiimotes = 0;
            }
            
		    // Poll the Wiimotes for updates
		    if ( application_references.wii.Poll() )
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

        // return;
    }
    
    extern "C" void CalibrateForce(int axis, int turn)
    {

        if (turn == 0) // If it's the first step, clear the recording array
        {
            for (int i = 0 ; i < MAX_CALIBRATION_RECORDINGS; i++)
            {
                calibration_status.calibration_angles[i][0] = 0.0;
                calibration_status.calibration_angles[i][1] = 0.0;
                calibration_status.calibration_angles[i][2] = 0.0;
            }
        }
        
        /* Tell controller manager to record the rates */
        calibration_status.calibration_index = 0;
        calibration_status.calibration_turn = turn;
        calibration_status.calibration_axis = axis;
        application_status.calibrating = true;
        
        // Wait until all rates are recorded
        while(calibration_status.calibration_index < MAX_CALIBRATION_RECORDINGS) { }
        
        // Stop recording
        application_status.calibrating = 0;
        
        if (turn == 1) // If it's the last step, calculate average max
        {
            double result[3];
            // Get max for each of the 3 recordings
            MaxAngleChangeRates(result, calibration_status.calibration_angles, MAX_CALIBRATION_RECORDINGS);
            
            // Average these max values
            //             recording 0   recording 1  recording 2 
            double average = (result[0] + result[1] + result[2])/3;
            
            if (axis == 0) // If down axis
            {
                calibration_status.angle_rate_threshold_down = average;            
            }
            else // If side axis
            {
                calibration_status.angle_rate_threshold_sides = average;
            }
        }
        // return
    }

    extern "C" void ToggleVibration()
    {
        application_status.vibration = !application_status.vibration;
    }

    extern "C" void ToggleSound()
    {
        application_status.sound = !application_status.sound;
    }

    extern "C" int  SetSound(char* filename, int type)
    {
        // TODO Check if file exists and is a valid sound file
        // IF valid: 
        //Save file to sides_sound / center_sound and play it once to signal sucess
        return 0;
    }

    extern "C" void PlaySound(int type)
    {
        // TODO Play the set sound for that type through the wiimote speakers
    }
}
