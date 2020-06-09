#include <gtkmm.h>
#include <thread>
#include <windows.h>
#include "osumote.h"
#include "controllermanager.h"
#include "include/virtualKeyboardManager"

/*
virtual Keyboard example

	int virtualKeyboardExampel()
	{
		// This structure will be used to create the keyboard
		// input event.
		INPUT ip{};

		// Create the generic keyboard to use as our virtual keyboard
		ip = createGenericKeyboard(ip);

		// call the letters D,F,J and K
		keystroke(ip, KEYBOARD_D);
		keystroke(ip, KEYBOARD_F);
		keystroke(ip, KEYBOARD_J);
		keystroke(ip, KEYBOARD_K);


		// you can use Slepp function to delay the callback
		Sleep(5000); // 5 seconds
		// and then call the keystroke
		keystroke(ip, KEYBOARD_K);
		// Exit normally
		return 0;
	}

*/

using namespace std;

int main(int argc, char **argv)
{
	INPUT ip{};
	Sleep(5000);
	keystroke(ip, KEYBOARD_D);

	// Initialize wiiuse library
	CWii wii;
	int connected_wiimotes = 0;

	// Spawn thread for handling controller events
	std::thread wiimote_manager(&ControllerManager::spawn, ControllerManager(&wii, &connected_wiimotes));
	
	// Initialize application window
	auto app = Gtk::Application::create(argc, argv, "org.gtkm.example");

	// Run application
	OsuMote osumote(&wii, &connected_wiimotes);
	int status = app->run(osumote);
	
	// Signal controller manager thread to exit
	//TODO Mutex connected_wiimotes
	connected_wiimotes = EXIT_SIGNAL;
	
	// Wait for controller manager to finish
	wiimote_manager.join();
	
	// End application
	return status;
}

