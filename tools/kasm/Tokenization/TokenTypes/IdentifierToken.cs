using kasm.Parsing;
using System.Text.RegularExpressions;

namespace kasm.Tokenization.TokenTypes;

public sealed class IdentifierToken : ITokenHandler
{
    private static readonly Regex _parseRegex = new(@"^[A-Za-z_][A-Za-z0-9_]*$", RegexOptions.Compiled);
    

    // Properties
    public string TypeName => "Identifier";

    TokenType ITokenHandler.Type => TokenType.Operand;


    // Regex


    // Func
    public Token? TryParse(AssemblerContext context, string value)
    {
        if (!_parseRegex.IsMatch(value))
            return null;

        return new Token(this, value);
    }


    // Interface
    OperandType ITokenHandler.GetOperandType() => OperandType.Identifier;
}
