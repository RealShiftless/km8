using kasm.Parsing;
using Kasm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    public sealed class InstructionToken : ITokenHandler
    {
        // Properties
        public string TypeName => "Instruction";

        TokenType ITokenHandler.Type => TokenType.StatementType;


        // Func
        public Token? TryParse(AssemblerContext context, string value) => context.InstructionSet.Contains(value) ? new Token(this, value) : null;


        // Interface
        StatementType ITokenHandler.GetStatementType() => StatementType.Instruction;
    }
}
