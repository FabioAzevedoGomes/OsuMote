using System.Runtime.InteropServices;
using System.Threading;
using Gtk;

namespace TRAB_IHC
{

    class MainClass
    {
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void InitLibrary();
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void ControllerManager();
        [DllImport("libcontrollermanager.so", CallingConvention = CallingConvention.StdCall)] public static extern void Finish();

        public static void Main(string[] args)
        {
            // Initialize the wiiC library wrapper
            InitLibrary();

            // Spawn the controller manager thread
            ThreadStart work = ControllerManager;
            Thread wm_manager = new Thread(work);
            wm_manager.Start();

            // Start the GUI
            Application.Init();
            MainWindow win = new MainWindow();
            win.Show();
            Application.Run();

            // Finalize library
            Finish();
                
        }
    }
}
