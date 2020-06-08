#include <wiicpp.h>
#include "constants.h"

typedef enum {LEFT, RIGHT, UP, DOWN, NONE} directions;
class ControllerManager
{

    private:
        std::string DIRECTION_MAP[5] = {"LEFT","RIGHT","UP","DOWN","NONE"}; // Moves detected using the gyroscope
        CWii* wii; // Pointer to wii structure that contains wiimotes
        int* connected; // Pointer to amount of wiimotes that are connected
        double recorded_rates[MAX_RECORDINGS][3]; // Last rates recorded (pitch, roll, yaw)
        int index = 0; // Last place where a rate trio was stored
        directions last_read, last_direction; // Last detected reading and actual last detected direction
                                              // Difference is last_direction won't be NONE

    public:
        ControllerManager(CWii* main_wii, int* connected_count);
        virtual ~ControllerManager();     
        int spawn();
        directions detect_movement(int is_pressed_B);
        void HandleEvent(CWiimote &wm);
        void HandleDisconnect(CWiimote &wm);
        void SendToGame(directions direction, int double_input);
};
