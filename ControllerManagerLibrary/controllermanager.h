#include <wiicpp.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

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
        char* center_sound, sides_sound; // Path to sound files used in center and side sounds
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

    // Toggles sound playing on controller hit
    extern "C" void ToggleSound();

    /*
    *   Sets the path to the sound file played on controller hit
    *   Receives:
    *    filename => Path to the file containing the sound to be played
    *    type     => Which hit to set this sound on (0: Center, 1: Sides)
    *   Returns:
    *    0        => File was found and sound played sucessfully
    *    Negative => File not found / Could not set sound
    */
    extern "C" int SetSound(char* filename, int type);

    /*
    *   Plays the current set sound for provided type through the wiimote speaker
    *   Receives:
    *    type => Which hit to play this from (0: Center, 1: Sides)
    */
    extern "C" void PlaySound(int type);
}
