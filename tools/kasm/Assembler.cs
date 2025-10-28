using kasm.Parsing;
using kasm.Symbols;
using kasm.Tokenization;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    public static class Assembler
    {
        // Settings
        private static readonly JsonSerializerSettings _jsonOptions = new()
        {
            Formatting = Formatting.Indented,

        };


        // Func
        public static bool Assemble(KasmProject project, bool verbose)
        {
            // Context
            AssemblerContext context = new(project);

            // Instruction set
            InstructionSet instructionSet = LoadInstructionSet(context.Project.InstructionSet);
            context.BindInstructionSet(instructionSet);

            Console.WriteLine($"Using: {context.InstructionSet.Name} v{context.InstructionSet.Version}");


            // Validate paths
            foreach(string path in project.SourceFiles)
            {
                if (!File.Exists(path))
                {
                    Console.WriteLine($"Could not find {path}!");
                    return false;
                }
            }

            // From here we go thru each source file to build the IL
            foreach (string path in project.SourceFiles)
            {
                if (verbose)
                    Console.WriteLine($"\nBuilding {path}...");

                string source = File.ReadAllText(path);

                // Tokenization
                if (verbose)
                    Console.WriteLine("| Tokenizing...");

                TokenSequence tokenSequence = Tokenizer.Tokenize(context, source);

                if (verbose)
                    PrintTokens(tokenSequence);

                // Parsing
                if (verbose)
                    Console.WriteLine("| Parsing...");

                Parser.Parse(context, tokenSequence);
            }

            // Print the IL
            if (verbose)
            {
                PrintStatements(context.Statements);
                PrintLabelNames(context.Symbols);
            }

            return true;
        }

        // Helper
        private static InstructionSet LoadInstructionSet(string path)
        {
            string json = File.ReadAllText(path);

            return JsonConvert.DeserializeObject<InstructionSet>(json, _jsonOptions) ?? throw new FileLoadException("Instruction set could not be loaded!");
        }

        private static void PrintTokens(TokenSequence tokens)
        {
            foreach (Token token in tokens)
            {
                Console.WriteLine($"| | {token.Handler.TypeName} {token.Value}");
            }
        }
        private static void PrintStatements(StatementSequence statements)
        {
            Console.WriteLine("\nStatements: ");
            foreach (Statement statement in statements)
            {
                switch (statement.Type)
                {
                    case StatementType.Instruction: Console.Write("ins: "); break;
                    case StatementType.Directive: Console.Write("dir: "); break;
                    case StatementType.Label: Console.Write("lbl: "); break;
                }

                Console.Write($"{statement.Name} ");

                foreach (Statement.Operand operand in statement.Operands)
                {
                    string type = Enum.GetName(operand.Type) ?? "unknown";
                    Console.Write($"{type.ToLower()}:{operand.ValueString} ");
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
}
