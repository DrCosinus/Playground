// -clang -msvc
// -debug -ndebug
// -output <output_filepath>
// -force
// -warnings_are_errors

enum ECompiler { Clang6_0, MSVC19 };
enum EWarningLevel { None, Low, MediumLow, MediumHigh, High, Max }
enum ECppVersion { Cpp11, Cpp14, Cpp17, Cpp20 }
enum EDebugLevel { Debug, NonDebug }

interface ICompiler
{
    void SetupEnvironmentVariables();
    string ExecutableName { get; }
    string ExtraOptionsString { get; }
    string GetCppVersionString(ECppVersion _language_version);
    string GetOutputFileString(string _output_path);
    string GetWarningLevelString(EWarningLevel _warning_level);
    string GetOptimizationString(EDebugLevel _optimization);
    string GetDefineString(string _name, object _value);
    string WarningAsErrorsString { get; }
}

abstract class Compiler : ICompiler
{
    public abstract void SetupEnvironmentVariables();
    public abstract string ExecutableName { get; }
    public abstract string ExtraOptionsString { get; }
    public abstract string GetCppVersionString(ECppVersion _language_version);
    public abstract string GetOutputFileString(string _output_path);
    public abstract string GetWarningLevelString(EWarningLevel _warning_level);
    public virtual string GetOptimizationString(EDebugLevel _optimization)
    {
        if (_optimization==EDebugLevel.Debug)
            return GetDefineString("DEBUG",1);
        else
            return string.Join(" ", GetDefineString("DEBUG",0), GetDefineString("NDEBUG",null));
    }
    public abstract string GetDefineString(string _name, object _value);
    public abstract string WarningAsErrorsString { get; }
}

class MSVC : Compiler
{
    private static string MSVCPath => @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128"; // should be deduced
    private static string WindowsKitPath => @"C:\Program Files (x86)\Windows Kits"; // should be deduced
    private static string Windows10KitPath(string _group) => $@"{WindowsKitPath}\10\{_group}\10.0.16299.0"; // should be deduced
    private static string DotNetFrameworkPath => $@"{WindowsKitPath}\NETFXSDK\4.6.1"; // should be deduced

    public override void SetupEnvironmentVariables()
    {
        Environment.SetEnvironmentVariable( "PATH",
            $@"{MSVCPath}\bin\HostX64\x64;{Environment.GetEnvironmentVariable("PATH")}");
        Environment.SetEnvironmentVariable( "INCLUDE", string.Join(";"
            //, $@"{MSVCPath}\ATLMFC\include"
            , $@"{MSVCPath}\include"
            //, $@"{DotNetFrameworkPath}\include\um"
            , $@"{Windows10KitPath("include")}\ucrt"
            //, $@"{Windows10KitPath}\shared"
            //, $@"{Windows10KitPath}\um"
            //, $@"{Windows10KitPath}\winrt"
            //, $@"{Windows10KitPath}\cppwinrt"
            ));
        Environment.SetEnvironmentVariable( "LIB", string.Join(";"
            //, $@"{MSVCPath}\ATLMFC\lib\x64"
            , $@"{MSVCPath}\lib\x64"
            //, $@"{DotNetFrameworkPath}\lib\um\x64"
            , $@"{Windows10KitPath("lib")}\ucrt\x64"
            , $@"{Windows10KitPath("lib")}\um\x64"
            ));
        // Environment.SetEnvironmentVariable( "LIBPATH", string.Join(";"
        //     , $@"{MSVCPath}\ATLMFC\lib\x64"
        //     , $@"{MSVCPath}\lib\x64"
        //     , $@"{MSVCPath}\lib\x86\store\references"
        //     , $@"{Windows10KitPath("UnionMetadata")}"
        //     , $@"{Windows10KitPath("References")}"
        //     , @"C:\Windows\Microsoft.NET\Framework64\v4.0.30319"
        //     ));
    }
    public override string ExecutableName => "cl";
    public override string ExtraOptionsString => "/EHsc";
    public override string GetCppVersionString(ECppVersion _language_version)
    {
        switch (_language_version)
        {
            case ECppVersion.Cpp11: return "/std:c++11";
            case ECppVersion.Cpp14: return "/std:c++14";
            case ECppVersion.Cpp17: return "/std:c++17";
            case ECppVersion.Cpp20: return "/std:c++latest";
            default: goto case ECppVersion.Cpp17;
        }
    }
    public override string GetOutputFileString(string _output_path) => $"/Fe{_output_path}";
    public override string GetWarningLevelString(EWarningLevel _warning_level)
    {
        switch(_warning_level)
        {
            case EWarningLevel.None: return "/W0";
            case EWarningLevel.Low: return "/W1";
            case EWarningLevel.MediumLow: return "/W2";
            case EWarningLevel.MediumHigh: return "/W3";
            case EWarningLevel.High: return "/W4";
            case EWarningLevel.Max: return "/Wall";
            default: goto case EWarningLevel.Low;
        }
    }
    public override string GetOptimizationString(EDebugLevel _optimization)
    {
        if (_optimization==EDebugLevel.Debug)
            return string.Join(" ", base.GetOptimizationString(_optimization), "/Zi", "/Od");
        else
            return string.Join(" ", base.GetOptimizationString(_optimization), "/Ox");
    }
    public override string GetDefineString(string _name, object _value) => $"/D{_name}={_value}";
    public override string WarningAsErrorsString => "/WX";
}

