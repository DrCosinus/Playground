// C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\Roslyn cssscrpt.csx

// arg0 = clang/msvc
// arg1 = debug/release
// arg2 = output_file
// arg3..n = source_files

enum WarningLevel
{
    None,
    Low,
    MediumLow,
    MediumHigh,
    High,
    Max
}

enum LanguageVersion
{
    Cpp11,
    Cpp14,
    Cpp17,
    Cpp20
}

enum DebugLevel
{
    Debug,
    NonDebug
}

interface ICompilo
{
    string ExecutableName { get; }
    string ExtraFlags { get; }
    string LanguageFlag(LanguageVersion _language_version);
    string OutputFileFlag(string _output_path);
    string WarningLevelFlag(WarningLevel _warning_level);
    string DebugFlags(DebugLevel _optimization);
    string DefineFlag(string _name, object _value);
}

abstract class Compilo : ICompilo
{
    public abstract string ExecutableName { get; }
    public abstract string ExtraFlags { get; }

    public string DebugFlags(DebugLevel _optimization)
    {
        return DefineFlag("DEBUG",(_optimization==DebugLevel.Debug)?1:0);
    }

    public abstract string DefineFlag(string _name, object _value);
    public abstract string LanguageFlag(LanguageVersion _language_version);
    public abstract string OutputFileFlag(string _output_path);
    public abstract string WarningLevelFlag(WarningLevel _warning_level);
}

class MSVC : Compilo
{
    public override string ExecutableName => "cl";

    public override string ExtraFlags => "/EHsc";

    public override string DefineFlag(string _name, object _value) => $"/D{_name}={_value}";
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
}

class Clang6_0 : Compilo
{
    public override string ExecutableName => "clang";

    public override string ExtraFlags => "-Xclang -flto-visibility-public-std";

    public override string DefineFlag(string _name, object _value) => $"-D{_name}={_value}";
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
}

Console.WriteLine( $"{Args.Count} arguments");
foreach(var arg in Args)
{
    Console.WriteLine( $"- \"{arg}\"");
}

var fi = new FileInfo("magic.cpp");
Console.WriteLine("Source: {0}", fi.LastWriteTime);

var fi2 = new System.IO.FileInfo("bin/windows/clang/release/magic.exe");
Console.WriteLine("Binary: {0}", fi2.LastWriteTime);

//ICompilo compilo = new MSVC();
ICompilo compilo = new Clang6_0();

string sourceName = "magic.cpp";
string outputName = "bin/windows/clang/release/magic.exe";
var arguments = string.Join(" ",
    sourceName,
    compilo.OutputFileFlag(outputName),
    compilo.WarningLevelFlag(WarningLevel.High),
    compilo.ExtraFlags,
    compilo.LanguageFlag(LanguageVersion.Cpp17),
    compilo.DebugFlags(DebugLevel.NonDebug)
    );
 Console.WriteLine(arguments);

var process = new Process();
process.StartInfo.FileName = compilo.ExecutableName;
process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
process.StartInfo.UseShellExecute = false;
process.StartInfo.RedirectStandardOutput = true;
process.StartInfo.Arguments = arguments;
process.Start();
string output = process.StandardOutput.ReadToEnd();
process.WaitForExit();

Console.WriteLine(output);