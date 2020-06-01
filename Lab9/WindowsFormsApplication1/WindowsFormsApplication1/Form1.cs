using System;
using System.Data;
using System.Linq;
using System.Windows.Forms;
using System.IO.Ports;
using System.Drawing;
using System.Collections.Generic;
using System.Threading;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        private int byteCount = 0;
        private int packetNumber;
        private List<int> droppedPackets = new List<int>();
        private List<byte> info = new List<byte>();
        private byte[] packet = new byte[3];
        private int button = 0;
        private bool response=false;


        private byte[] crctable = CalulateTable_CRC8();
        /* { 0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53, 0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
        0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E, 0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
        0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4, 0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
        0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19, 0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
        0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40, 0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
        0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D, 0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
        0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7, 0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
        0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A, 0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
        0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75, 0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
        0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8, 0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
        0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2, 0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
        0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F, 0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
        0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66, 0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
        0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB, 0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
        0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1, 0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
        0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4

};*/
     
        byte Compute_CRC8(byte[] bytes)
        {
            byte crc = 0;
            for (int i = 0; i < 3; i++)
            {
                /* XOR-in next input byte */
                byte data = (byte)(bytes[i] ^ crc);
                /* get current CRC value = remainder */
                crc = (byte)(crctable[data]);
            }

            return crc;
        }

        public Form1()
        {
            InitializeComponent();
            byte[] bytes = { 0x01, 0x02, 0x03};

        }
        
        private void comboBox1_Click(object sender, EventArgs e)
        {
            int num;
            comboBox1.Items.Clear();
            string[] ports = SerialPort.GetPortNames().OrderBy(a => a.Length > 3 && int.TryParse(a.Substring(3), out num) ? num : 0).ToArray();
            comboBox1.Items.AddRange(ports);
        }

        private void buttonOpenPort_Click(object sender, EventArgs e)
        {
            if (!serialPort1.IsOpen)
                try
                {
                    serialPort1.PortName = comboBox1.Text;
                    serialPort1.Open();
                    buttonOpenPort.Text = "Close";
                    comboBox1.Enabled = false;
                    button1.Visible = true;
                    button2.Visible = true;
                }
                catch
                {
                    MessageBox.Show("Port " + comboBox1.Text + " is invalid!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            else
            {
                serialPort1.Close();
                buttonOpenPort.Text = "Open";
                comboBox1.Enabled = true;
                button1.Visible = false;
                button2.Visible = false;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            response = false;
            label2.Text = "";
            byte[] b1 = new byte[4];
            b1[0] = 0x37;
            b1[1] = 0xA2;
            b1[2] = 0xA3;
            byte crc = Compute_CRC8(b1);
            b1[3] = crc;
            serialPort1.Write(b1, 0, 4);
            button = 1;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            response = false;
            label3.Text = "";
            byte[] b1 = new byte[4];
            b1[0] = 0x41;
            b1[1] = 0xA2;
            b1[2] = 0xA3;
            byte crc = Compute_CRC8(b1);
            b1[3] = crc;
            serialPort1.Write(b1, 0, 4);
            button = 2;
        }

        delegate void SetTextCallback(string text);

        private void updateText1(string txt)
        {
            if (this.label2.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(updateText1);
                this.Invoke(d, new object[] { txt });
            }
            else
            {
                this.label2.Text += txt;
            }
        }

        private void updateText2(string txt)
        {
            if (this.label3.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(updateText2);
                this.Invoke(d, new object[] { txt });
            }
            else
            {
                this.label3.Text += txt;
            }
        }

        private void startTimer()
        {
            timer1.Start();
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            while (serialPort1.BytesToRead > 0)
            {
                byte commandFromArduino = (byte)serialPort1.ReadByte();
                if (byteCount < 3)
                {
                    packet[byteCount++] = commandFromArduino;
                }
                else
                {
                    if (Compute_CRC8(packet) != commandFromArduino)
                    {
                        droppedPackets.Add(packetNumber);
                        byteCount = 0;
                    }
                    else
                    {
                        byteCount = 0;
                        if (response)
                        {
                            packetNumber++;
                            info.AddRange(packet);
                            if (button == 1)
                            {
                                updateText1(System.Text.Encoding.ASCII.GetString(packet));
                            }
                            else if (button == 2)
                            {
                                updateText2(System.Text.Encoding.ASCII.GetString(packet));
                            }
                        } else
                        {
                            response = true;
                        }
                    }
                }

            }
        }
        public static byte[] CalulateTable_CRC8()
        {
            const byte generator = 0x1D;
            byte[] crctabl = new byte[256];
            /* iterate over all byte values 0 - 255 */
            for (int divident = 0; divident < 256; divident++)
            {
                byte currByte = (byte)divident;
                /* calculate the CRC-8 value for current byte */
                for (byte bit = 0; bit < 8; bit++)
                {
                    if ((currByte & 0x80) != 0)
                    {
                        currByte <<= 1;
                        currByte ^= generator;
                    }
                    else
                    {
                        currByte <<= 1;
                    }
                }
                /* store CRC value in lookup table */
                crctabl[divident] = currByte;
            }
            return crctabl;
        }
    }
}
