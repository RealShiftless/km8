using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    public sealed class IntegerLiteralToken : ITokenType
    {
        public string TypeName => "Integer Literal";

        private static readonly Regex _decRegex = new(@"^-?[0-9]*$", RegexOptions.Compiled);
        private static readonly Regex _hexRegex = new(@"^-?0x[A-Fa-f0-9]*$", RegexOptions.Compiled);
        private static readonly Regex _binRegex = new(@"^-?0b[01]*$", RegexOptions.Compiled);

        public Token? TryParse(string value)
        {
            if(_decRegex.IsMatch(value))
                return new Token(this, value);

            if(_hexRegex.IsMatch(value))
                return new Token(this, "" + Convert.ToInt32(value[2..], 16));

            if (_binRegex.IsMatch(value))
                return new Token(this, "" + Convert.ToInt32(value[2..], 2));

            return null;
        }
    }
}
