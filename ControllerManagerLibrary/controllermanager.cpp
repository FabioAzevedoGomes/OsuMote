#include "controllermanager.h"

namespace ControllerManager
{
    // Important vars
    static CWii wii;
    static int connected_wiimotes;
    static double recorded_rates[MAX_RECORDINGS][3];
    static int position_index; // Last place where a rate trio was stored
    static directions last_read, last_direction; // Last detected reading and actual last detected direction, Difference is last_direction won't be NONE
    static Display* display;

    // ===================================================================================
    // Non-exposed functions
    // ===================================================================================

    void set_index(int n)
    {
        position_index = n;
    }

    void max_rates(double* result, double recorded_rates[MAX_RECORDINGS][3])
    {
	    for (int i=0; i < 3; i ++)
	    {
		    double max = 0;
		    for (int j=0;j<MAX_RECORDINGS;j++)
		    {
			    if (abs(recorded_rates[j][i]) > abs(max) )
			    {
				    max = recorded_rates[j][i];
			    }
		    }
		    result[i] = max;
	    }
    }

    void HandleDisconnect(CWiimote &wm) 
    {
        connected_wiimotes--;
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
            }
        }

        // Press the key
        XTestFakeKeyEvent(display, keycode, 1, 0);
        // Release the key
        XTestFakeKeyEvent(display, keycode, 0, 0);
        // Send
        XFlush(display);
        
    }

    // Detect direction of movement
    void detect_movement(int is_pressed_B)
    {

	    double solution[3];
	    // Get the max rates for all 3 axis
	    max_rates(solution, recorded_rates);
        
        int significantMovement = 0; // Flag if there was a significant move detected
        
	    // If pitch change was greater than yaw change in the last MAX_RECORDINGS readings
	    // and greater than the threshold
        if (abs(solution[0]) > abs(solution[2]) && solution[0] > ANGLE_RATE_THRESHOLD_DOWN)
        {
            last_read = DOWN;
            significantMovement = 1;
        }
	    // Else if yaw change was greater than the treshold
	    else if (abs(solution[2]) > ANGLE_RATE_THRESHOLD_SIDES)
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
		    }
            // wether a move was
		    last_read = NONE;
        }
    }

    // Handle a sensor update
    void HandleEvent(CWiimote &wm)
    {
	    // Get pitch roll and yaw rates
        float pitch_rate,roll_rate,yaw_rate;
        wm.ExpansionDevice.MotionPlus.Gyroscope.GetRates(roll_rate,pitch_rate,yaw_rate);

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
        // Wait for first wiimote connection
	    while(connected_wiimotes <= 0 && (connected_wiimotes != EXIT_SIGNAL)){}

	    // Get vector containing wiimotes
	    std::vector<CWiimote>& wiimotes = wii.GetWiimotes();
	    std::vector<CWiimote>::iterator i;

	    // Start main loop
        int exit = 0;
        while (!exit && connected_wiimotes != EXIT_SIGNAL)
        {   
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
						    // Handle that event			
						    HandleEvent(wiimote);
						    break;
					    case CWiimote::EVENT_DISCONNECT:
						    // Power button pressed
						    HandleDisconnect(wiimote);
						    exit = 1;
						    break;
					    case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
						    // Pause
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
}
