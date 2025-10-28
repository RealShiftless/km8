using kasm.Parsing;
using kasm.Symbols;
using kasm.Tokenization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    internal sealed class AssemblerContext(KasmProject project)
    {
        // Values
        public readonly KasmProject Project = project;

        public readonly SymbolTable Symbols = new();

        public readonly StatementSequence Statements = new();

        public uint CurrentAddress;

        private InstructionSet? _instructionSet;


        // Properties
        public InstructionSet InstructionSet => _instructionSet ?? throw new InvalidOperationException("Instruction set was not bound!");
        

        // Func
        internal void BindInstructionSet(InstructionSet instructionSet) => _instructionSet = instructionSet;
    }
}
