using kasm.Parsing;
using kasm.Symbols;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    public sealed class IntegerLiteralToken : ITokenHandler
    {
        // Properties
        public string TypeName => "Integer Literal";

        TokenType ITokenHandler.Type => TokenType.Operand;


        // Regex
        private static readonly Regex _decRegex = new(@"^-?[0-9]*$", RegexOptions.Compiled);
        private static readonly Regex _hexRegex = new(@"^0x[A-Fa-f0-9]*$", RegexOptions.Compiled);
        private static readonly Regex _binRegex = new(@"^0b[01]*$", RegexOptions.Compiled);


        // Func
        public Token? TryParse(AssemblerContext context, string value)
        {
            if(_decRegex.IsMatch(value))
                return new Token(this, value);

            if(_hexRegex.IsMatch(value))
                return new Token(this, "" + Convert.ToInt32(value, 16));

            if (_binRegex.IsMatch(value))
                return new Token(this, "" + Convert.ToInt32(value, 2));

            return null;
        }


        // Interface
        OperandType ITokenHandler.GetOperandType() => OperandType.Immediate;
        SymbolType ITokenHandler.GetSymbolType() => SymbolType.Integer;
    }
}
