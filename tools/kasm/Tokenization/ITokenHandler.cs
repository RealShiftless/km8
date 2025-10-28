using kasm.Parsing;
using kasm.Symbols;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization
{
    internal interface ITokenHandler
    {
        string TypeName { get; }

        TokenType Type { get; }
        
        Token? TryParse(AssemblerContext context, string value);

        StatementType GetStatementType() => StatementType.Unkown;
        OperandType GetOperandType() => OperandType.Unkown;
        SymbolType GetSymbolType() => SymbolType.Unknown;
    }
}
