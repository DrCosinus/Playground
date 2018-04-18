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
    ECppVersion? CppVersion { set; }
    EWarningLevel? WarningLevel { set; }
    EDebugLevel? DebugLevel { set; }
    bool WarningAsErrors { set; }
    List<string> SourceFilePaths { set; }
    string OutputFilepath { set; }
    string IntermediaryFileFolderName { set; }

    int Run();
}

abstract class Compiler : ICompiler
{
    protected virtual void SetupEnvironmentVariables() {}
    protected abstract string ExecutableName { get; }
    public ECppVersion? CppVersion { protected get; set; }
    public string OutputFilepath { set; protected get; }
    public EWarningLevel? WarningLevel { set; protected get; }
    public EDebugLevel? DebugLevel { protected get; set; }
    public bool WarningAsErrors { set; protected get; }
    protected virtual string CompilationParameters { get; }
    public List<string> SourceFilePaths { set; protected get; }
    public string IntermediaryFileFolderName { set; protected get; }

    public static ICompiler Create(ECompiler compiler)
    {
        switch (compiler)
        {
            case ECompiler.Clang6_0: return new Clang6_0();
            default: return new MSVC();
        }
    }

    protected virtual void PrepareRun()
    {
        new FileInfo(OutputFilepath).Directory.Create();
    }
    public int Run()
    {
        PrepareRun();
        SetupEnvironmentVariables();

        var arguments = CompilationParameters;
        Console.WriteLine(arguments);

        var process = new Process();
        process.StartInfo.FileName = ExecutableName;
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

        return process.ExitCode;
    }
}

class MSVC : Compiler
{
    private static string MSVCPath => @"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.13.26128"; // should be deduced
    private static string WindowsKitPath => @"C:\Program Files (x86)\Windows Kits"; // should be deduced
    private static string Windows10KitPath(string _group) => $@"{WindowsKitPath}\10\{_group}\10.0.16299.0"; // should be deduced
    private static string DotNetFrameworkPath => $@"{WindowsKitPath}\NETFXSDK\4.6.1"; // should be deduced

    protected override void SetupEnvironmentVariables()
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
    protected override void PrepareRun()
    {
        base.PrepareRun();
        if (!string.IsNullOrEmpty(IntermediaryFileFolderName))
        {
            new DirectoryInfo(IntermediaryFileFolderName).Create();
        }
    }
    protected override string ExecutableName => "cl";
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
        parameters.AddRange(SourceFilePaths);
        parameters.Add($"/Fe{OutputFilepath}");
        parameters.Add("/EHsc"); // avoid warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
        parameters.Add("/permissive-"); // disable soms nonconforming code to compile
        parameters.Add("/Za"); // disable extensions
        parameters.Add("/nologo"); // disable copyright message
        if (!string.IsNullOrEmpty(IntermediaryFileFolderName))
        {
            parameters.Add($"/Fo{IntermediaryFileFolderName}/");
        }
        return string.Join(" ", parameters);
    }
    protected override string CompilationParameters
    {
        get => GenerateCompilationParametersString();
    }
}

class Clang6_0 : Compiler
{
    protected override string ExecutableName => "clang";
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
        parameters.AddRange(SourceFilePaths);
        parameters.Add($"-o {OutputFilepath}");
        parameters.Add("-Xclang -flto-visibility-public-std");
        return string.Join(" ", parameters);
    }

    protected override string CompilationParameters
    {
        get => GenerateCompilationParametersString();
    }
}

public struct WriteCounter<T>
{
    public bool HasValue { get => writing_count_>0; }
    public bool WasCrashed  { get => writing_count_>1; }
    public T Value { get => value_; set{ SetValue(value); } }
    public uint WritingCount => writing_count_;
    private uint writing_count_;
    private T value_;

    public T GetValueOrDefault(T _default_value) => HasValue?Value:_default_value;
    private void SetValue(T _value) { value_ = _value; writing_count_++; }
}

