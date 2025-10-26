using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    public sealed class LabelToken : ITokenType
    {
        private static readonly Regex _parseRegex = new(@"^[A-Za-z_][A-Za-z0-9_]*:$", RegexOptions.Compiled);

        public string TypeName => "Label";

        public Token? TryParse(string value)
        {
            if (!_parseRegex.IsMatch(value))
                return null;

            return new Token(this, value[..^1]);
        }
    }
}
