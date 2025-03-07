using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace WpfApp1
{
    /// <summary>
    /// Logica di interazione per MainWindow.xaml
    /// </summary>
    /// 

    public partial class MainWindow : Window
    {
        private Dispatcher dispatcher;
        private string fileName = "Settings.ini";

        private string log;
        public string Log
        {
            get { return this.log; }
            set
            {
                this.log = value;
                this.displayToWindow(this.Log);
            }
        }

        private void displayToWindow(string inData)
        {
            this.dispatcher.BeginInvoke(new Action(() => this.UpdateLOG(inData)));
        }

        void UpdateLOG(string inData)
        {
            this.TextLog.Text = inData;
            this.TextLog.Focus();
            this.TextLog.SelectionStart = this.TextLog.Text.Length;
        }

        public MainWindow()
        {
            this.InitializeComponent();
            this.dispatcher = Application.Current.MainWindow.Dispatcher;

            this.ComboColor.Items.Add("NONE");
            this.ComboColor.Items.Add("RED");
            this.ComboColor.Items.Add("GREEN");
            this.ComboColor.Items.Add("BLUE");
            this.ComboColor.Items.Add("VIOLET");
            this.ComboColor.Items.Add("YELLOW");
            this.ComboColor.Items.Add("CYAN");
            this.ComboColor.Items.Add("WHITE");
            this.ComboColor.Items.Add("RANDOM");


            this.ComboBlinkRGB.Items.Add("No blinking");
            this.ComboBlinkRGB.Items.Add("Blink 500ms");
            this.ComboBlinkRGB.Items.Add("Blink 1s");
            this.ComboBlinkRGB.Items.Add("Blink 184ms");
            this.ComboBlinkRGB.Items.Add("Blink 75ms");

            if (File.Exists(this.fileName))
            {
                var lines = File.ReadAllLines(this.fileName);
                this.PortName.Text = lines[0];
            }

        }


        private void Button_Click_8(object sender, RoutedEventArgs e)
        {
            byte value = (byte)this.ComboColor.SelectedIndex;
            this.Send_TO_HC(new byte[] { value });
        }


        private void Button_Click_9(object sender, RoutedEventArgs e)
        {
            this.Send_TO_HC(new byte[] { (byte)(this.ComboBlinkRGB.SelectedIndex + 0x0A) });
        }



        SerialPort p1 = new SerialPort();


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.p1.BaudRate = 9600;
                this.p1.PortName = this.PortName.Text;
                this.p1.DataReceived += this.P1_DataReceived;
                this.p1.Open();
                File.WriteAllLines(this.fileName, new string[] { this.PortName.Text });
                this.Log += "\r\n CONNECTED! \r\n";
            }
            catch (Exception EE)
            { MessageBox.Show(EE.ToString()); }

        }

        byte lenReturn = 0;
        private void P1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort spL = (SerialPort)sender;
            byte[] buf = new byte[spL.BytesToRead];
            spL.Read(buf, 0, buf.Length);
            string temp = "";
            foreach (byte b in buf)
            {
                if (this.lenReturn == 1)
                {
                    if (this.listByte != null)
                    {
                        this.listByte.Add(b);
                        received = true;
                    }
                }

                if (this.lenReturn == 0)
                {
                    this.lenReturn = b;
                    this.Log += "\n";
                }
                else
                {
                    this.lenReturn--;
                    if (this.lenReturn == 0)
                    {
                        this.Log += "\n";
                    }

                }
                temp += (b.ToString() + " ");
            }
            this.Log += temp;
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            if (this.p1.IsOpen)
            {
                this.p1.Close();
            }

            this.Log += "\r\n CLOSE \r\n";
        }

        //SEND_PHRASE   0xLL 0x50 0x01 0xAA 0xBB 0xCC......
        void SendString(string phrase)
        {
            char[] bytes = phrase.ToCharArray();
            List<byte> commandWithPhrase = new List<byte>();

            commandWithPhrase.Add(0x50);
            commandWithPhrase.Add(0x01);

            foreach (var bb in bytes)
            {
                commandWithPhrase.Add((byte)bb);
            }

            this.Send_TO_HC(commandWithPhrase.ToArray());
        }

        //READByte XX YY ZZ	    0x04	0x10	0x01	0xXX	0xYY
        //WRITEByte XX YY ZZ	0x05	0x10	0x02	0xXX	0xYY	0xZZ
        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            this.Send_TO_HC(new byte[] { 0x10, 0x01, (byte)(Convert.ToInt16(this.Addressbyte.Text) / 256), (byte)Convert.ToInt16(this.Addressbyte.Text) });
        }

        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            this.Send_TO_HC(new byte[] { 0x10, 0x02, (byte)(Convert.ToInt16(this.Addressbyte.Text) / 256), (byte)Convert.ToInt16(this.Addressbyte.Text), (byte)Convert.ToInt16(this.Valuebyte.Text) });
        }

        private void Button_Click_SendText(object sender, RoutedEventArgs e)
        {
            var frase = this.SendText.Text;
            List<byte> arraybytes = new List<byte>();
            arraybytes.AddRange(Encoding.ASCII.GetBytes(frase));
            arraybytes.Insert(0, 0x50);
            arraybytes.Insert(1, 0x01);
            this.Send_TO_HC(arraybytes.ToArray());
        }


        void Send_TO_HC(byte[] request)
        {
            if (this.p1.IsOpen)
            {
                this.p1.Write(new byte[] { (byte)request.Length }, 0, 1);
                System.Threading.Thread.Sleep(1);

                foreach (byte bb in request)
                {
                    this.p1.Write(new byte[] { bb }, 0, 1);
                    System.Threading.Thread.Sleep(1);
                }
            }
            else
            {
                this.Log += "\r\n PORT IS CLOSE! \r\n";
            }
        }


        void WriteEEPROM(string Filename, bool IsText)
        {
            this.Send_TO_HC(new byte[] { 0x10, 0x03 });  //STOP COMMUNICATION
            System.Threading.Thread.Sleep(200);
            List<byte> bytes = new List<byte>();
            if (IsText)
            {
                var strByte = File.ReadAllText(Filename).Replace("\r", "").Replace("\n", "\t").Replace(" ", "").Trim();//tab separation (copy from Excel)
                var Bytes = strByte.Split('\t');

                for (int i = 0; i < Bytes.Length; i++)
                {
                    bytes.Add(Convert.ToByte(Bytes[i], 16));
                }
            }
            else
            {
                bytes.AddRange(File.ReadAllBytes(Filename));
            }



            File.WriteAllBytes(Filename + ".bin", bytes.ToArray());

            float Percentage = 0;
            for (int i = 0; i < bytes.Count; i++)
            {
                Percentage = (float)i / (float)bytes.Count;
                Percentage *= 100;
                this.Log = $"\r\nWRITE: Index: {((byte)(i / 256)).ToString("X2")} {((byte)i).ToString("X2")} Value: {bytes[i].ToString("X2")} - ({Percentage.ToString("F01")}%)\r\nREPONSE: ";
                this.Send_TO_HC(new byte[] { 0x10, 0x02, (byte)(i / 256), (byte)i, bytes[i] });

                System.Threading.Thread.Sleep(200);
            }


            this.Send_TO_HC(new byte[] { 0x10, 0x04 });  //RESTART COMMUNICATION
            System.Threading.Thread.Sleep(200);
            // this.EEPROM.Dispose();
        }

        List<byte> listByte;
        bool received = false;
        void ReadEEPROM(string Filename)
        {
            this.Send_TO_HC(new byte[] { 0x10, 0x03 });  //STOP COMMUNICATION
            System.Threading.Thread.Sleep(200);
            //int len = 32768;//32K
            int len = 1024;//1K
            int volte;
            if (File.Exists(Filename))
            {
                File.Delete(Filename);
            }

            float Percentage = 0;
            this.listByte = new List<byte>();
            bool scritta = false;
            for (int i = 0; i < len; i++)
            {

                Percentage = (float)i / (float)len;
                Percentage *= 100;
                /*if ((int)Percentage % 2 == 0)
                {
                    if (scritta == false)
                    {*/
                this.Log = $"{Percentage.ToString("F01")}%";
                /*        scritta = true;
                    }
                }
                else
                {
                    scritta = false;
                }*/
                received = false;
                this.Send_TO_HC(new byte[] { 0x10, 0x01, (byte)(i / 256), (byte)i });
                volte = 10;
                while (volte != 0 && !received)
                {
                    volte--;
                    System.Threading.Thread.Sleep(1);
                }
            }

            File.WriteAllBytes(Filename, this.listByte.ToArray());
            this.Log += $"\r\nFILE: {Filename} OK!";
            this.listByte = null;
            this.Send_TO_HC(new byte[] { 0x10, 0x04 });  //RESTART COMMUNICATION
            System.Threading.Thread.Sleep(200);
            // this.EEPROM.Dispose();
        }


        Task EEPROM;
        private void Button_Click_4(object sender, RoutedEventArgs e)
        {
            System.Windows.Controls.Button senderButton = (System.Windows.Controls.Button)sender;
            Microsoft.Win32.OpenFileDialog openFileDlg = new Microsoft.Win32.OpenFileDialog();
            openFileDlg.Filter = $"Documents (.{senderButton.Tag})|*.{senderButton.Tag}";
            Nullable<bool> result = openFileDlg.ShowDialog();
            if (result == true)
            {
                switch (senderButton.Tag)
                {
                    case "txt":
                        this.EEPROM = new Task(() => this.WriteEEPROM(openFileDlg.FileName, true));
                        this.EEPROM.Start();
                        break;
                    case "bin":
                        this.EEPROM = new Task(() => this.WriteEEPROM(openFileDlg.FileName, false));
                        this.EEPROM.Start();
                        break;
                    default:
                        this.Log += "\r\nButton TAG ERROR!";
                        break;
                }
            }
        }

        private void Button_Click_5(object sender, RoutedEventArgs e)
        {
            this.EEPROM = new Task(() => this.ReadEEPROM("EEPROM_READ.bin"));
            this.EEPROM.Start();
        }
    }
}
