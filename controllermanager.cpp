#include <iostream>
#include "controllermanager.h"

ControllerManager::ControllerManager(CWii* main_wii, int* connected_count)
{
	wii = main_wii; 
	connected = connected_count;
	last_direction = NONE;
	last_read = NONE;
}

ControllerManager::~ControllerManager(){}

// Return max absolute value for each roll, pitch and yaw
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

directions ControllerManager::detect_movement(int is_pressed_B)
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

// Simulates a key press based on the direction and if a double input must be sent
void ControllerManager::SendToGame(directions direction, int double_input)
{
	if (double_input)
	{ // If the B key is held, moves should be interpreted as double input,
		// and thus mapped to DOUBLE_CENTER or RIGHT_AND_LEFT according to axis
		if (direction == DOWN)
		{
			std::cout << "Movement was:  DOUBLE CENTER" << std::endl;
		}
		else
		{
			std::cout << "Movement was: LEFT + RIGHT" << std::endl;
		}	
	}
	else
	{ // If the key is not held, moves should be interpreted as simple moves
		switch (direction){
			case DOWN: 
				std::cout << "Movement was: DOWN" << std::endl;
				break;
			case LEFT:
				std::cout << "Movement was: LEFT" << std::endl;
				break;
			case RIGHT:
				std::cout << "Movement was: RIGHT" << std::endl;
				break;
		}
	}
}

void ControllerManager::HandleEvent(CWiimote &wm)
{
	// Get pitch roll and yaw rates
    float pitch_rate,roll_rate,yaw_rate;
    wm.ExpansionDevice.MotionPlus.Gyroscope.GetRates(roll_rate,pitch_rate,yaw_rate);

	// Detect movement
	ControllerManager::detect_movement(wm.Buttons.isHeld(CButtons::BUTTON_B));

	// Update rates history
    recorded_rates[index][0] = pitch_rate;
    recorded_rates[index][1] =  roll_rate;
    recorded_rates[index][2] =   yaw_rate;

    index++;
    if (index >= MAX_RECORDINGS) index = 0;	// Write over the oldest reading

	// Print debug stuff
	
}

void ControllerManager::HandleDisconnect(CWiimote &wm)
{

}

int ControllerManager::spawn()
{

	// Wait for first wiimote connection
	while(*connected <= 0 && (*connected != EXIT_SIGNAL)){}

	// Get vector containing wiimotes
	std::vector<CWiimote>& wiimotes = (*wii).GetWiimotes();
	std::vector<CWiimote>::iterator i;

	// Start main loop
    int exit = 0;
    while (!exit && (*connected) != EXIT_SIGNAL)
    {   
		// Poll the Wiimotes for updates
		if ( (*wii).Poll() )
		{
			// For each wiimote present
			for (i = wiimotes.begin(); i != wiimotes.end(); ++i)
			{
				CWiimote & wiimote = *i;
				switch(wiimote.GetEvent())
				{
					case CWiimote::EVENT_EVENT:
						// Handle that event			
						ControllerManager::HandleEvent(wiimote);
						break;
					case CWiimote::EVENT_DISCONNECT:
						// Power button pressed
						ControllerManager::HandleDisconnect(wiimote);
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
    
    return 0;
}
