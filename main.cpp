#include <gtkmm.h>
#include <thread>
#include "osumote.h"
#include "controllermanager.h"

using namespace std;

int main(int argc, char **argv)
{

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

