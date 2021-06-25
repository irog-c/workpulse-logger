using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;
using static System.Net.Mime.MediaTypeNames;

namespace FileWatcher
{
    class Watcher
    {
        private const string fileToWatch = "C:\\log-app.txt";
        private const string dirToWatch = "C:";
        private const string logFile = "C:\\log-open-log-file.txt";

        static FileSystemWatcher watcher;

        private static string getCurrentUserName()
        {
            return System.Security.Principal.WindowsIdentity.GetCurrent().Name;
        }
        
        private static void LogEvent(String ev)
        {
            string user = getCurrentUserName();
            File.AppendAllText(logFile, user + ": " + ev + "\n");
        }

        private static void WatcherEventCallback(object source, FileSystemEventArgs e)
        {
            Console.WriteLine(e.FullPath);
            if(e.FullPath == fileToWatch)
                LogEvent("File " + e.Name + " has been opened.");
        }

        private static void InitWatcher()
        {
            watcher = new FileSystemWatcher();

            watcher.Path = dirToWatch;
            watcher.NotifyFilter = NotifyFilters.LastAccess;
            watcher.Filter = Path.GetFileName(fileToWatch);

            watcher.Changed += WatcherEventCallback;
            watcher.EnableRaisingEvents = true;
        }

        [STAThread]
        static void Main(string[] args)
        {
            InitWatcher();
            System.Windows.Forms.Application.Run(new ApplicationContext());
        }
    }
}
