#include <stdio.h>
#include <wiiuse.h>
#include <stdlib.h>
#include <math.h>

#define MAX_WIIMOTES 1
#define DELTA_GRAV 1.5
#define NOISE_PASS 1.0

enum moves {LEFT, RIGHT, DOWN, NOTHING};
enum moves detected_last = NOTHING;
enum moves detected_last_not_updated = NOTHING; // Just for displaying readings


// Connect wiimote
int connect_wiimotes(wiimote** wiimotes)
{
    wiiuse_find(wiimotes, MAX_WIIMOTES, 10);
    return wiiuse_connect(wiimotes, MAX_WIIMOTES);
}

void enable_sensors(wiimote** wiimotes)
{
    wiiuse_motion_sensing(wiimotes[0], 1);
	wiiuse_set_leds(wiimotes[0],WIIMOTE_LED_1);
    //wiiuse_set_accel_threshold(wiimotes[0],ACCEL_THRESHOLD);
}

void simple_detect_movement(struct gforce_t last_grav, struct orient_t last_orient, struct gforce_t grav, struct orient_t  orient)
{
    // Simple (bad) solution 
    // (Barely works, has issues detecting the oposite direction at the end of movements, is not orientation based)
    double delta_x = last_grav.x - grav.x;
    double delta_z = last_grav.z - grav.z;

    double delta_pitch = last_orient.pitch - orient.pitch;

    int sm = 0; // Was a significant movement deteted?
    
    if (abs(delta_z) > NOISE_PASS)
    {
            printf("DOWN! ");
            detected_last = DOWN;
            sm = 1;
    }
    else if (abs(delta_x) > NOISE_PASS)
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
    

    if (sm)
        detected_last_not_updated = detected_last;
    else
        detected_last = NOTHING;
           
}

// Rotate accelerometer readings to controller coordinate system and subtract gravity from those
//                                    X,Y,Z                        pitch, roll, yaw
struct gforce_t remove_gravity(struct gforce_t accelerometer, struct orient_t orientation)
{

    // Convert angles to radians
    double r_pitch = -orientation.pitch * M_PI / 180;
    double r_roll  =  orientation.roll  * M_PI / 180;
    double r_yaw   =  orientation.yaw   * M_PI / 180; // Always 0 ( Not available =( )

    double res = 0;

    double accel_original[3] = {accelerometer.x,accelerometer.y,accelerometer.z};
    double accel_rotated_z[3] = {0,0,0};
    double accel_rotated_y[3] = {0,0,0};
    double accel_rotated_x[3] = {0,0,0};

    double rotate_X[3][3] = 
    {{1.0,        0.0,        0.0},
     {0.0, cos(r_pitch), -sin(r_pitch)},
     {0.0, sin(r_pitch),  cos(r_pitch)}};
    double rotate_Y[3][3] =
    {{ cos(r_roll), 0.0, sin(r_roll)},
     {       0.0, 1.0,       0.0},
     {-sin(r_roll), 0.0, cos(r_roll)}};  
    double rotate_Z[3][3] =
    {{ cos(r_yaw), -sin(r_yaw), 0.0},
     { sin(r_yaw),  cos(r_yaw), 0.0},
     {        0.0,         0.0, 1.0}};


    // Rotate accelerometer vector aroud Z axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_original[j]*rotate_Z[j][i];
        }
        accel_rotated_z[i] = res;
    }
    
    // Rotate accelerometer vector around Y axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_rotated_z[j]*rotate_Y[j][i];
        }
        accel_rotated_y[i] = res;
    }

    // Rotate accelerometer vector around X axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_rotated_y[j]*rotate_X[j][i];
        }
        accel_rotated_x[i] = res;
    }

    // Subtract gravity                                                                  HERE
    return (struct gforce_t){accel_rotated_x[0], accel_rotated_x[1], accel_rotated_x[2] - 1};
}

int main(void)
{
    // Accelerometer and orientation history (Last readings)
    struct gforce_t last_acceleration;
    struct orient_t last_orientation;
    
    // Accelerometer and orientation current 
    struct gforce_t acceleration;
    struct orient_t orientation;

    // Initialize readings with zeros
    last_acceleration = (struct gforce_t){ .x=0, .y=0, .z=1 };
    last_orientation = (struct orient_t){ .a_roll=0, .a_pitch=0, .roll=0, .pitch=0, .yaw=0 };

    // Init library
    wiimote** wiimotes = wiiuse_init(MAX_WIIMOTES);
    
    if (connect_wiimotes(wiimotes))
    {
        // Enable motion sensing and LEDs
        enable_sensors(wiimotes);

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
					
					    // Get orientation angles in degrees
					    orientation = wiimotes[0]->orient;
					
					    // Get acceleration vector without gravity interference
					    acceleration = remove_gravity( wiimotes[0]->gforce, orientation );
					
                        // Look for significant movement 
                        simple_detect_movement(last_acceleration, last_orientation, acceleration, orientation);
                        
                        // Update last event readings
                        last_acceleration = acceleration;
						last_orientation  = orientation;

						// Debug
						printf("\e[1;1H\e[2J");
					
                        printf("\n[WIIMOTE]\n[Acceleration] [gforces]\n X: %.2f\n Y: %.2f\n Z: %.2f\n",last_acceleration.x,last_acceleration.y,last_acceleration.z);
						printf("\n[Orientation] [degrees]\n Roll: %.2f\n Pitch: %.2f\n Abs. Roll: %.2f\n Abs. Pitch: %.2f\n",last_orientation.roll,last_orientation.pitch,last_orientation.a_roll, last_orientation.a_pitch);
						printf("\n[Movement] [0 - Left, 1 - Right, 2 - Down, 3 - None]\n %d\n",detected_last_not_updated);

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