class Clang6_0 : Compiler
{
    public override void SetupEnvironmentVariables() {}
    public override string ExecutableName => "clang";
    public override string ExtraOptionsString => "-Xclang -flto-visibility-public-std";
    public override string GetCppVersionString(ECppVersion _language_version)
    {
        switch (_language_version)
        {
            case ECppVersion.Cpp11: return "-std=c++11";
            case ECppVersion.Cpp14: return "-std=c++14";
            case ECppVersion.Cpp17: return "-std=c++17";
            case ECppVersion.Cpp20: return "-std=c++2a";
            default: goto case ECppVersion.Cpp17;
        }
    }
    public override string GetOutputFileString(string _output_path) => $"-o {_output_path}";
    public override string GetWarningLevelString(EWarningLevel _warning_level)
    {
        switch(_warning_level)
        {
            case EWarningLevel.None: return "";
            case EWarningLevel.Low: return "-Wall";
            case EWarningLevel.MediumLow: return "-Wall -pedantic";
            case EWarningLevel.MediumHigh: return "-Wall -pedantic";
            case EWarningLevel.High: return "-Wall -pedantic -Wextra";
            case EWarningLevel.Max: return "-Wall -pedantic -Wextra";
            default: goto case EWarningLevel.Low;
        }
    }
    public override string GetOptimizationString(EDebugLevel _optimization)
    {
        if (_optimization==EDebugLevel.Debug)
            return string.Join(" ", base.GetOptimizationString(_optimization), "-O0");
        else
            return string.Join(" ", base.GetOptimizationString(_optimization), "-O3");
    }
    public override string GetDefineString(string _name, object _value) => $"-D{_name}={_value}";
    public override string WarningAsErrorsString => "-Werror";
}

class Arguments
{
    public ECompiler? Compiler;
    public EDebugLevel? DebugLevel;
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
                    arguments.Compiler = ECompiler.Clang6_0;
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
                    arguments.Compiler = ECompiler.MSVC19;
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
                    arguments.DebugLevel = EDebugLevel.Debug;
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
                    arguments.DebugLevel = EDebugLevel.NonDebug;
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

    ICompiler compilo;
    switch (args.Compiler)
    {
        case ECompiler.Clang6_0:
            compilo = new Clang6_0();
            break;
        default:
            compilo = new MSVC();
            break;
    }

    var arguments = string.Join(" ",
        string.Join(" ", args.SourceFilenames),
        compilo.GetOutputFileString(args.OutputFilename),
        compilo.GetWarningLevelString(EWarningLevel.High),
        compilo.ExtraOptionsString,
        compilo.GetCppVersionString(ECppVersion.Cpp17),
        compilo.GetOptimizationString(args.DebugLevel.HasValue&&args.DebugLevel == EDebugLevel.Debug ? EDebugLevel.Debug : EDebugLevel.NonDebug)
        );
    if (args.WarningsAreErrors)
        arguments += " " + compilo.WarningAsErrorsString;

    Console.WriteLine(arguments);

    compilo.SetupEnvironmentVariables();

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