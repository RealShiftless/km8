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
    public sealed class AssemblerContext(string input, string output, string instructionSet)
    {
        // Values
        public readonly string InputPath = input;
        public readonly string OutputPath = output;

        public readonly string InstructionSetPath = instructionSet;

        public readonly SymbolTable Symbols = new();

        private InstructionSet? _instructionSet;
        private TokenSequence? _tokens;
        private StatementSequence? _statements;


        // Properties
        public string Source { get; private set; } = string.Empty;

        public InstructionSet InstructionSet => _instructionSet ?? throw new InvalidOperationException("Instruction set was not bound!");
        public TokenSequence Tokens => _tokens ?? throw new InvalidOperationException("Token sequence was not bound!");
        public StatementSequence Statements => _statements ?? throw new InvalidOperationException("Statement sequence was not bound!");
        

        // Func
        internal void SetSource(string source) => Source = source;

        internal void BindInstructionSet(InstructionSet instructionSet) => _instructionSet = instructionSet;
        internal void BindTokenSequence(TokenSequence tokenSequence) => _tokens = tokenSequence;
        internal void BindStatementSequence(StatementSequence statementSequence) => _statements = statementSequence;
    }
}
