using kasm.Parsing;
using System.Collections.Generic;
using System.Globalization;

namespace kasm.Tokenization.TokenTypes;

internal sealed class RegisterToken : ITokenHandler
{
    // Values
    private static readonly Dictionary<string, int> _registerMap = new(StringComparer.OrdinalIgnoreCase)
    {
        ["r0"] = 0,
        ["r1"] = 1,
        ["r2"] = 2,
        ["r3"] = 3,
        ["r4"] = 4,
        ["r5"] = 5,
        ["r6"] = 6,
        ["r7"] = 7,
        ["r8"] = 8,
        ["r9"] = 9,
        ["r10"] = 10,
        ["r11"] = 11,
        ["sp"] = 7,
        ["pc"] = 9,
        ["flag"] = 11,
    };


    // Properties
    public string TypeName => "Register";

    TokenType ITokenHandler.Type => TokenType.Operand;


    // Func
    public Token? TryParse(AssemblerContext context, string value)
    {
        if (!_registerMap.TryGetValue(value, out int index))
            return null;

        return new Token(this, index.ToString(CultureInfo.InvariantCulture));
    }


    // Interface
    OperandType ITokenHandler.GetOperandType() => OperandType.Register;
}
