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
    internal sealed class LabelToken : ITokenHandler
    {
        // Properties
        public string TypeName => "Label";

        TokenType ITokenHandler.Type => TokenType.StatementType;


        // Regex
        private static readonly Regex _parseRegex = new(@"^[A-Za-z_][A-Za-z0-9_]*:$", RegexOptions.Compiled);


        // Parse
        public Token? TryParse(AssemblerContext context, string value)
        {
            if (!_parseRegex.IsMatch(value))
                return null;

            return new Token(this, value[..^1]);
        }


        // Interface
        StatementType ITokenHandler.GetStatementType() => StatementType.Label;
        SymbolType ITokenHandler.GetSymbolType() => SymbolType.Address;
    }
}
