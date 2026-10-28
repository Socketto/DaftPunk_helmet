using Android;
using Android.App;
using Android.Bluetooth;
using Android.Content;
using Android.Media;
using Android.OS;
using Android.Support.Design.Widget;
using Android.Support.V4.View;
using Android.Support.V4.Widget;
using Android.Support.V7.App;
using Android.Views;
using Android.Widget;

using Java.Util;

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DaftPunkHelmet
{
    [Activity(Label = "@string/app_name", Theme = "@style/AppTheme.NoActionBar", MainLauncher = true)]
    public class MainActivity : AppCompatActivity, NavigationView.IOnNavigationItemSelectedListener
    {
        private const int ReadMemoryRequestCode = 1001;
        private const int WriteMemoryRequestCode = 1002;
        private const int MemorySize = 1024;
        private const int MemoryFrameDelayMilliseconds = 150;
        private const int MemoryResponseToNextCommandDelayMilliseconds = 750;
        private const int MemoryResponseTimeoutMilliseconds = 15000;
        private const int MemoryReadMaxRetries = 8;
        private const int MemoryReadRetryDelayMilliseconds = 1000;

        public MediaRecorder mRecorder;
        System.Timers.Timer tt;
        private bool memoryOperationRunning;
        private bool pendingTextWrite;

        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);
            this.SetContentView(Resource.Layout.activity_main);
            Android.Support.V7.Widget.Toolbar toolbar = this.FindViewById<Android.Support.V7.Widget.Toolbar>(Resource.Id.toolbar);
            this.SetSupportActionBar(toolbar);

            FloatingActionButton fab = this.FindViewById<FloatingActionButton>(Resource.Id.fab);
            fab.Click += this.FabOnClick;

            Button setred = this.FindViewById<Button>(Resource.Id.set_red);
            setred.Click += this.SendRGB;
            Button setblue = this.FindViewById<Button>(Resource.Id.set_blue);
            setblue.Click += this.SendRGB;
            Button setgreen = this.FindViewById<Button>(Resource.Id.set_green);
            setgreen.Click += this.SendRGB;
            Button setcian = this.FindViewById<Button>(Resource.Id.set_cian);
            setcian.Click += this.SendRGB;
            Button setblack = this.FindViewById<Button>(Resource.Id.set_black);
            setblack.Click += this.SendRGB;
            Button setblinkfast = this.FindViewById<Button>(Resource.Id.set_blink_fast);
            setblinkfast.Click += this.SendRGB;
            Button setblinkflash = this.FindViewById<Button>(Resource.Id.set_blink_flash);
            setblinkflash.Click += this.SendRGB;
            Button setblinknone = this.FindViewById<Button>(Resource.Id.set_blink_none);
            setblinknone.Click += this.SendRGB;
            Button setblinknormal = this.FindViewById<Button>(Resource.Id.set_blink_normal);
            setblinknormal.Click += this.SendRGB;
            Button setblinkslow = this.FindViewById<Button>(Resource.Id.set_blink_slow);
            setblinkslow.Click += this.SendRGB;
            Button setrandom = this.FindViewById<Button>(Resource.Id.set_random);
            setrandom.Click += this.SendRGB;
            Button setviolet = this.FindViewById<Button>(Resource.Id.set_violet);
            setviolet.Click += this.SendRGB;
            Button setwhite = this.FindViewById<Button>(Resource.Id.set_white);
            setwhite.Click += this.SendRGB;
            Button setyellow = this.FindViewById<Button>(Resource.Id.set_yellow);
            setyellow.Click += this.SendRGB;

            EditText edittext = this.FindViewById<EditText>(Resource.Id.edittext);
            edittext.KeyPress += (object sender, View.KeyEventArgs e) =>
            {
                e.Handled = false;
                if (e.Event.Action == KeyEventActions.Down && e.KeyCode == Keycode.Enter)
                {
                    /*ACTION STRING*/

                    var frase = edittext.Text;
                    List<byte> arraybytes = new List<byte>();
                    arraybytes.AddRange(System.Text.Encoding.ASCII.GetBytes(frase));
                    arraybytes.Insert(0, 0x50);
                    arraybytes.Insert(1, 0x01);
                    if (this.btSocket?.IsConnected == true)
                    {
                        this.Send_TO_HC(arraybytes.ToArray());
                        Toast.MakeText(this, edittext.Text, ToastLength.Short).Show();
                    }
                    else
                    {
                        Toast.MakeText(this, "NOT CONNECTED!", ToastLength.Short).Show();
                    }
                    e.Handled = true;
                    edittext.Text = "";
                }
            };

            DrawerLayout drawer = this.FindViewById<DrawerLayout>(Resource.Id.drawer_layout);
            ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, Resource.String.navigation_drawer_open, Resource.String.navigation_drawer_close);
            drawer.AddDrawerListener(toggle);
            toggle.SyncState();


            NavigationView navigationView = this.FindViewById<NavigationView>(Resource.Id.nav_view);
            navigationView.SetNavigationItemSelectedListener(this);
            this.tt = new System.Timers.Timer();
            //Trigger event every second
            tt.Interval = 80;
            tt.Elapsed += Send_TO_HC_;

            tt.Enabled = true;
        }

        private void startMic()
        {
            if (Android.Support.V4.App.ActivityCompat.CheckSelfPermission(this, Manifest.Permission.RecordAudio) != Android.Content.PM.Permission.Granted)
            {

                Android.Support.V4.App.ActivityCompat.RequestPermissions(this, new String[] { Manifest.Permission.RecordAudio },
                        0);

            }
            try
            {
                this.mRecorder = new MediaRecorder();
                this.mRecorder.SetAudioSource(AudioSource.Mic);
                this.mRecorder.SetOutputFormat(OutputFormat.ThreeGpp);
                this.mRecorder.SetAudioEncoder(AudioEncoder.AmrNb);
                this.mRecorder.SetOutputFile("/dev/null");
                this.mRecorder.Prepare();
                this.mRecorder.Start();
            }
            catch { }

        }

        public int getAmplitude()
        {
            if (mRecorder == null)
            {
                this.startMic();
            }
            return mRecorder.MaxAmplitude;
        }

        byte[] requestpublic = null;
        string palla = "palla";
        void Send_TO_HC(byte[] request)
        {
            lock (palla)
            {
                if (!this.memoryOperationRunning)
                {
                    requestpublic = request;
                }
            }
        }

        int tempApmlitude;
        int maxAmp = 0;
        void Send_TO_HC_(object sender, System.Timers.ElapsedEventArgs e)
        {
            lock (palla)
            {
                if (this.memoryOperationRunning)
                {
                    return;
                }

                if (this.btSocket?.IsConnected == true)
                {
                    if (requestpublic != null)
                    {
                        this.WriteBluetoothRequest(requestpublic, requestpublic.Length < 3 ? 10 : 50);
                        requestpublic = null;
                    }
                    else
                    {
                        /*tempApmlitude = getAmplitude();
                        if(maxAmp < tempApmlitude)
                        {
                            maxAmp = tempApmlitude;
                        }
                        var temp = Convert.ToByte(tempApmlitude / 128); //2048
                        Send_TO_HC(new byte[] { 0xDC, temp });*/
                    }
                }
            }
        }

        public override void OnBackPressed()
        {
            DrawerLayout drawer = this.FindViewById<DrawerLayout>(Resource.Id.drawer_layout);
            if (drawer.IsDrawerOpen(GravityCompat.Start))
            {
                drawer.CloseDrawer(GravityCompat.Start);
            }
            else
            {
                base.OnBackPressed();
            }
        }

        public override bool OnCreateOptionsMenu(IMenu menu)
        {
            this.MenuInflater.Inflate(Resource.Menu.menu_main, menu);
            return true;
        }

        public override bool OnOptionsItemSelected(IMenuItem item)
        {
            int id = item.ItemId;
            if (id == Resource.Id.action_settings)
            {
                return true;
            }

            return base.OnOptionsItemSelected(item);
        }

        private void FabOnClick(object sender, EventArgs eventArgs)
        {
            View view = (View)sender;
            Snackbar.Make(view, "marco.sambin88@gmail.com", Snackbar.LengthLong)
                .SetAction("Action", (Android.Views.View.IOnClickListener)null).Show();
        }

        public bool OnNavigationItemSelected(IMenuItem item)
        {
            int id = item.ItemId;

            if (id == Resource.Id.nav_send)
            {
                this.bluetoothf();
            }

            if (id == Resource.Id.nav_on)
            {
                if (this.btSocket?.IsConnected == true)
                {
                    this.CommunicationI2C(true);
                }
                else
                {
                    Android.App.AlertDialog.Builder alert = new Android.App.AlertDialog.Builder(this);
                    alert.SetTitle("ERROR!\n---NOT CONNECTED!!!---");
                    alert.SetPositiveButton("OK", (senderAlert, args) =>
                    {
                        // write your own set of instructions
                    });
                }
            }

            if (id == Resource.Id.nav_off)
            {
                if (this.btSocket?.IsConnected == true)
                {
                    this.CommunicationI2C(false);
                }
                else
                {
                    Android.App.AlertDialog.Builder alert = new Android.App.AlertDialog.Builder(this);
                    alert.SetTitle("ERROR!\n---NOT CONNECTED!!!---");
                    alert.SetPositiveButton("OK", (senderAlert, args) =>
                    {
                        // write your own set of instructions
                    });
                }
            }

            if (id == Resource.Id.nav_read_memory)
            {
                this.StartMemoryFilePicker(true, false);
            }

            if (id == Resource.Id.nav_write_memory_binary)
            {
                this.StartMemoryFilePicker(false, false);
            }

            if (id == Resource.Id.nav_write_memory_text)
            {
                this.StartMemoryFilePicker(false, true);
            }

            DrawerLayout drawer = this.FindViewById<DrawerLayout>(Resource.Id.drawer_layout);
            drawer.CloseDrawer(GravityCompat.Start);
            return true;
        }

        private void StartMemoryFilePicker(bool readMemory, bool textFile)
        {
            if (this.btSocket?.IsConnected != true)
            {
                this.ShowMemoryMessage("Bluetooth error", "Connect to the helmet before transferring memory.");
                return;
            }

            try
            {
                Intent intent;
                int requestCode;
                if (readMemory)
                {
                    intent = new Intent(Intent.ActionCreateDocument);
                    intent.AddCategory(Intent.CategoryOpenable);
                    intent.SetType("application/octet-stream");
                    intent.PutExtra(Intent.ExtraTitle, "EEPROM_READ.bin");
                    requestCode = ReadMemoryRequestCode;
                }
                else
                {
                    intent = new Intent(Intent.ActionOpenDocument);
                    intent.AddCategory(Intent.CategoryOpenable);
                    intent.SetType("*/*");
                    this.pendingTextWrite = textFile;
                    requestCode = WriteMemoryRequestCode;
                }

                this.StartActivityForResult(intent, requestCode);
            }
            catch (ActivityNotFoundException ex)
            {
                this.ShowMemoryMessage("File picker unavailable", ex.Message);
            }
        }

        protected override void OnActivityResult(int requestCode, Result resultCode, Intent data)
        {
            base.OnActivityResult(requestCode, resultCode, data);
            if (resultCode != Result.Ok || data == null || data.Data == null)
            {
                return;
            }

            Android.Net.Uri fileUri = data.Data;
            if (requestCode == ReadMemoryRequestCode)
            {
                this.StartMemoryTransfer("Reading EEPROM", progress => this.ReadMemoryToFile(fileUri, progress));
            }
            else if (requestCode == WriteMemoryRequestCode)
            {
                bool isText = this.pendingTextWrite;
                this.StartMemoryTransfer("Writing EEPROM", progress => this.WriteFileToMemory(fileUri, isText, progress));
            }
        }

        private void StartMemoryTransfer(string title, Action<ProgressDialog> transfer)
        {
            lock (palla)
            {
                if (this.memoryOperationRunning)
                {
                    this.ShowMemoryMessage("Transfer in progress", "Wait for the current memory transfer to finish.");
                    return;
                }

                if (this.btSocket?.IsConnected != true)
                {
                    this.ShowMemoryMessage("Bluetooth error", "The connection to the helmet is no longer active.");
                    return;
                }

                this.memoryOperationRunning = true;
                requestpublic = null;
            }

            ProgressDialog progress = new ProgressDialog(this);
            progress.SetTitle(title);
            progress.SetMessage("Preparing transfer...");
            progress.SetProgressStyle(ProgressDialogStyle.Horizontal);
            progress.Max = MemorySize;
            progress.SetCancelable(false);
            progress.Show();

            Thread transferThread = new Thread(() =>
            {
                string errorMessage = null;
                try
                {
                    transfer(progress);
                }
                catch (Exception ex)
                {
                    errorMessage = ex.Message;
                }
                finally
                {
                    lock (palla)
                    {
                        this.memoryOperationRunning = false;
                    }

                    this.RunOnUiThread(() =>
                    {
                        progress.Dismiss();
                        if (errorMessage == null)
                        {
                            Toast.MakeText(this, "Memory transfer completed.", ToastLength.Long).Show();
                        }
                        else
                        {
                            this.ShowMemoryMessage("Memory transfer failed", errorMessage);
                        }
                    });
                }
            });
            transferThread.IsBackground = true;
            transferThread.Start();
        }

        private void ReadMemoryToFile(Android.Net.Uri fileUri, ProgressDialog progress)
        {
            this.RunMemoryProtocol(() =>
            {
                byte[] memory = new byte[MemorySize];
                for (int address = 0; address < MemorySize; address++)
                {
                    try
                    {
                        memory[address] = this.ReadMemoryByteWithRetry(
                            new byte[] { 0x10, 0x01, (byte)(address / 256), (byte)address },
                            progress,
                            address);
                    }
                    catch (Exception ex)
                    {
                        throw new IOException(
                            "Failed to read EEPROM address 0x" + address.ToString("X4", CultureInfo.InvariantCulture) + ": " + ex.Message,
                            ex);
                    }

                    this.UpdateMemoryProgress(progress, address + 1, "Reading memory...");
                }

                using (System.IO.Stream output = this.ContentResolver.OpenOutputStream(fileUri))
                {
                    if (output == null)
                    {
                        throw new IOException("The selected document cannot be opened for writing.");
                    }

                    output.Write(memory, 0, memory.Length);
                    output.Flush();
                }
            });
        }

        private void WriteFileToMemory(Android.Net.Uri fileUri, bool isText, ProgressDialog progress)
        {
            byte[] memory;
            using (System.IO.Stream input = this.ContentResolver.OpenInputStream(fileUri))
            {
                if (input == null)
                {
                    throw new IOException("The selected document cannot be opened for reading.");
                }

                if (isText)
                {
                    using (StreamReader reader = new StreamReader(input, System.Text.Encoding.UTF8))
                    {
                        memory = this.ParseTextMemory(reader.ReadToEnd());
                    }
                }
                else
                {
                    using (MemoryStream buffer = new MemoryStream())
                    {
                        input.CopyTo(buffer);
                        memory = buffer.ToArray();
                    }
                }
            }

            if (memory.Length == 0)
            {
                throw new InvalidDataException("The selected file contains no memory bytes.");
            }

            if (memory.Length > MemorySize)
            {
                throw new InvalidDataException("The selected file is larger than the 1024-byte EEPROM.");
            }

            this.RunMemoryProtocol(() =>
            {
                for (int address = 0; address < memory.Length; address++)
                {
                    this.SendMemoryCommand(new byte[] { 0x10, 0x02, (byte)(address / 256), (byte)address, memory[address] });
                    this.ReadMemoryAcknowledgement();
                    this.UpdateMemoryProgress(progress, address + 1, "Writing memory...");
                }
            });
        }

        private byte[] ParseTextMemory(string text)
        {
            string normalized = text.Replace("\r", "").Replace("\n", "\t").Replace(" ", "").Trim();
            string[] values = normalized.Split(new[] { '\t' }, StringSplitOptions.RemoveEmptyEntries);
            List<byte> bytes = new List<byte>(values.Length);
            foreach (string value in values)
            {
                bytes.Add(byte.Parse(value, NumberStyles.HexNumber, CultureInfo.InvariantCulture));
            }

            return bytes.ToArray();
        }

        private void RunMemoryProtocol(Action transfer)
        {
            this.SendMemoryCommand(new byte[] { 0x10, 0x03 });
            this.ReadMemoryAcknowledgement();
            Thread.Sleep(200);
            try
            {
                transfer();
            }
            finally
            {
                if (this.btSocket?.IsConnected == true)
                {
                    this.SendMemoryCommand(new byte[] { 0x10, 0x04 });
                    this.ReadMemoryAcknowledgement();
                    Thread.Sleep(200);
                }
            }
        }

        private void SendMemoryCommand(byte[] request)
        {
            lock (palla)
            {
                if (this.btSocket?.IsConnected != true)
                {
                    throw new IOException("The Bluetooth connection to the helmet was lost.");
                }

                this.WriteBluetoothRequest(request, 1);
            }

            Thread.Sleep(MemoryFrameDelayMilliseconds);
        }

        private void WriteBluetoothRequest(byte[] request, int delayMilliseconds)
        {
            this.btSocket.OutputStream.Write(new byte[] { (byte)request.Length }, 0, 1);
            this.btSocket.OutputStream.Flush();
            Thread.Sleep(delayMilliseconds);

            foreach (byte value in request)
            {
                this.btSocket.OutputStream.Write(new byte[] { value }, 0, 1);
                this.btSocket.OutputStream.Flush();
                Thread.Sleep(delayMilliseconds);
            }
        }

        private byte ReadMemoryByte()
        {
            byte[] response = this.ReadMemoryResponse();
            if (response.Length != 2 || response[0] != 0x0A)
            {
                throw new IOException(
                    "Unexpected response from the helmet while reading memory (length "
                    + response.Length.ToString(CultureInfo.InvariantCulture)
                    + ", payload "
                    + BitConverter.ToString(response)
                    + ").");
            }

            return response[1];
        }

        private byte ReadMemoryByteWithRetry(byte[] request, ProgressDialog progress, int address)
        {
            Exception lastException = null;

            for (int attempt = 1; attempt <= MemoryReadMaxRetries; attempt++)
            {
                try
                {
                    this.UpdateMemoryProgress(
                        progress,
                        address,
                        "Reading memory... address 0x"
                        + address.ToString("X4", CultureInfo.InvariantCulture)
                        + " (attempt "
                        + attempt.ToString(CultureInfo.InvariantCulture)
                        + "/"
                        + MemoryReadMaxRetries.ToString(CultureInfo.InvariantCulture)
                        + ")");
                    this.SendMemoryCommand(request);
                    return this.ReadMemoryByte();
                }
                catch (Exception ex)
                {
                    lastException = ex;
                    if (attempt < MemoryReadMaxRetries)
                    {
                        Thread.Sleep(MemoryReadRetryDelayMilliseconds);
                    }
                }
            }

            throw new IOException(
                "The memory address could not be read after "
                + MemoryReadMaxRetries.ToString(CultureInfo.InvariantCulture)
                + " attempts.", lastException);
        }

        private void ReadMemoryAcknowledgement()
        {
            byte[] response = this.ReadMemoryResponse();
            if (response.Length != 1 || response[0] != 0x0A)
            {
                throw new IOException("Unexpected acknowledgement from the helmet.");
            }
        }

        private byte[] ReadMemoryResponse()
        {
            byte[] responseLength = new byte[1];
            this.ReadBluetoothBytes(responseLength, 0, responseLength.Length);
            if (responseLength[0] == 0)
            {
                throw new IOException("The helmet returned an empty response.");
            }

            byte[] response = new byte[responseLength[0]];
            this.ReadBluetoothBytes(response, 0, response.Length);
            Thread.Sleep(MemoryResponseToNextCommandDelayMilliseconds);
            return response;
        }

        private void ReadBluetoothBytes(byte[] buffer, int offset, int count)
        {
            int bytesReadTotal = 0;

            while (bytesReadTotal < count)
            {
                if (this.btSocket?.IsConnected != true)
                {
                    throw new IOException("The Bluetooth connection to the helmet was lost while reading memory.");
                }

                System.IO.Stream input = this.btSocket.InputStream;
                Task<int> readTask = Task.Run(
                    () => input.Read(buffer, offset + bytesReadTotal, count - bytesReadTotal));
                if (!readTask.Wait(MemoryResponseTimeoutMilliseconds))
                {
                    if (!readTask.IsCompleted)
                    {
                        if (this.btSocket != null)
                        {
                            this.btSocket.Close();
                        }

                        throw new TimeoutException("The helmet did not respond to a memory read command within "
                            + MemoryResponseTimeoutMilliseconds + " ms.");
                    }
                }

                int bytesRead = readTask.Result;
                if (bytesRead <= 0)
                {
                    throw new IOException("The Bluetooth connection closed while reading memory.");
                }

                bytesReadTotal += bytesRead;
            }
        }

        private void UpdateMemoryProgress(ProgressDialog progress, int completed, string message)
        {
            this.RunOnUiThread(() =>
            {
                progress.Progress = completed;
                progress.SetMessage(message + " " + completed + "/" + MemorySize);
            });
        }

        private void ShowMemoryMessage(string title, string message)
        {
            this.RunOnUiThread(() =>
            {
                new Android.App.AlertDialog.Builder(this)
                    .SetTitle(title)
                    .SetMessage(message)
                    .SetPositiveButton("OK", (sender, args) => { })
                    .Show();
            });
        }


        BluetoothSocket btSocket;
        bool requestConnection = false;
        private void bluetoothf()
        {
            Thread thread = new Thread(() =>
            {
                if ((this.btSocket?.IsConnected == true) || (requestConnection == true))
                {
                    if (this.btSocket?.IsConnected == true)
                    {
                        this.btSocket.Close();
                        Android.App.AlertDialog.Builder alert = new Android.App.AlertDialog.Builder(this);
                        alert.SetTitle("DISCONNECTED!");
                        alert.SetPositiveButton("OK", (senderAlert, args) =>
                        {
                            // write your own set of instructions
                        });
                    }
                    this.btSocket = null;
                    this.requestConnection = false;
                    return;
                }

                requestConnection = true;
                UUID sppUuid = UUID.FromString("00001101-0000-1000-8000-00805F9B34FB");

                var btAdapter = BluetoothAdapter.DefaultAdapter;
                if (btAdapter == null)
                {
                    this.sendLogMessage("Bluetooth adapter is not available.");
                    requestConnection = false;
                    return; // false;
                }
                this.sendLogMessage("Bluetooth adapter is found.");

                if (!btAdapter.IsEnabled)
                {
                    this.sendLogMessage("Bluetooth is disabled. Check configuration.");
                    requestConnection = false;
                    return; // false;
                }
                this.sendLogMessage("Bluetooth is enabled.");

                BluetoothDevice btDevice = null;
                ICollection<BluetoothDevice> bondedDevices = btAdapter.BondedDevices;
                foreach (BluetoothDevice dev in bondedDevices)
                {
                    this.sendLogMessage("Paired device: " + dev.Name + " (" + dev.Address + ")");

                    if (dev.Name.Equals("HELMET"))
                    {
                        btDevice = dev;
                    }
                }

                if (btDevice == null)
                {
                    this.sendLogMessage("Target Bluetooth device is not found.");
                    requestConnection = false;
                    return; // false;
                }
                this.sendLogMessage("Target Bluetooth device is found.");


                try
                {
                    this.btSocket = btDevice.CreateRfcommSocketToServiceRecord(sppUuid);
                }
                catch (Java.IO.IOException ex)
                {
                    this.sendLogMessage("Failed to create RfComm socket: " + ex.ToString());
                    requestConnection = false;
                    return; // false;
                }
                this.sendLogMessage("Created a bluetooth socket.");


                try
                {
                    this.btSocket.Connect();
                    Android.App.AlertDialog.Builder alert = new Android.App.AlertDialog.Builder(this);
                    alert.SetTitle("CONNECTED!");
                    alert.SetPositiveButton("OK", (senderAlert, args) =>
                    {
                        // write your own set of instructions
                    });

                    //run the alert in UI thread to display in the screen
                    this.RunOnUiThread(() =>
                    {
                        alert.Show();
                    });
                }
                catch (Exception ee)
                {
                    Android.App.AlertDialog.Builder alert = new Android.App.AlertDialog.Builder(this);
                    alert.SetTitle("ERROR!\n---NOT CONNECTED!!!---" + ee.ToString());
                    alert.SetPositiveButton("OK", (senderAlert, args) =>
                    {
                        // write your own set of instructions
                    });

                    //run the alert in UI thread to display in the screen
                    this.RunOnUiThread(() =>
                    {
                        alert.Show();
                    });
                }


                this.sendLogMessage("Connected to the bluetooth socket.");
                requestConnection = false;
            });
            thread.Start();
        }

        private void CommunicationI2C(bool start = true)
        {
            try
            {
                if (this.btSocket?.IsConnected == true)
                {
                    if (start)
                    {
                        Send_TO_HC(new byte[] { 0x10, 0x04 });
                    }
                    else
                    {
                        Send_TO_HC(new byte[] { 0x10, 0x03 });
                    }
                }
            }
            catch { }
        }


        private void SendRGB(object sender, EventArgs eventArgs)
        {
            try
            {
                if (this.btSocket?.IsConnected == true)
                {
                    switch (((View)sender).Id)
                    {
                        case Resource.Id.set_red:
                            //RED	0x01	0x01
                            Send_TO_HC(new byte[] { 0x01 });
                            break;
                        case Resource.Id.set_blue:
                            //BLUE	0x01	0x03
                            Send_TO_HC(new byte[] { 0x03 });
                            break;
                        case Resource.Id.set_green:
                            //GREEN	0x01	0x02
                            Send_TO_HC(new byte[] { 0x02 });
                            break;
                        case Resource.Id.set_black:
                            //OFF	0x01	0x00
                            Send_TO_HC(new byte[] { 0x00 });
                            break;
                        case Resource.Id.set_violet:
                            //VIOLET	0x01	0x04
                            Send_TO_HC(new byte[] { 0x04 });
                            break;
                        case Resource.Id.set_yellow:
                            //YELLOW	0x01	0x05
                            Send_TO_HC(new byte[] { 0x05 });
                            break;
                        case Resource.Id.set_cian:
                            //LIGHT BLUE	0x01	0x06
                            Send_TO_HC(new byte[] { 0x06 });
                            break;
                        case Resource.Id.set_white:
                            //WHITE	0x01	0x07
                            Send_TO_HC(new byte[] { 0x07 });
                            break;
                        case Resource.Id.set_blink_none:
                            //BLINK_NO	0x01	0x0A
                            Send_TO_HC(new byte[] { 0x0A });
                            break;
                        case Resource.Id.set_blink_normal:
                            //BLINK 500ms	0x01	0x0B
                            Send_TO_HC(new byte[] { 0x0B });
                            break;
                        case Resource.Id.set_blink_slow:
                            //BLINK 1s	0x01	0x0C
                            Send_TO_HC(new byte[] { 0x0C });
                            break;
                        case Resource.Id.set_blink_fast:
                            //BLINK 184ms	0x01	0x0D
                            Send_TO_HC(new byte[] { 0x0D });
                            break;
                        case Resource.Id.set_blink_flash:
                            //BLINK 75ms	0x01	0x0E
                            Send_TO_HC(new byte[] { 0x0E });
                            break;
                        case Resource.Id.set_random:
                            //RANDOM COLOR	0x01	0x08
                            Send_TO_HC(new byte[] { 0x08 });
                            break;
                    }
                }
                else
                {
                    View view = (View)sender;
                    Snackbar.Make(view, "Error disconnected!", Snackbar.LengthLong)
                        .SetAction("Action", (Android.Views.View.IOnClickListener)null).Show();
                }
            }
            catch
            {

                View view = (View)sender;
                Snackbar.Make(view, "Error disconnected!", Snackbar.LengthLong)
                    .SetAction("Action", (Android.Views.View.IOnClickListener)null).Show();
                return;
            }
        }


        void sendLogMessage(string message)
        {
            //Android.App.AlertDialog.Builder alertDialogBuilder = new Android.App.AlertDialog.Builder(this);
            //alertDialogBuilder.SetMessage(message);
            //Android.App.AlertDialog alertDialog = alertDialogBuilder.Create();
            //alertDialog.Show();
        }

    }
}
