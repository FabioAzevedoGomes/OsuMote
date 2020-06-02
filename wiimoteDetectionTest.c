#include <stdio.h>
#include <wiiuse.h>
#include <stdlib.h>
#include <time.h>

#define MAX_WIIMOTES 1
#define DELTA_GRAV 1.5
#define ACCEL_THRESHOLD 2
#define NOISE_PASS 0.5


enum moves {LEFT, RIGHT, DOWN, NOTHING};
enum moves detected_last = NOTHING;


// Connect wiimote
int connect_wiimotes(wiimote** wiimotes)
{
    wiiuse_find(wiimotes, MAX_WIIMOTES, 10);
    return wiiuse_connect(wiimotes, MAX_WIIMOTES);
}

void simple_detect_movement(struct gforce_t last_grav, struct orient_t last_orient, struct gforce_t grav, struct orient_t  orient)
{
    // Simple (bad) solution 
    // (Barely works, has issues detecting the oposite direction at the end of movements, is not orientation based)
    float delta_x = last_grav.x - grav.x;
    float delta_z = last_grav.z - grav.z;

    int sm = 0;

    if (abs(grav.x) > NOISE_PASS)
    {
        if (delta_x > DELTA_GRAV && detected_last != RIGHT )
        {
            printf("LEFT! ");
            detected_last = LEFT;
            sm = 1;
        }
        else if (delta_x < -DELTA_GRAV && detected_last != LEFT)
        {
            printf("RIGHT! ");
            detected_last = RIGHT;
            sm = 1;
        }
    }
    if (abs(grav.z) > NOISE_PASS && delta_z < - DELTA_GRAV)
    {
            printf("DOWN! ");
            detected_last = DOWN;
            sm = 1;
    }

    if (sm) 
        printf("\n");
    else
        detected_last = NOTHING;
    
    
}

int main(void)
{

    // Accelerometer and orientation history (Last readings)
    struct gforce_t last_grav;
    struct orient_t last_orient;

    // Initialize readings with zeros
    last_grav = (struct gforce_t){ .x=0, .y=0, .z=1};
    last_orient = (struct orient_t){.a_roll=0, .a_pitch =0, .roll=0, .pitch=0, .yaw=0};

    wiimote** wiimotes = wiiuse_init(MAX_WIIMOTES);
    
    if (connect_wiimotes(wiimotes))
    {
        wiiuse_motion_sensing(wiimotes[0], 1);
		wiiuse_set_leds(wiimotes[0],WIIMOTE_LED_1);
        //wiiuse_set_accel_threshold(wiimotes[0],ACCEL_THRESHOLD);

        // Main loop
		int exit = 1;
		while(exit){
			
			// Poll the Wiimotes for updates
			if ( wiiuse_poll(wiimotes, MAX_WIIMOTES) )
			{
				// Identify event update and react properly
				switch(wiimotes[0]->event)
				{
					case WIIUSE_EVENT:
                        // Look for significant movement 
                        simple_detect_movement(last_grav, last_orient,wiimotes[0]->gforce,wiimotes[0]->orient);
                        
                        // Update last event readings
                        last_grav = wiimotes[0]->gforce;
						last_orient  = wiimotes[0]->orient;
					
						if (IS_HELD(wiimotes[0],WIIMOTE_BUTTON_A))
						{
							exit = 0;
						}
						break;
						
					// In case of a disconnect (Power down, low battery)
					case WIIUSE_DISCONNECT:
						printf("\nWiimote disconnected\n");
                            exit = 0;
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