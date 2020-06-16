using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Reflection;
using System.Runtime.ExceptionServices;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;

public partial class App : Application
{
    public App()
    {
        DispatcherUnhandledException += App_DispatcherUnhandledException;
        AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
        TestCppCrash.enableCppUnhandledExceptionDispatcher(true);
    }

    /// <summary>
    /// This function will never be reached if native C++ exception handling is registered.
    /// </summary>
    [HandleProcessCorruptedStateExceptions]
    private void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        Console.WriteLine("In CurrentDomain_UnhandledException, reported exception:\n\n" + e.ExceptionObject.ToString());
    }

    private void App_DispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
    {
        Exception ex = e.Exception;

        if (ex is TargetInvocationException)
        {
            ex = ex.InnerException;
        }

        Console.WriteLine("In App_DispatcherUnhandledException, handling exception:\n\n" + ex.ToString());
        e.Handled = true;
    }

    protected override void OnExit(ExitEventArgs e)
    {
        TestCppCrash.enableCppUnhandledExceptionDispatcher(false);
        base.OnExit(e);
    }

    [DllImport("kernel32")]
    private extern static bool SetConsoleFont(IntPtr hOutput, uint index);

    [DllImport("kernel32")]
    private static extern IntPtr GetStdHandle(int index);

    // Starting here application manually just to get rid of extra app.xaml file, which is not necessarily needed for demo purposes.
    // (class App can also reside outside of main namespace)
    [STAThread]
    public static void Main()
    {
        try
        {
            SetConsoleFont(GetStdHandle(-11), 14);
            Console.SetWindowSize(Console.WindowWidth, 50);
        }
        catch (Exception)
        { 
        }

        App app = new App();
        app.StartupUri = new System.Uri("csharp_crashy_window.xaml", System.UriKind.Relative);
        app.Run();
    }
}

