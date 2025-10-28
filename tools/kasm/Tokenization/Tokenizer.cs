using kasm.Tokenization.TokenTypes;
using System.Collections.Generic;
using System.Diagnostics;

namespace kasm.Tokenization;



public static class Tokenizer
{
    private static List<ITokenHandler> _tokenTypes = [];

    private static NewLineToken _newLineTokenType = new NewLineToken();
    private static EOFToken _eofTokenType = new EOFToken();

    static Tokenizer()
    {
        _tokenTypes.Add(new DirectiveTokenHandler());
        _tokenTypes.Add(new InstructionToken());
        _tokenTypes.Add(new LabelToken());
        _tokenTypes.Add(new RegisterToken());
        _tokenTypes.Add(new IdentifierToken());

        _tokenTypes.Add(new IntegerLiteralToken());
        _tokenTypes.Add(new AddressLiteralTokenHandler());
    }

    private static Token? TryParseToken(AssemblerContext context, string value)
    {
        Token? token = null;
        foreach (ITokenHandler tokenType in _tokenTypes)
        {
            token = tokenType.TryParse(context, value);

            if (token != null)
                break;
        }

        return token;
    }

    internal static TokenSequence Tokenize(AssemblerContext context, string source)
    {
        TokenSequenceBuilder tokens = new();

        source = source.Replace("\t", string.Empty);
        source = source.Replace("\r", string.Empty);
        source = source.Replace(',', ' ');

        int lineCounter = 0;
        foreach(string line in source.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            int argCounter = 0;
            foreach (string tokenStr in line.Split(' ', StringSplitOptions.RemoveEmptyEntries))
            {
                Token token = TryParseToken(context, tokenStr) ?? throw new InvalidOperationException($"Could not parse token! (l: {lineCounter}, a: {argCounter})");

                tokens.Add(token);

                argCounter++;
            }

            tokens.Add(new Token(_newLineTokenType, ""));

            lineCounter++;
        }

        tokens.Add(new Token(_eofTokenType, ""));

        return tokens.Build();
    }
}
