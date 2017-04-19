using System;
using System.Collections.Generic;
using System.ComponentModel;
//using System.Data;
//using System.Drawing;
//using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace chess
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            comboBox1.SelectedIndex = 4;
            textBox2.Focus();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string[] widhei = comboBox1.Text.Split('x');
            string IP = textBox2.Text;
            string port = textBox1.Text;
            Process process = new Process();
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.WindowStyle = ProcessWindowStyle.Normal;
            startInfo.FileName = "client\\client.exe";
            startInfo.Arguments = widhei[0] + " " + widhei[1] + " " + IP + " " + port;
            process.StartInfo = startInfo;
            process.Start();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            string port = textBox1.Text;
            Process process = new Process();
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.WindowStyle = ProcessWindowStyle.Normal;
            startInfo.FileName = "client\\server.exe";
            startInfo.Arguments = port;
            process.StartInfo = startInfo;
            process.Start();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            string[] widhei = comboBox1.Text.Split('x');
            Process process = new Process();
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.WindowStyle = ProcessWindowStyle.Normal;
            startInfo.FileName = "client\\client.exe";
            startInfo.Arguments = widhei[0] + " " + widhei[1];
            process.StartInfo = startInfo;
            process.Start();
        }
    }
}
