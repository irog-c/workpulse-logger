using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

namespace LogService
{
    public partial class Log : ServiceBase
    {
        private static string fileWatcher = "F:\\VisualStudioRepos\\FileWatcher\\FileWatcher\\FileWatcher\\bin\\Debug\\FileWatcher.exe";
        private static string loggerApplication = "F:\\VisualStudioRepos\\TestProject\\TestProject\\Debug\\TestProject.exe";

        public Log()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {
            ProcessExtensions.StartProcessAsCurrentUser(fileWatcher);
            ProcessExtensions.StartProcessAsCurrentUser(loggerApplication);
        }

        private static void KillAll(string path)
        {
            string name = Path.GetFileNameWithoutExtension(path);
            Process[] workers = Process.GetProcessesByName(name);
            foreach (Process worker in workers)
            {
                worker.Kill();
                worker.WaitForExit();
                worker.Dispose();
            }
        }

        protected override void OnStop()
        {
            KillAll(fileWatcher);
            KillAll(loggerApplication);
        }
    }
}
