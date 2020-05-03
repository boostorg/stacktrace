using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.ExceptionServices;
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

        [HandleProcessCorruptedStateExceptions]
        void NativeCrash_ManagedGuarded()
        {
            try
            {
                TestCppCrash.crashingFunction();
            }
            catch (Exception ex)
            {
                WriteLine("NativeCrash_ManagedGuarded exception: " + ex.ToString());
            }
        }

        void WriteLine(string format, params object[] arg)
        {
            String msg = String.Format(format, arg);
            Console.Clear();
            Console.WriteLine(msg);
            //MessageBox.Show(msg);
        }


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (e.Source == NormalExceptionButton)
            {
                TestClass testClass = null;
                testClass.testString = "testX";
            }

            if (e.Source == NativeGetCallStackButton)
            {
                WriteLine(TestCppCrash.getCallStack());
            }

            if (e.Source == NativeCrashButton)
            {
                TestCppCrash.crashingFunction();
            }

            if (e.Source == NativeCrashCatchedByManagedButton)
            {
                NativeCrash_ManagedGuarded();
            }

            if (e.Source == NativeCrashViaInvokeButton)
            {
                typeof(TestCppCrash).GetMethod("crashingFunction", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.Public).Invoke(null, new object[] { });
            }

            if (e.Source == NativeCrashCatchedByNativeButton)
            {
                TestCppCrash.crashingFunctionBehindNativeCatch();
            }
        }
    }

    public class TestClass
    {
        public String testString;
    }
}

