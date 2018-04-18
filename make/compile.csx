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
    ECppVersion? CppVersion { set; }
    string OutputFilepath { set; }
    EWarningLevel? WarningLevel { set; }
    string CompilationParameters { get; }
    EDebugLevel? DebugLevel { set; }
    bool WarningAsErrors { set; }
}

abstract class Compiler : ICompiler
{
    public abstract void SetupEnvironmentVariables();
    public abstract string ExecutableName { get; }
    public ECppVersion? CppVersion { protected get; set; }
    public string OutputFilepath { set; protected get; }
    public EWarningLevel? WarningLevel { set; protected get; }
    public EDebugLevel? DebugLevel { protected get; set; }
    public bool WarningAsErrors { set; protected get; }
    public abstract string CompilationParameters { get; }
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
        new DirectoryInfo("tmp").Create();
    }
    public override string ExecutableName => "cl";
    private string GenerateCompilationParametersString()
    {
        List<string> parameters = new List<string>();
        if (CppVersion.HasValue)
        {
            switch (CppVersion)
            {
                case ECppVersion.Cpp11: parameters.Add("/std:c++11"); break;
                case ECppVersion.Cpp14: parameters.Add("/std:c++14"); break;
                case ECppVersion.Cpp17: parameters.Add("/std:c++17"); break;
                case ECppVersion.Cpp20: parameters.Add("/std:c++latest"); break;
                default: goto case ECppVersion.Cpp17;
            }
        }
        switch(WarningLevel.GetValueOrDefault(EWarningLevel.High))
        {
            case EWarningLevel.None: parameters.Add("/W0"); break;
            case EWarningLevel.Low: parameters.Add("/W1"); break;
            case EWarningLevel.MediumLow: parameters.Add("/W2"); break;
            case EWarningLevel.MediumHigh: parameters.Add("/W3"); break;
            case EWarningLevel.High: parameters.Add("/W4"); break;
            case EWarningLevel.Max: parameters.Add("/Wall"); break;
            default: goto case EWarningLevel.Low;
        }
        switch(DebugLevel.GetValueOrDefault(EDebugLevel.NonDebug))
        {
            case EDebugLevel.Debug:
            parameters.Add("/DDEBUG=1");
            parameters.Add("/Zi");
            parameters.Add("/Od");
            break;
            case EDebugLevel.NonDebug:
            parameters.Add("/DDEBUG=0");
            parameters.Add("/DNDEBUG");
            parameters.Add("/Ox");
            break;
        }
        if(WarningAsErrors)
        {
            parameters.Add("/WX");
        }
        parameters.Add($"/Fe{OutputFilepath}");
        parameters.Add("/EHsc");
        parameters.Add("/Fotmp/");
        return string.Join(" ", parameters);
    }
    public override string CompilationParameters
    {
        get => GenerateCompilationParametersString();
    }
}

class Clang6_0 : Compiler
{
    public override void SetupEnvironmentVariables() {}
    public override string ExecutableName => "clang";
    private string GenerateCompilationParametersString()
    {
        List<string> parameters = new List<string>();
        if (CppVersion.HasValue)
        {
            switch (CppVersion)
            {
                case ECppVersion.Cpp11: parameters.Add("-std=c++11"); break;
                case ECppVersion.Cpp14: parameters.Add("-std=c++14"); break;
                case ECppVersion.Cpp17: parameters.Add("-std=c++17"); break;
                case ECppVersion.Cpp20: parameters.Add("-std=c++2a"); break;
                default: goto case ECppVersion.Cpp17;
            }
        }
        switch(WarningLevel.GetValueOrDefault(EWarningLevel.High))
        {
            case EWarningLevel.None: break;
            case EWarningLevel.Low: parameters.Add("-Wall"); break;
            case EWarningLevel.MediumLow: parameters.Add("-Wall -pedantic"); break;
            case EWarningLevel.MediumHigh: parameters.Add("-Wall -pedantic"); break;
            case EWarningLevel.High: parameters.Add("-Wall -pedantic -Wextra"); break;
            case EWarningLevel.Max: parameters.Add("-Wall -pedantic -Wextra"); break;
            default: goto case EWarningLevel.Low;
        }
        switch(DebugLevel.GetValueOrDefault(EDebugLevel.NonDebug))
        {
            case EDebugLevel.Debug:
            parameters.Add("-DDEBUG=1");
            parameters.Add("-O0");
            break;
            case EDebugLevel.NonDebug:
            parameters.Add("-DDEBUG=0");
            parameters.Add("-DNDEBUG");
            parameters.Add("-O3");
            break;
        }
        if(WarningAsErrors)
        {
            parameters.Add("-Werror");
        }
        parameters.Add($"-o {OutputFilepath}");
        parameters.Add("-Xclang -flto-visibility-public-std");
        return string.Join(" ", parameters);
    }

    public override string CompilationParameters
    {
        get => GenerateCompilationParametersString();
    }
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

    compilo.CppVersion = ECppVersion.Cpp17;
    compilo.WarningLevel = EWarningLevel.High;
    compilo.OutputFilepath = args.OutputFilename;
    compilo.WarningAsErrors = args.WarningsAreErrors;
    var arguments = string.Join(" ",
        string.Join(" ", args.SourceFilenames),
        compilo.CompilationParameters
        );

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