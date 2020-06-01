#include <wiiuse.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_WIIMOTES 1

int main(void)
{

	// Accelerometers for X, Y and Z axis (force)
	struct gforce_t gravforce;
	// Orientation in the X, Y and Z axis (angles)
	struct orient_t orientation;

	// Initialize wiiuse library with 2 wiimotes
	wiimote** wiimotes = wiiuse_init(MAX_WIIMOTES);
	
	// Search for 10 seconds for up to 2 wiimotes
	int found = wiiuse_find(wiimotes, MAX_WIIMOTES, 10);

	// Attemp to connect the wiimotes
	int connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);

	if (connected)
	{
		printf("\nConnected to %d out of %d found wiimotes\n",connected, found);
		
		// Enabled motion sensing (Disabled by default)
		wiiuse_motion_sensing(wiimotes[0], 1);

		wiiuse_set_leds(wiimotes[0],WIIMOTE_LED_1);

		// Main loop
		int exit = 1;
		while(exit){
			
			// Poll the Wiimotes for updates
			if ( wiiuse_poll(wiimotes, MAX_WIIMOTES) )
			{
				printf("\nWiimote responded\n Detected event: %d",wiimotes[0]->event);
				
				// Identify event and react properly
				switch(wiimotes[0]->event)
				{
					case WIIUSE_EVENT:
						gravforce = wiimotes[0]->gforce;
						orientation = wiimotes[0]->orient;
						
						printf("\n[Acceleration]\n X: %.2f\n Y: %.2f\n Z: %.2f\n",gravforce.x,gravforce.y,gravforce.z);
						printf("\n[Orientation]\n Roll: %.2f\n Pitch: %.2f",orientation.roll,orientation.pitch);
					
						if (IS_HELD(wiimotes[0],WIIMOTE_BUTTON_A))
						{
							exit = 0;
						}
						break;
						
					// In case of a disconnect (Power down, low battery)
					case WIIUSE_DISCONNECT:
						printf("\nWiimote disconnected\n");
						
						break;
					case WIIUSE_STATUS:
						printf("\nStatus event occurred\n");
						break;
					default:
						break;
				}
			}
		}

		// Disconnect all wiimotes currently connected
		wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	}
	else
	{
		printf("\nFailed to connect to any wiimotes\n");
	}

	return 0;
}
