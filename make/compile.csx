// C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\Roslyn cssscrpt.csx

// -clang -msvc
// -debug -ndebug
// -output <output_filepath>
// -force
// -warnings_are_errors

enum Compiler { Clang6_0, MSVC19 };
enum WarningLevel { None, Low, MediumLow, MediumHigh, High, Max }
enum LanguageVersion { Cpp11, Cpp14, Cpp17, Cpp20 }
enum DebugLevel { Debug, NonDebug }

interface ICompilo
{
    void SetupEnvironment();
    string ExecutableName { get; }
    string ExtraFlags { get; }
    string LanguageFlag(LanguageVersion _language_version);
    string OutputFileFlag(string _output_path);
    string WarningLevelFlag(WarningLevel _warning_level);
    string DebugFlags(DebugLevel _optimization);
    string DefineFlag(string _name, object _value);
    string WarningAreErrorsFlag { get; }
}

abstract class Compilo : ICompilo
{
    public abstract void SetupEnvironment();
    public abstract string ExecutableName { get; }
    public abstract string ExtraFlags { get; }
    public abstract string LanguageFlag(LanguageVersion _language_version);
    public abstract string OutputFileFlag(string _output_path);
    public abstract string WarningLevelFlag(WarningLevel _warning_level);
    public virtual string DebugFlags(DebugLevel _optimization)
    {
        if (_optimization==DebugLevel.Debug)
            return DefineFlag("DEBUG",1);
        else
            return string.Join(" ", DefineFlag("DEBUG",0), DefineFlag("NDEBUG",null));
    }
    public abstract string DefineFlag(string _name, object _value);
    public abstract string WarningAreErrorsFlag { get; }
}

class MSVC : Compilo
{
    public override void SetupEnvironment()
    {
        Environment.SetEnvironmentVariable( "PATH",
            $@"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\bin\HostX64\x64;{Environment.GetEnvironmentVariable("PATH")}");
        Environment.SetEnvironmentVariable( "INCLUDE", string.Join(";"
            //, @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\ATLMFC\include"
            , @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\include"
            //, @"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.1\include\um"
            , @"C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\ucrt"
            //, @"C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\shared"
            //, @"C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\um"
            //, @"C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\winrt"
            //, @"C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\cppwinrt"
            ));
        Environment.SetEnvironmentVariable( "LIB", string.Join(";"
            //, @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\ATLMFC\lib\x64"
            , @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\lib\x64"
            //, @"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.1\lib\um\x64"
            , @"C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\ucrt\x64"
            , @"C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\um\x64"
            ));
     //LIBPATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\ATLMFC\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128\lib\x86\store\references;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\10.0.16299.0;C:\Program Files (x86)\Windows Kits\10\References\10.0.16299.0;C:\Windows\Microsoft.NET\Framework64\v4.0.30319;
    }
    public override string ExecutableName => "cl";
    public override string ExtraFlags => "/EHsc";
    public override string LanguageFlag(LanguageVersion _language_version)
    {
        switch (_language_version)
        {
            case LanguageVersion.Cpp11: return "/std:c++11";
            case LanguageVersion.Cpp14: return "/std:c++14";
            case LanguageVersion.Cpp17: return "/std:c++17";
            case LanguageVersion.Cpp20: return "/std:c++latest";
            default: goto case LanguageVersion.Cpp17;
        }
    }
    public override string OutputFileFlag(string _output_path) => $"/Fe{_output_path}";
    public override string WarningLevelFlag(WarningLevel _warning_level)
    {
        switch(_warning_level)
        {
            case WarningLevel.None: return "/W0";
            case WarningLevel.Low: return "/W1";
            case WarningLevel.MediumLow: return "/W2";
            case WarningLevel.MediumHigh: return "/W3";
            case WarningLevel.High: return "/W4";
            case WarningLevel.Max: return "/Wall";
            default: goto case WarningLevel.Low;
        }
    }
    public override string DebugFlags(DebugLevel _optimization)
    {
        if (_optimization==DebugLevel.Debug)
            return string.Join(" ", base.DebugFlags(_optimization), "/Zi", "/Od");
        else
            return string.Join(" ", base.DebugFlags(_optimization), "/Ox");
    }
    public override string DefineFlag(string _name, object _value) => $"/D{_name}={_value}";
    public override string WarningAreErrorsFlag => "/WX";
}

