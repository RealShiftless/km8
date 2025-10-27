using kasm.Parsing;
using System.Text.RegularExpressions;

namespace kasm.Tokenization.TokenTypes;

public sealed class DirectiveTokenHandler : ITokenHandler
{
    // Properties
    public string TypeName => "Directive";

    TokenType ITokenHandler.Type => TokenType.StatementType;


    // Regex
    private static readonly Regex _parseRegex = new(@"^\.[A-Za-z][A-Za-z0-9_]*$", RegexOptions.Compiled);


    // Func
    public Token? TryParse(AssemblerContext context, string value)
    {
        if (!_parseRegex.IsMatch(value))
            return null;

        return new Token(this, value[1..]);
    }


    // Interface
    StatementType ITokenHandler.GetStatementType() => StatementType.Directive;
}
