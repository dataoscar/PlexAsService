using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using System.IO;

namespace PlexSvcConfig
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private static string PLEX_DEFAULT_INSTALL = "C:\\Program Files (x86)\\Plex\\Plex Media Server";
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {

        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.CheckFileExists = true;
            dlg.CheckPathExists = true;
            dlg.DefaultExt = "Plex Media Server.exe";
            dlg.Filter = "Plex|Plex Media Server.exe|All|*.*";
            if (CheckPlexExists())
            {
                dlg.InitialDirectory = PLEX_DEFAULT_INSTALL;
            }
            else
            {
                dlg.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
            }
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                // Open document
                string filename = dlg.FileName;
                plexLocTxt.Text = filename;
            }
        }

        bool CheckPlexExists()
        {
            return Directory.Exists(PLEX_DEFAULT_INSTALL);
        }

        private void saveBttn_Click(object sender, RoutedEventArgs e)
        {
            if (plexLocTxt.Text == null || plexLocTxt.Text == "")
            {
                MessageBox.Show("Please browse for the Plex Media Server location", "Error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return;
            }

            RegistryKey rk = Registry.LocalMachine;
            rk = rk.CreateSubKey(@"Software\dataoscar\PlexSvcLoader\");
            rk.SetValue("PlexExeLocation", plexLocTxt.Text);
            rk.Close();
        }
    }
}
