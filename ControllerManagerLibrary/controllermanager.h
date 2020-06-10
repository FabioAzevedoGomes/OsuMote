#include <wiicpp.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

namespace ControllerManager{

    // Constants
    #define MAX_RECORDINGS 12
    #define ANGLE_RATE_THRESHOLD_DOWN 900 // Angle rate limit for pitch to be detected as down
    #define ANGLE_RATE_THRESHOLD_SIDES 500 // Angle rate limit for yaw to be detected as sides
    // Error codes and signals
    #define ERROR_BLUETOOTH_OFF -1
    #define ERROR_NO_WIIMOTES -2
    #define EXIT_SIGNAL -9


    // Directions enum
    typedef enum {LEFT, RIGHT, UP, DOWN, NONE} directions;

    // Initialize library
    extern "C" void init();

    // Set connected wiimote count
    extern "C" void set_connected_wiimotes(int n);

    /*
    *   Connects to the user's wiimote
    *   Returns:
    *    Positive integer => Number of connected wiimotes
    *    Negative or 0    => Error connecting / None connected
    */
    extern "C" int connect_wiimotes();


    /*
    *   Disconnects the user's wiimote
    *   Returns:
    *    Positive or Negative => Error disconnecting wiimotes
    *    0                    => Wiimote disconnected sucessfuly
    */
    extern "C" int disconnect_wiimotes();
    
    /* 
    *   Enable motion sensing on wiimote
    *   Returns:
    *    0 => Motion sensing enabled   
    */
    extern "C" int enable_motion_sensing();

    /*
    *   Main loop for the controller manager, should be spawned in a separate thread at the start of the application
    */
    extern "C" void controller_manager();
}
