#include <wiicpp.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

namespace ControllerManager{

    // Constants
    #define MAX_RECORDINGS 12
    #define MAX_CALIBRATION_RECORDINGS 128
    //#define ANGLE_RATE_THRESHOLD_DOWN 900 // Angle rate limit for pitch to be detected as down
    //#define ANGLE_RATE_THRESHOLD_SIDES 500 // Angle rate limit for yaw to be detected as sides
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

    /*
    *   Records the user's movements 3 times, and sets the average
    *   max angle change rate detected as the threshold for that axis
    *   Receives:
    *    Axis 0 => DOWN
    *    Axis 1 => SIDES
    */
    extern "C" void calibrate_force(int axis, int turn);

    // Toggles vibration setting on controller hit
    extern "C" void toggle_vibration();

    // Toggles sound playing on controller hit
    extern "C" void toggle_sound();

    /*
    *   Sets the path to the sound file played on controller hit
    *   Receives:
    *    filename => Path to the file containing the sound to be played
    *    type     => Which hit to set this sound on (0: Center, 1: Sides)
    *   Returns:
    *    0        => File was found and sound played sucessfully
    *    Negative => File not found / Could not set sound
    */
    extern "C" int set_sound(char* filename, int type);

    /*
    *   Plays the current set sound for provided type through the wiimote speaker
    *   Receives:
    *    type => Which hit to play this from (0: Center, 1: Sides)
    */
    extern "C" void play_sound(int type);
}