class Clang6_0 : Compilo
{
    public override void SetupEnvironment() {}
    public override string ExecutableName => "clang";
    public override string ExtraFlags => "-Xclang -flto-visibility-public-std";
    public override string LanguageFlag(LanguageVersion _language_version)
    {
        switch (_language_version)
        {
            case LanguageVersion.Cpp11: return "-std=c++11";
            case LanguageVersion.Cpp14: return "-std=c++14";
            case LanguageVersion.Cpp17: return "-std=c++17";
            case LanguageVersion.Cpp20: return "-std=c++2a";
            default: goto case LanguageVersion.Cpp17;
        }
    }
    public override string OutputFileFlag(string _output_path) => $"-o {_output_path}";
    public override string WarningLevelFlag(WarningLevel _warning_level)
    {
        switch(_warning_level)
        {
            case WarningLevel.None: return "";
            case WarningLevel.Low: return "-Wall";
            case WarningLevel.MediumLow: return "-Wall -pedantic";
            case WarningLevel.MediumHigh: return "-Wall -pedantic";
            case WarningLevel.High: return "-Wall -pedantic -Wextra";
            case WarningLevel.Max: return "-Wall -pedantic -Wextra";
            default: goto case WarningLevel.Low;
        }
    }
    public override string DebugFlags(DebugLevel _optimization)
    {
        if (_optimization==DebugLevel.Debug)
            return string.Join(" ", base.DebugFlags(_optimization), "-O0");
        else
            return string.Join(" ", base.DebugFlags(_optimization), "-O3");
    }
    public override string DefineFlag(string _name, object _value) => $"-D{_name}={_value}";
    public override string WarningAreErrorsFlag => "-Werror";
}

class Arguments
{
    public Compiler? Compiler;
    public DebugLevel? DebugLevel;
    public string OutputFilename;
    public List<string> SourceFilenames = new List<string>();
    public bool ForceCompilation = false;
    public bool WarningsAreErrors = false;
}

(bool no_error, Arguments arguments) ParseArguments()
{
    bool error = false;
    Arguments arguments = new Arguments();
    var arg_count = Args.Count;
    for(var i=0; i < arg_count; ++i)
    {
        if(Args[i][0]=='-')
        {
            string option = Args[i].Substring(1);
            switch(option)
            {
                case "clang":
                if (arguments.Compiler.HasValue)
                {
                    Console.WriteLine("Compiler already set!");
                    error = true;
                }
                else
                {
                    arguments.Compiler = Compiler.Clang6_0;
                }
                break;
                case "msvc":
                if (arguments.Compiler.HasValue)
                {
                    Console.WriteLine("Compiler already set!");
                    error = true;
                }
                else
                {
                    arguments.Compiler = Compiler.MSVC19;
                }
                break;
                case "debug":
                if (arguments.DebugLevel.HasValue)
                {
                    Console.WriteLine("Debug level already set!");
                    error = true;
                }
                else
                {
                    arguments.DebugLevel = DebugLevel.Debug;
                }
                break;
                case "ndebug":
                if (arguments.DebugLevel.HasValue)
                {
                    Console.WriteLine("Debug level already set!");
                    error = true;
                }
                else
                {
                    arguments.DebugLevel = DebugLevel.NonDebug;
                }
                break;
                case "output":
                if (!string.IsNullOrEmpty(arguments.OutputFilename))
                {
                    Console.WriteLine("Output filename already set!");
                    error = true;
                }
                else
                {
                    ++i;
                    if (i>=arg_count || Args[i][0]=='-')
                    {
                        Console.WriteLine("Output filename not specified in output filename option!");
                        error = true;
                    }
                    else
                    {
                        arguments.OutputFilename = Args[i];
                    }
                }
                break;
                case "force":
                    arguments.ForceCompilation = true;
                    break;
                case "warnings_are_errors":
                    arguments.WarningsAreErrors = true;
                    break;
                default:
                    Console.WriteLine($"Unknown option \"{Args[i]}\"!");
                    error = true;
                    break;
            }
        }
        else
        {
            arguments.SourceFilenames.Add(Args[i]);
        }
    }
    return (!error, arguments);
}

