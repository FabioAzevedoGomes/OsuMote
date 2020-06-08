#define EXIT_SIGNAL -9      // Signal for wiimote event handler thread to finish

#define MAX_WIIMOTES 1      // Max amount of wiimotes that can be connected to the application

#define MAX_SEARCH_TIME 10  // Max time (seconds) for the wiimote search in wiiuse_find
                            // Subject to change, maybe let user decide ?

#define ANGLE_RATE_THRESHOLD_DOWN 1000 // Angle rate limit for pitch to be detected as down

#define ANGLE_RATE_THRESHOLD_SIDES 500 // Angle rate limit for yaw to be detected as sides

#define MAX_RECORDINGS 12 // Max number of angle rate changes to be saved