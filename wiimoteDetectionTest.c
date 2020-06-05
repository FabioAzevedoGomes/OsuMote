#include <stdio.h>
#include <wiiuse.h>
#include <stdlib.h>
#include <math.h>

#define MAX_WIIMOTES 1
#define MAX_RECORDINGS 128

enum moves {LEFT, RIGHT, DOWN, NOTHING};
enum moves detected_last = NOTHING;
enum moves detected_last_not_updated = NOTHING; // Just for displaying readings

// Saved recordings of acceleration values
double recorded_accelerations[MAX_RECORDINGS][3];

// Connect wiimote
int connect_wiimotes(wiimote** wiimotes)
{
    wiiuse_find(wiimotes, MAX_WIIMOTES, 10);
    return wiiuse_connect(wiimotes, MAX_WIIMOTES);
}

// Enable LEDs and motion sensing on connected wiimotes
void enable_sensors(wiimote** wiimotes)
{
    wiiuse_motion_sensing(wiimotes[0], 1);
	wiiuse_set_leds(wiimotes[0],WIIMOTE_LED_1);
    wiiuse_set_flags(wiimotes[0],WIIUSE_CONTINUOUS, WIIUSE_SMOOTHING);
    //wiiuse_set_accel_threshold(wiimotes[0],ACCEL_THRESHOLD);
}

// Detect if a significant movement was performed in some direction
//                              accceleration (X,Y,Z)      orientation(pitich, yaw)  last acceleration index
void detect_movement(struct gforce_t acceleration, struct orient_t orientation, int index)
{
    // Retrieve current accelerometer readings and last accelerometer readings
    double x = acceleration.x, y = acceleration.y, z = acceleration.z;
    double last_x = recorded_accelerations[index][0],last_y = recorded_accelerations[index][1], last_z = recorded_accelerations[index][2];

    // Retrieve current orientation angles ( in radians )
    double pitch = - orientation.pitch * M_PI/180, roll = orientation.roll * M_PI/180, yaw = orientation.yaw * M_PI/180;

           
}

// Rotate accelerometer readings to world coordinate system, subtract gravity and rotate back to controller coordinate system
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

    accel_rotated_x[2] = accel_rotated_x[2] - 1;

    // Rotate accelerometer vector around X axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_rotated_x[j]*rotate_X[i][j];
        }
        accel_rotated_y[i] = res;
    }

    // Rotate accelerometer vector around Y axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_rotated_y[j]*rotate_Y[i][j];
        }
        accel_rotated_z[i] = res;
    }     
    
    // Rotate accelerometer vector aroud Z axis
    for (int i = 0; i < 3; i++)
    {
        res = 0;
        for (int j=0; j<3; j++)
        {
            res = res + accel_rotated_z[j]*rotate_Z[i][j];
        }
        accel_original[i] = res;
    }
    
    return (struct gforce_t){.x=accel_original[0], .y=accel_original[1], .z=accel_original[2]};
    // Subtract gravity                                                                  HERE
    //return (struct gforce_t){accel_rotated_x[0], accel_rotated_x[1], accel_rotated_x[2] - 1};
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

    // Index for saved accelerations array
    int index = 0;

    // Init library
    wiimote** wiimotes = wiiuse_init(MAX_WIIMOTES);
    
    if (connect_wiimotes(wiimotes))
    {
        // Enable motion sensing and LEDs
        enable_sensors(wiimotes);

        // Main loop
		int exit = 0;
		while(!exit)
        {
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
                        // TODO Remove the noise from acceleration
                        acceleration = remove_gravity( wiimotes[0]->gforce, orientation );

                        // Look for significant movement 
                        detect_movement(acceleration, orientation, index);
                                                    
                        // Update last event readings
                        last_acceleration = acceleration;
                        last_orientation  = orientation;

                        // Save this acceleration reading in the array and update index
                        recorded_accelerations[index][0] = acceleration.x;
                        recorded_accelerations[index][1] = acceleration.y;
                        recorded_accelerations[index][2] = acceleration.z;
                        index++;
                        if(index > MAX_RECORDINGS) index = 0; // Overwrite older recordings

                        // Debug
                        printf("\e[1;1H\e[2J");
                    
                        printf("\n[WIIMOTE]\n[Acceleration] [gforces]\n X: %.2f\n Y: %.2f\n Z: %.2f\n",last_acceleration.x,last_acceleration.y,last_acceleration.z);
                        printf("\n[Orientation] [degrees]\n Roll: %.2f\n Pitch: %.2f\n Abs. Roll: %.2f\n Abs. Pitch: %.2f\n",last_orientation.roll,last_orientation.pitch,last_orientation.a_roll, last_orientation.a_pitch);
                        printf("\n[Movement] [0 - Left, 1 - Right, 2 - Down, 3 - None]\n %d\n",detected_last_not_updated);

						break;
						
					// In case of a disconnect (Power down, low battery)
					case WIIUSE_DISCONNECT:
                        exit = 1;
						break;
					case WIIUSE_STATUS:
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
