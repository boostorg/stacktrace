﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace csharp_crashy_app
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        public object Screen { get; }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (e.Source == NormalExceptionButton)
            {
                TestClass testClass = null;
                testClass.testString = "testX";
            }

            if (e.Source == NativeGetCallStack)
            {
                Console.Clear();
                Console.WriteLine("----------- C++ native call stack -----------");
                Console.WriteLine(TestCppCrash.getCallStack());
            }

        }
    }

    public class TestClass
    {
        public String testString;
    }
}
