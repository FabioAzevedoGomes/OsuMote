#include <wiicpp.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

namespace ControllerManager
{
    // Constants
    #define MAX_RECORDINGS 12
    #define MAX_CALIBRATION_RECORDINGS 128

    // Error codes and signals
    #define ERROR_BLUETOOTH_OFF -1
    #define ERROR_NO_WIIMOTES -2
    #define ERROR_FILE_NOT_FOUND -3
    #define EXIT_SIGNAL -9

    // Directions enum
    typedef enum {LEFT, RIGHT, UP, DOWN, NONE, DOUBLE_SIDE, DOUBLE_DOWN} directions;

    // Status flags in the application
    struct status_t
    {
        bool calibrating; // Whether calibration is currently ongoing or not
        bool vibration;   // Whether vibration is enabled or not
        bool sound;       // Whether sound is enabled or not
    };

    // References and values used by the application to connect / detect motion
    struct refs_t
    {
        CWii wii;    // Wii struct used by the application to manage wiimotes
        int connected_wiimotes; // Current connected wiimote count
        double recorded_rates[MAX_RECORDINGS][3]; // Angle rates recorded for motion detection
        int position_index; // Current index in recorded_rates
        directions last_read, last_direction; // Last assumed movement and last detected (sent) movement
        directions last_sent; // Last command sent to the game
        char* center_sound; // Path to sound file used in center hit
        char* sides_sound; // Path to sound file used in side hit
    };

    // Variables relating to the calibration of the wiimote force
    struct calibration_t
    {
        int    calibration_index;
        int    calibration_turn;
        int    calibration_axis;
        double calibration_angles[MAX_CALIBRATION_RECORDINGS][3];

        double angle_rate_threshold_down;
        double angle_rate_threshold_sides;
    };
    
    // Initialize library
    extern "C" void InitLibrary();

    // Signals the controller manager thread to end
    extern "C" void Finish();

    /*
    *   Connects to the user's wiimote
    *   Returns:
    *    Positive integer => Number of connected wiimotes
    *    Negative or 0    => Error connecting / None connected
    */
    extern "C" int ConnectWiimotes();

    /*
    *   Disconnects the user's wiimote
    *   Returns:
    *    Positive or Negative => Error disconnecting wiimotes
    *    0                    => Wiimote disconnected sucessfuly
    */
    extern "C" int DisconnectWiimotes();
    
    /* 
    *   Enable motion sensing on wiimote
    *   Returns:
    *    0 => Motion sensing enabled   
    */
    extern "C" int EnableMotionSensing();

    /*
    *   Main loop for the controller manager, should be spawned in a separate thread at the start of the application
    */
    extern "C" void ControllerManager();

    /*
    *   Records the user's movements 3 times, and sets the average
    *   max angle change rate detected as the threshold for that axis
    *   Receives:
    *    Axis 0 => DOWN
    *    Axis 1 => SIDES
    */
    extern "C" void CalibrateForce(int axis, int turn);

    // Toggles vibration setting on controller hit
    extern "C" void ToggleVibration();
    
    /*
    *   Returns last movement that was sent to the game
    *   Returns:
    *    0 => NONE
    *    1 => LEFT
    *    2 => RIGHT
    *    3 => DOWN
    *    4 => DOUBLE DOWN
    *    5 => DOUBLE SIDES
    */
    extern "C" int  GetLastReadMovement();
}
