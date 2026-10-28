using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading;
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
        private const int EepromCapacity = 32768;
        private const int CommandTimeoutMilliseconds = 5000;

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
        private readonly object responseLock = new object();
        private readonly Queue<byte[]> responseFrames = new Queue<byte[]>();
        private readonly List<byte> incomingFrame = new List<byte>();
        private readonly AutoResetEvent responseAvailable = new AutoResetEvent(false);
        private int expectedIncomingFrameLength;
        private volatile bool eepromOperationInProgress;

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.p1.BaudRate = 9600;
                this.p1.PortName = this.PortName.Text;
                this.p1.ReadTimeout = CommandTimeoutMilliseconds;
                this.p1.WriteTimeout = CommandTimeoutMilliseconds;
                this.p1.DataReceived -= this.P1_DataReceived;
                this.p1.DataReceived += this.P1_DataReceived;
                this.p1.Open();
                File.WriteAllLines(this.fileName, new string[] { this.PortName.Text });
                this.Log += "\r\n CONNECTED! \r\n";
            }
            catch (Exception EE)
            { MessageBox.Show(EE.ToString()); }

        }

        private void P1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort serialPort = (SerialPort)sender;
            byte[] buffer = new byte[serialPort.BytesToRead];
            int bytesRead = serialPort.Read(buffer, 0, buffer.Length);
            StringBuilder receivedText = new StringBuilder();

            lock (this.responseLock)
            {
                for (int i = 0; i < bytesRead; i++)
                {
                    this.AddReceivedByte(buffer[i]);
                    receivedText.Append(buffer[i].ToString("X2"));
                    receivedText.Append(' ');
                }
            }

            if (bytesRead > 0 && !this.eepromOperationInProgress)
            {
                this.Log += "\r\nRX: " + receivedText;
            }
        }

        private void AddReceivedByte(byte value)
        {
            if (this.incomingFrame.Count == 0)
            {
                this.expectedIncomingFrameLength = value + 1;
                if (this.expectedIncomingFrameLength < 2 ||
                    this.expectedIncomingFrameLength > 50)
                {
                    this.expectedIncomingFrameLength = 0;
                    return;
                }

                this.incomingFrame.Add(value);
                return;
            }

            this.incomingFrame.Add(value);
            if (this.incomingFrame.Count == this.expectedIncomingFrameLength)
            {
                this.responseFrames.Enqueue(this.incomingFrame.ToArray());
                this.incomingFrame.Clear();
                this.expectedIncomingFrameLength = 0;
                this.responseAvailable.Set();
            }
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
            try
            {
                int address = this.ParseNumber(this.Addressbyte.Text, EepromCapacity - 1);
                this.Send_TO_HC(new byte[] { 0x10, 0x01, (byte)(address >> 8), (byte)address });
            }
            catch (FormatException error)
            {
                MessageBox.Show(error.Message, "Invalid EEPROM address", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
        }

        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            try
            {
                int address = this.ParseNumber(this.Addressbyte.Text, EepromCapacity - 1);
                int value = this.ParseNumber(this.Valuebyte.Text, byte.MaxValue);
                this.Send_TO_HC(new byte[] { 0x10, 0x02, (byte)(address >> 8), (byte)address, (byte)value });
            }
            catch (FormatException error)
            {
                MessageBox.Show(error.Message, "Invalid EEPROM value", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
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


        private int ParseNumber(string text, int maximum)
        {
            int value;
            if (!int.TryParse(text, out value) || value < 0 || value > maximum)
            {
                throw new FormatException("Enter a decimal value from 0 to " + maximum + ".");
            }

            return value;
        }

        private bool Send_TO_HC(byte[] request)
        {
            if (!this.p1.IsOpen)
            {
                this.Log += "\r\n PORT IS CLOSE! \r\n";
                return false;
            }

            if (request == null || request.Length == 0 || request.Length >= 50)
            {
                this.Log += "\r\n INVALID COMMAND LENGTH! \r\n";
                return false;
            }

            byte[] frame = new byte[request.Length + 1];
            frame[0] = (byte)request.Length;
            Array.Copy(request, 0, frame, 1, request.Length);
            this.p1.Write(frame, 0, frame.Length);
            return true;
        }

        private byte[] WaitForResponse(int expectedPayloadLength, int timeoutMilliseconds)
        {
            DateTime deadline = DateTime.UtcNow.AddMilliseconds(timeoutMilliseconds);
            while (true)
            {
                lock (this.responseLock)
                {
                    while (this.responseFrames.Count > 0)
                    {
                        byte[] frame = this.responseFrames.Dequeue();
                        if (frame.Length == expectedPayloadLength + 1 && frame[1] == 0x0B)
                        {
                            throw new IOException("The PIC reported an EEPROM I2C error.");
                        }

                        if (frame.Length == expectedPayloadLength + 1 && frame[1] == 0x0A)
                        {
                            return frame;
                        }
                    }
                }

                int remainingMilliseconds = (int)(deadline - DateTime.UtcNow).TotalMilliseconds;
                if (remainingMilliseconds <= 0 ||
                    !this.responseAvailable.WaitOne(remainingMilliseconds))
                {
                    throw new TimeoutException("Timed out waiting for the PIC response.");
                }
            }
        }

        private void ClearPendingResponses()
        {
            lock (this.responseLock)
            {
                this.responseFrames.Clear();
                this.incomingFrame.Clear();
                this.expectedIncomingFrameLength = 0;
            }
        }

        private byte[] SendAndCheck(byte[] request, int responsePayloadLength)
        {
            if (!this.Send_TO_HC(request))
            {
                throw new IOException("The serial port is not open or the command is invalid.");
            }

            return this.WaitForResponse(responsePayloadLength, CommandTimeoutMilliseconds);
        }

        private byte ReadEEPROMByte(int address)
        {
            byte[] response = this.SendAndCheck(
                new byte[] { 0x10, 0x01, (byte)(address >> 8), (byte)address },
                2);
            return response[2];
        }

        private void WriteEEPROMByte(int address, byte value)
        {
            this.SendAndCheck(
                new byte[] { 0x10, 0x02, (byte)(address >> 8), (byte)address, value },
                1);
        }

        private void SetDisplayCommunication(bool enabled)
        {
            this.SendAndCheck(new byte[] { 0x10, enabled ? (byte)0x04 : (byte)0x03 }, 1);
        }

        private void WriteEEPROM(string filename, bool isText)
        {
            List<byte> bytes = new List<byte>();
            if (isText)
            {
                var strByte = File.ReadAllText(filename).Replace("\r", "").Replace("\n", "\t").Replace(" ", "").Trim();//tab separation (copy from Excel)
                var Bytes = strByte.Split('\t');

                for (int i = 0; i < Bytes.Length; i++)
                {
                    bytes.Add(Convert.ToByte(Bytes[i], 16));
                }
            }
            else
            {
                bytes.AddRange(File.ReadAllBytes(filename));
            }

            if (bytes.Count > EepromCapacity)
            {
                throw new IOException("The input file exceeds the 24LC256 capacity of 32768 bytes.");
            }

            this.ClearPendingResponses();
            this.SetDisplayCommunication(false);
            try
            {
                for (int i = 0; i < bytes.Count; i++)
                {
                    this.WriteEEPROMByte(i, bytes[i]);
                    if (i % 327 == 0)
                    {
                        this.Log = string.Format("WRITE: {0:F1}%", (double)(i + 1) * 100 / bytes.Count);
                    }
                }
            }
            finally
            {
                this.SetDisplayCommunication(true);
            }
        }

        private void ReadEEPROM(string filename)
        {
            byte[] contents = new byte[EepromCapacity];
            this.ClearPendingResponses();
            this.SetDisplayCommunication(false);
            try
            {
                for (int i = 0; i < contents.Length; i++)
                {
                    contents[i] = this.ReadEEPROMByte(i);
                    if (i % 327 == 0)
                    {
                        this.Log = string.Format("READ: {0:F1}%", (double)(i + 1) * 100 / contents.Length);
                    }
                }
            }
            finally
            {
                this.SetDisplayCommunication(true);
            }

            File.WriteAllBytes(filename, contents);
            this.Log += "\r\nFILE: " + filename + " OK!";
        }

        Task EEPROM;

        private void StartEEPROMTask(Action operation)
        {
            if (this.EEPROM != null && !this.EEPROM.IsCompleted)
            {
                MessageBox.Show("An EEPROM operation is already running.",
                    "EEPROM busy", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            this.EEPROM = Task.Run(() =>
            {
                this.eepromOperationInProgress = true;
                try
                {
                    operation();
                }
                catch (Exception error) when (
                    error is IOException ||
                    error is TimeoutException ||
                    error is UnauthorizedAccessException ||
                    error is InvalidOperationException ||
                    error is FormatException ||
                    error is ArgumentException)
                {
                    this.Log += "\r\nEEPROM ERROR: " + error.Message;
                    this.dispatcher.BeginInvoke(new Action(() =>
                        MessageBox.Show(this, error.Message, "EEPROM communication error",
                            MessageBoxButton.OK, MessageBoxImage.Error)));
                }
                finally
                {
                    this.eepromOperationInProgress = false;
                }
            });
        }

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
                        this.StartEEPROMTask(() => this.WriteEEPROM(openFileDlg.FileName, true));
                        break;
                    case "bin":
                        this.StartEEPROMTask(() => this.WriteEEPROM(openFileDlg.FileName, false));
                        break;
                    default:
                        this.Log += "\r\nButton TAG ERROR!";
                        break;
                }
            }
        }

        private void Button_Click_5(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.SaveFileDialog saveFileDialog = new Microsoft.Win32.SaveFileDialog
            {
                FileName = "EEPROM_READ.bin",
                Filter = "Binary files (*.bin)|*.bin|All files (*.*)|*.*"
            };
            if (saveFileDialog.ShowDialog() == true)
            {
                this.StartEEPROMTask(() => this.ReadEEPROM(saveFileDialog.FileName));
            }
        }
    }
}
