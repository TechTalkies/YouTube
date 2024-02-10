using OpenHardwareMonitor.Hardware;
using System;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Timers;
using System.Windows.Forms;

namespace TechTalkies_HWM
{
    public partial class Form1 : Form
    {
        private ContextMenu contextMenu1;
        private MenuItem menuItem1;

        bool isConnected = false;
        bool switchHardware = false;
        String[] ports;
        SerialPort port;
        String displayText;
        System.Timers.Timer aTimer;

        public Form1()
        {
            this.contextMenu1 = new ContextMenu();
            this.menuItem1 = new MenuItem();
            InitializeComponent();


            // Initialize contextMenu1
            this.contextMenu1.MenuItems.AddRange(
                        new MenuItem[] { this.menuItem1 });

            // Initialize menuItem1
            this.menuItem1.Index = 0;
            this.menuItem1.Text = "E&xit";
            this.menuItem1.Click += new EventHandler(this.menuItem1_Click);

            notifyIcon1.ContextMenu = this.contextMenu1;

            // Handle the DoubleClick event to activate the form.
            notifyIcon1.DoubleClick += new EventHandler(this.notifyIcon1_DoubleClick);

            getAvailableComPorts();

            foreach (string port in ports)
            {
                comboBox1.Items.Add(port);
                //Console.WriteLine(port);
                if (ports[0] != null)
                {
                    comboBox1.SelectedItem = ports[0];
                }
            }
            displayText = "CPU00";
        }

        private void SetTimer()
        {
            // Create a timer with a two second interval.
            aTimer = new System.Timers.Timer(2500);
            // Hook up the Elapsed event for the timer. 
            aTimer.Elapsed += OnTimedEvent;
            aTimer.AutoReset = true;
            aTimer.Enabled = true;
        }

        private void OnTimedEvent(Object source, ElapsedEventArgs e)
        {
            GetSystemInfo();
        }

        public class UpdateVisitor : IVisitor
        {
            public void VisitComputer(IComputer computer)
            {
                computer.Traverse(this);
            }
            public void VisitHardware(IHardware hardware)
            {
                hardware.Update();
                foreach (IHardware subHardware in hardware.SubHardware) subHardware.Accept(this);
            }
            public void VisitSensor(ISensor sensor) { }
            public void VisitParameter(IParameter parameter) { }
        }

        void GetSystemInfo()
        {
            UpdateVisitor updateVisitor = new UpdateVisitor();
            Computer computer = new Computer();
            computer.Open();
            computer.CPUEnabled = true;
            computer.GPUEnabled = true;
            computer.Accept(updateVisitor);
            Console.WriteLine("Get System info");

            for (int i = 0; i < computer.Hardware.Length; i++)
            {
                if (switchHardware)
                {
                    if (computer.Hardware[i].HardwareType == HardwareType.CPU)
                    {
                        for (int j = 0; j < computer.Hardware[i].Sensors.Length; j++)
                        {
                            if (computer.Hardware[i].Sensors[j].SensorType == SensorType.Temperature)
                            {
                                float? temperature = computer.Hardware[i].Sensors[j].Value;
                                displayText = "";
                                displayText = "CPU:" + Math.Round(temperature.Value).ToString() + "~c";
                                Console.WriteLine(displayText + "\r");
                                if (!(port == null) && port.IsOpen)
                                {
                                    port.Write(displayText);
                                    textBox1.Clear();
                                    textBox1.Text = displayText;
                                    textBox1.AppendText(displayText);
                                }
                                switchHardware = !switchHardware;
                                break;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if (computer.Hardware[i].HardwareType == HardwareType.GpuNvidia)
                    {
                        for (int j = 0; j < computer.Hardware[i].Sensors.Length; j++)
                        {
                            if (computer.Hardware[i].Sensors[j].SensorType == SensorType.Temperature)
                            {
                                float? temperature = computer.Hardware[i].Sensors[j].Value;
                                displayText = "";
                                displayText = displayText + "GPU:" + Math.Round(temperature.Value).ToString() + "~c";
                                Console.WriteLine(displayText + "\r");
                                if (!(port == null) && port.IsOpen)
                                {
                                    port.Write(displayText);
                                    textBox1.Clear();
                                    textBox1.Text = displayText;
                                    textBox1.AppendText(displayText);
                                }
                                switchHardware = !switchHardware;
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            Console.WriteLine("End of function");
            Console.WriteLine(switchHardware.ToString());
            computer.Close();
        }
        void getAvailableComPorts()
        {
            ports = SerialPort.GetPortNames();
            
            using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption like '%(COM%'"))
            {
                var ports2 = searcher.Get().Cast<ManagementBaseObject>().ToList().Select(p => p["Caption"].ToString());

                var portList = ports.Select(n => n + " - " + ports2.FirstOrDefault(s => s.Contains(n))).ToList();

                foreach (string s in portList)
                {
                    //Console.WriteLine(s);
                    textBox2.AppendText(s + Environment.NewLine);
                }
            }
            
        }

        private void connectToArduino()
        {
            isConnected = true;
            string selectedPort = comboBox1.GetItemText(comboBox1.SelectedItem);
            port = new SerialPort(selectedPort, 9600, Parity.None, 8, StopBits.One);
            try
            {
                port.Open();
            }catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
                textBox2.AppendText(ex.Message + Environment.NewLine);
            }

            //port.Write(displayText);
            button1.Text = "Disconnect";
            SetTimer();
            Console.WriteLine("Connect to Serial");
        }

        private void disconnectFromArduino()
        {
            isConnected = false;
            port.Write("CPU00");
            //InvalidOperationException
            try
            {
                port.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                textBox2.AppendText(ex.Message + Environment.NewLine);
            }
            
            button1.Text = "Connect";
            if (port.IsOpen)
                Console.WriteLine("Error closing port");
            else Console.WriteLine("Disconnected\n");
        }


        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!isConnected)
            {
                connectToArduino();
            }
            else
            {
                disconnectFromArduino();
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                notifyIcon1.Visible = true;
                this.Hide();
                e.Cancel = true;
            }
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            Show();
            this.WindowState = FormWindowState.Normal;
            notifyIcon1.Visible = false;
        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            /* Brightness slider not implemented for this Arduino Project yet
            if (!(port == null) && port.IsOpen)
            {
                port.Write("b"+trackBar1.Value);
            }
            */
        }

        private void notifyIcon1_DoubleClick(object Sender, EventArgs e)
        {
            // Show the form when the user double clicks on the notify icon.

            // Set the WindowState to normal if the form is minimized.
            if (this.WindowState == FormWindowState.Minimized)
                this.WindowState = FormWindowState.Normal;

            // Activate the form.
            this.Activate();
        }

        private void menuItem1_Click(object Sender, EventArgs e)
        {
            // Close the form, which closes the application.
            if (isConnected)
            {
                disconnectFromArduino();
            }
            this.Close();
            Application.Exit();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // Close the form, which closes the application.
            if (isConnected)
            {
                disconnectFromArduino();
            }
            this.Close();
            Application.Exit();
        }
    }
}
