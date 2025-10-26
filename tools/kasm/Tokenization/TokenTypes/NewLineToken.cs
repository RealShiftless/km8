using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    internal class NewLineToken : ITokenType
    {
        public string TypeName => "New Line";

        public Token? TryParse(string value)
        {
            if(value != "\n")
                return null;

            return new Token(this, "");
        }
    }
}
