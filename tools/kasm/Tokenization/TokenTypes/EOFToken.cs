using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    internal class EOFToken : ITokenType
    {
        public string TypeName => "EOF";

        public Token? TryParse(string value)
        {
            throw new NotImplementedException();
        }
    }
}
