using System;
using System.IO;
using System.Text.Json.Serialization;
using System.Text.Json;
using CommandLine;
using kasm.Tokenization;
using kasm;
using static Kasm.Program;

namespace Kasm;

class Program
{
    // Settings
    private static readonly JsonSerializerOptions _jsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
        Converters = { new JsonStringEnumConverter() }
    };

    private static InstructionSet? _instructionSet;
    public static InstructionSet InstructionSet => _instructionSet ?? throw new InvalidOperationException("Instruction set was not initialized!");


    // CLI Args
    public class Options
    {
        [Value(0, MetaName = "input", HelpText = "Input KM8 assembly file", Required = true)]
        public string Input { get; set; } = string.Empty;

        [Option('o', "output", HelpText = "Output binary file path")]
        public string? Output { get; set; }

        [Option('t', "target", HelpText = "ISA taget path")]
        public string? Target { get; set; }
    }


    // Main
    static int Main(string[] args)
    {
        return Parser.Default
            .ParseArguments<Options>(args)
            .MapResult(
                (Options opts) => Run(opts),
                errs => 1);
    }

    

    private static int Run(Options opts)
    {

        string inputPath = Path.GetFullPath(opts.Input);
        string outputPath = string.IsNullOrWhiteSpace(opts.Output)
            ? Path.ChangeExtension(inputPath, ".bin") ?? (inputPath + ".bin")
            : Path.GetFullPath(opts.Output);

        string targetPath = string.IsNullOrWhiteSpace(opts.Target)
            ? "km8-isa-latest.json"
            : opts.Target;

        _instructionSet = LoadInstructionSet(targetPath);

        Console.WriteLine($"Assembling {inputPath} -> {outputPath}");
        // TODO: add assembler logic here

        string source = File.ReadAllText(inputPath);
        List<Token> tokens = Tokenizer.Tokenize(source);

        Console.WriteLine("Tokens: ");
        foreach(Token token in tokens)
        {
            Console.WriteLine($"{token.Type.TypeName} {token.Value}");
        }

        return 0;
    }
    // Helper
    private static InstructionSet LoadInstructionSet(string path)
    {
        string json = File.ReadAllText(path);
        return JsonSerializer.Deserialize<InstructionSet>(json, _jsonOptions) ?? throw new FileLoadException("Could not load instruction set!");
    }
}
