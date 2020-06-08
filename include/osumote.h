#include <gtkmm.h>
#include <wiicpp.h>
#include "constants.h"

class OsuMote: public Gtk::Window
{

    private:    
        CWii* wii;
        int* connected;

    public:
        OsuMote(CWii* main_wii, int* connected_count);
        virtual ~OsuMote();
        
    protected:
        // Signal handler
        void on_button_clicked(Glib::ustring data);
        void HandleConnect();

        // Child Widgets
        Gtk::Box box;
        Gtk::Button button;
               
};
