using System;
using System.IO;
using Newtonsoft.Json;
using CommandLine;
using kasm.Tokenization;
using kasm;
using static Kasm.Program;
using kasm.Parsing;
using kasm.Symbols;

namespace Kasm;

class Program
{
    // Settings
    private static readonly JsonSerializerSettings _jsonOptions = new()
    {
        Formatting = Formatting.Indented,
        
    };


    // CLI Args
    public class Options
    {
        [Value(0, MetaName = "input", HelpText = "Input KM8 assembly file", Required = true)]
        public string Input { get; set; } = string.Empty;

        [Option('o', "output", HelpText = "Output binary file path")]
        public string? Output { get; set; }

        [Option('t', "target", HelpText = "ISA taget path")]
        public string? Target { get; set; }

        [Option('v', "verbose", HelpText = "Prints all inbetween steps")]
        public bool Verbose { get; set; } = false;
    }


    // Main
    static int Main(string[] args)
    {
        return CommandLine.Parser.Default
            .ParseArguments<Options>(args)
            .MapResult(
                (Options opts) => Run(opts),
                errs => 1);
    }

    

    private static int Run(Options opts)
    {
        // Gettings arguments
        string inputPath = Path.GetFullPath(opts.Input);
        string outputPath = string.IsNullOrWhiteSpace(opts.Output)
            ? Path.ChangeExtension(inputPath, ".bin") ?? (inputPath + ".bin")
            : Path.GetFullPath(opts.Output);

        string targetPath = string.IsNullOrWhiteSpace(opts.Target)
            ? "km8-isa-latest.json"
            : opts.Target;

        bool verbose = opts.Verbose;

        // Little print
        Console.WriteLine($"Assembling {inputPath} -> {outputPath}");

        // Context
        AssemblerContext context = new(inputPath, outputPath, targetPath);

        // Instruction set
        context.BindInstructionSet(LoadInstructionSet(targetPath));

        Console.WriteLine($"Using: {context.InstructionSet.Name} v{context.InstructionSet.Version}");

        context.SetSource(File.ReadAllText(inputPath));

        // Tokenization
        if (verbose)
            Console.WriteLine("Tokenizing source...");

        Tokenizer.Tokenize(context);

        if (verbose) 
            PrintTokens(context.Tokens);
        
        kasm.Parsing.Parser.Parse(context);

        if (verbose)
        {
            PrintStatements(context.Statements);
            PrintLabelNames(context.Symbols);
        }
            

        return 0;
    }


    // Helper
    private static InstructionSet LoadInstructionSet(string path)
    {
        string json = File.ReadAllText(path);

        return JsonConvert.DeserializeObject<InstructionSet>(json, _jsonOptions) ?? throw new FileLoadException("Target could not be loaded!");
    }

    private static void PrintTokens(TokenSequence tokens)
    {
        Console.WriteLine("\nTokens: ");
        foreach (Token token in tokens)
        {
            Console.WriteLine($"{token.Handler.TypeName} {token.Value}");
        }
    }
    private static void PrintStatements(StatementSequence statements)
    {
        Console.WriteLine("\nStatements: ");
        foreach(Statement statement in statements)
        {
            switch(statement.Type)
            {
                case StatementType.Instruction: Console.Write("ins: "); break;
                case StatementType.Directive:   Console.Write("dir: "); break;
                case StatementType.Label:       Console.Write("lbl: "); break;
            }

            Console.Write($"{statement.Name} ");

            foreach(Statement.Operand operand in statement.Operands)
            {
                string type = Enum.GetName(operand.Type) ?? "unknown";
                Console.Write($"{type.ToLower()}:{operand.Value} ");
            }

            Console.Write("\n");
        }
    }
    private static void PrintLabelNames(SymbolTable symbols)
    {
        Console.WriteLine("\nLabel Names: ");

        foreach (KeyValuePair<string, Symbol> kvp in symbols)
        {
            if (kvp.Value.Type == SymbolType.Address)
                Console.WriteLine(kvp.Key);
        }
    }
}
