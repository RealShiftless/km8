using System.Collections.Generic;
using System.Globalization;

namespace kasm.Tokenization.TokenTypes;

public sealed class RegisterToken : ITokenType
{
    private static readonly Dictionary<string, int> s_registerMap = new(StringComparer.OrdinalIgnoreCase)
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

    public string TypeName => "Register";

    public Token? TryParse(string value)
    {
        if (!s_registerMap.TryGetValue(value, out int index))
            return null;

        return new Token(this, index.ToString(CultureInfo.InvariantCulture));
    }
}
