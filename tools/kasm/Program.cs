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
    


    // CLI Args
    public class Options
    {
        [Value(0, MetaName = "input", HelpText = "Input KM8 json project file", Required = true)]
        public string Input { get; set; } = string.Empty;

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
        bool verbose = opts.Verbose;

        // Loading project json
        if (!File.Exists(inputPath))
        {
            Console.WriteLine("Could not find project file!");
            return 1;
        }

        string projectSource = File.ReadAllText(inputPath);
        KasmProject? project = JsonConvert.DeserializeObject<KasmProject>(projectSource);

        if(project == null)
        {
            Console.WriteLine("Could not serialize project file!");
            return 1;
        }

        // Little print
        Console.WriteLine($"Assembling {project.Name}");

        Directory.SetCurrentDirectory(Path.GetDirectoryName(inputPath) ?? "./");
        Assembler.Assemble(project, verbose);

        return 0;
    }
}