class Arguments
{
    public WriteCounter<ECompiler> Compiler;
    public WriteCounter<EDebugLevel> DebugLevel;
    public WriteCounter<string> OutputFilename;
    public List<string> SourceFilenames = new List<string>();
    public bool ForceCompilation = false;
    public bool WarningsAreErrors = false;
}

bool IsFlag(string arg)
{
    return arg[0]=='-' || arg[0]=='/';
}
(bool no_error, Arguments arguments) ParseArguments()
{
    bool error = false;
    Action<string> ErrorHandler = (message) => { Console.WriteLine(message); error=true; };
    Arguments arguments = new Arguments();
    var arg_count = Args.Count;
    for(var arg_index=0; arg_index < arg_count; ++arg_index)
    {
        if(IsFlag(Args[arg_index]))
        {
            string option = Args[arg_index].Substring(1);
            switch(option)
            {
                case "clang":   arguments.Compiler.Value = ECompiler.Clang6_0;          break;
                case "msvc":    arguments.Compiler.Value = ECompiler.MSVC19;            break;
                case "debug":   arguments.DebugLevel.Value = EDebugLevel.Debug;         break;
                case "ndebug":  arguments.DebugLevel.Value = EDebugLevel.NonDebug;      break;
                case "force":   arguments.ForceCompilation = true;                      break;
                case "warnings_are_errors":  arguments.WarningsAreErrors = true;        break;
                case "output":
                    ++arg_index;
                    if (arg_index>=arg_count || IsFlag(Args[arg_index]))
                        ErrorHandler("Output filename not specified in output filename option!");
                    else
                        arguments.OutputFilename.Value = Args[arg_index];
                    break;
                default:        ErrorHandler($"Unknown option \"{Args[arg_index]}\"!"); break;
            }
        }
        else
        {
            arguments.SourceFilenames.Add(Args[arg_index]);
        }
    }
    if (arguments.Compiler.WasCrashed)          ErrorHandler("Compiler defined multiple times!");
    if (arguments.DebugLevel.WasCrashed)        ErrorHandler("Debug level defined multiple times!");
    if (arguments.OutputFilename.WasCrashed)    ErrorHandler("Output filepath defined multiple times!");
    return (!error, arguments);
}

bool AreArgumentValids(Arguments arguments)
{
    bool error = false;
    if (!arguments.Compiler.HasValue) { error = true; Console.WriteLine("No compiler specified! Please use -clang or -msvc"); }
    if (!arguments.OutputFilename.HasValue) { error = true; Console.WriteLine("No output filename specified! Please use -output <filepath>"); }
    if (arguments.SourceFilenames.Count==0) { error = true; Console.WriteLine("No source filename specified! You must specify at least one source filename."); }
    return !error;
}

int Main()
{
    var(arg_ok,args) = ParseArguments();
    arg_ok &= AreArgumentValids(args);
    if (!arg_ok)
    {
        Console.WriteLine("Bad arguments!");
        return 1;
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
        Console.WriteLine("Bad source files!");
        return 1;
    }
    //Console.WriteLine("Most recent source file time: {0}", most_recent_source_file_time);

    if (!args.ForceCompilation)
    {
        var outputFileInfo = new FileInfo(args.OutputFilename.Value);
        if (outputFileInfo.Exists)
        {
            //Console.WriteLine("Output file time: {0}", outputFileInfo.LastWriteTime);
            if (outputFileInfo.LastWriteTime>most_recent_source_file_time)
            {
                Console.WriteLine("No update needed.");
                return 0;
            }
        }
    }

    var compilo = Compiler.Create(args.Compiler.GetValueOrDefault(ECompiler.Clang6_0));

    compilo.IntermediaryFileFolderName = "obj";
    compilo.CppVersion = ECppVersion.Cpp17;
    compilo.WarningLevel = EWarningLevel.High;
    compilo.DebugLevel = args.DebugLevel.Value;
    compilo.OutputFilepath = args.OutputFilename.Value;
    compilo.WarningAsErrors = args.WarningsAreErrors;
    compilo.SourceFilePaths = args.SourceFilenames;
    var exitCode = compilo.Run();

    return exitCode;
}

Environment.Exit(Main());