#include "osumote.h"
#include "controllermanager.h"

OsuMote::OsuMote(CWii* main_wii, int* connected_count) : button("Connect")
{
    
    connected = connected_count;

    // Save reference to the wiimote pointer list
    wii = main_wii;

    set_title("OsuMote");
    set_border_width(10);
    
    add(box);
    
    // Bind connect function to on click handler
    button.signal_clicked().connect(
        sigc::bind<Glib::ustring>(
            sigc::mem_fun(*this, &OsuMote::on_button_clicked), "button")
    );    

    box.pack_start(button);
    button.show();
    
    box.show();
}

OsuMote::~OsuMote(){}
      

void OsuMote::HandleConnect()
{
    int index = 0;
    std::vector<CWiimote>::iterator i;

    // LED Map for the wiimote
    int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4};

    // Try connection to wiimotes
    std::vector<CWiimote>& wiimotes = (*wii).FindAndConnect();
    if (!wiimotes.size())
    {
        // If no wiimotes were connected
        Gtk::MessageDialog errorDialog(*this, "Could not find any remotes");
        errorDialog.set_secondary_text("Is your bluetooth adapter enabled?");
        errorDialog.run();
    }
    else
    {
        // For every found and connected wiimote
        for (index = 0, i = wiimotes.begin(); i != wiimotes.end(); ++i , ++index)
        {
            // Set leds
            CWiimote & wiimote = *i;
            wiimote.SetLEDs(LED_MAP[index]);

            // Instruct user to leave the controller still for gyroscope calibration
            Gtk::MessageDialog warnCalibration(*this, "Gyroscope calibration");
            warnCalibration.set_secondary_text("Gyroscope will now be calibrated.\nPlease leave the controller still face-up on a flat surface and press OK");
            warnCalibration.run();
            
            // Enable motion sensing and motion plus
            wiimote.SetMotionSensingMode(CWiimote::ON);
            wiimote.EnableMotionPlus(CWiimote::ON);

        }
        
        // Update connected count to tell controllermanager to start handling events
        *connected = wiimotes.size();

        // Signal sucessful connection to user
        Gtk::MessageDialog confirmDialog(*this, "Wiimote Connected!");
        char msg[256]={0};
        g_snprintf(msg, sizeof msg,"%d Wiimotes were sucessfully connected!\nWhen you want to disconnect the wiimotes simply close the application or hold the power button on your wiimote", *connected);
        confirmDialog.set_secondary_text(msg);
        confirmDialog.run();
        
    }

}
void OsuMote::on_button_clicked(Glib::ustring data)
{

    // Show instructions to user
    Gtk::MessageDialog connectDialog(*this, "Connection", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    connectDialog.set_secondary_text("Please hold buttons 1 and 2 in your controller and press OK.\nHold until LED 1 is lit up");

    switch (connectDialog.run())
    {
        // When user responds attempt to connect wiimote            
        case Gtk::RESPONSE_OK: 
            OsuMote::HandleConnect();
            break;
        case Gtk::RESPONSE_CANCEL:
        default:
            break;
    }

}

