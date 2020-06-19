using System.Runtime.InteropServices;
using System.Threading;
using Gtk;

namespace TRAB_IHC
{

    class MainClass
    {
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void init();
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void controller_manager();
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void set_connected_wiimotes(int n);

        public static void Main(string[] args)
        {
            // Initialize the wiiC library wrapper
            init();
            
            // Spawn the controller manager thread
            ThreadStart work = controller_manager;
            Thread wm_manager = new Thread(work);
            wm_manager.Start();

            // Start the GUI
            Application.Init();
            MainWindow win = new MainWindow();
            win.Show();
            Application.Run();

            // Signal the controller manager thread to end
            set_connected_wiimotes(-9);
                
        }
    }
}
