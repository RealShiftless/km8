using System.Text.RegularExpressions;

namespace kasm.Tokenization.TokenTypes;

public sealed class IdentifierToken : ITokenType
{
    private static readonly Regex _parseRegex = new(@"^[A-Za-z_][A-Za-z0-9_]*$", RegexOptions.Compiled);

    public string TypeName => "Identifier";

    public Token? TryParse(string value)
    {
        if (!_parseRegex.IsMatch(value))
            return null;

        return new Token(this, value);
    }
}
