using kasm.Tokenization.TokenTypes;
using System.Collections.Generic;
using System.Diagnostics;

namespace kasm.Tokenization;

public enum TokenType
{
    EndOfFile,

    // Identifiers / keywords
    Identifier,
    Directive,
    Register,
    Label,

    // Literals
    IntegerLiteral,
    AddressLiteral,
    StringLiteral,

    // Structure tokens
    NewLine,
}

public readonly struct Token(ITokenType type, string value)
{
    public readonly ITokenType Type = type;
    public readonly string Value = value;
}

public static class Tokenizer
{
    private static List<ITokenType> _tokenTypes = [];

    private static NewLineToken _newLineTokenType = new NewLineToken();
    private static EOFToken _eofTokenType = new EOFToken();

    static Tokenizer()
    {
        _tokenTypes.Add(new DirectiveToken());
        _tokenTypes.Add(new InstructionToken());
        _tokenTypes.Add(new LabelToken());
        _tokenTypes.Add(new RegisterToken());
        _tokenTypes.Add(new IdentifierToken());

        _tokenTypes.Add(new IntegerLiteralToken());
        _tokenTypes.Add(new AddressLiteralToken());
    }

    private static Token? TryParseToken(string value)
    {
        Token? token = null;
        foreach (ITokenType tokenType in _tokenTypes)
        {
            token = tokenType.TryParse(value);

            if (token != null)
                break;
        }

        return token;
    }

    public static List<Token> Tokenize(string str)
    {
        List<Token> tokens = [];

        str = str.Replace(',', ' ');
        str = str.Replace("\r", null);

        int lineCounter = 0;
        int argCounter = 0;
        foreach(string line in str.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            argCounter = 0;
            foreach (string tokenStr in line.Split(' ', StringSplitOptions.RemoveEmptyEntries))
            {
                Token token = TryParseToken(tokenStr) ?? throw new InvalidOperationException($"Could not parse token! (l: {lineCounter}, a: {argCounter})");
                tokens.Add(token);

                argCounter++;
            }

            tokens.Add(new Token(_newLineTokenType, ""));

            lineCounter++;
        }

        tokens.Add(new Token(_eofTokenType, ""));

        return tokens;
    }
}