bool AreArgumentValids(Arguments arguments)
{
    bool error = false;
    if (!arguments.Compiler.HasValue) { error = true; Console.WriteLine("No compiler specified! Please use -clang or -msvc"); }
    if (string.IsNullOrEmpty(arguments.OutputFilename)) { error = true; Console.WriteLine("No output filename specified! Please use -output <filepath>"); }
    if (arguments.SourceFilenames.Count==0) { error = true; Console.WriteLine("No source filename specified! You must specify at least one source filename."); }
    return !error;
}

void Main()
{
    var(arg_ok,args) = ParseArguments();
    arg_ok &= AreArgumentValids(args);
    if (!arg_ok)
    {
        Console.WriteLine("Early exit.");
        return;
    }

    var most_recent_source_file_time = DateTime.MinValue;
    bool error = false;
    foreach(var filename in args.SourceFilenames)
    {
        var sourceFileInfo = new FileInfo(filename);
        if (sourceFileInfo.Exists)
        {
            if (most_recent_source_file_time<sourceFileInfo.LastWriteTime)
                most_recent_source_file_time = sourceFileInfo.LastWriteTime;
        }
        else
        {
            Console.WriteLine($"Fail to find the source file \"{filename}\"!");
            error = true;
        }
    }
    if (error)
    {
        Console.WriteLine("Early exit.");
        return;
    }
    //Console.WriteLine("Most recent source file time: {0}", most_recent_source_file_time);

    if (!args.ForceCompilation)
    {
        var outputFileInfo = new FileInfo(args.OutputFilename);
        if (outputFileInfo.Exists)
        {
            //Console.WriteLine("Output file time: {0}", outputFileInfo.LastWriteTime);
            if (outputFileInfo.LastWriteTime>most_recent_source_file_time)
            {
                Console.WriteLine("No update needed.");
                return;
            }
        }
    }

    new FileInfo(args.OutputFilename).Directory.Create();

    ICompilo compilo;
    switch (args.Compiler)
    {
        case Compiler.Clang6_0:
            compilo = new Clang6_0();
            break;
        default:
            compilo = new MSVC();
            break;
    }

    var arguments = string.Join(" ",
        string.Join(" ", args.SourceFilenames),
        compilo.OutputFileFlag(args.OutputFilename),
        compilo.WarningLevelFlag(WarningLevel.High),
        compilo.ExtraFlags,
        compilo.LanguageFlag(LanguageVersion.Cpp17),
        compilo.DebugFlags(args.DebugLevel.HasValue&&args.DebugLevel == DebugLevel.Debug ? DebugLevel.Debug : DebugLevel.NonDebug)
        );
    if (args.WarningsAreErrors)
        arguments += " " + compilo.WarningAreErrorsFlag;

    Console.WriteLine(arguments);

    compilo.SetupEnvironment();

    ProcessStartInfo psi = new ProcessStartInfo();
    var process = new Process();
    process.StartInfo.FileName = compilo.ExecutableName;
    process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
    process.StartInfo.UseShellExecute = false;
    process.StartInfo.RedirectStandardOutput = true;
    process.StartInfo.RedirectStandardError = true;
    process.StartInfo.Arguments = arguments;
    process.Start();
    string output = process.StandardOutput.ReadToEnd();
    string errors = process.StandardError.ReadToEnd();
    process.WaitForExit();

    Console.WriteLine(output);
    Console.WriteLine(errors);

    Environment.Exit(process.ExitCode);
}

Main();