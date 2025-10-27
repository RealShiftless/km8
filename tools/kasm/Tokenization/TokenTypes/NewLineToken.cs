using kasm.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    internal class NewLineToken : ITokenHandler
    {
        public string TypeName => "New Line";

        TokenType ITokenHandler.Type => TokenType.Terminator;

        public Token? TryParse(AssemblerContext context, string value)
        {
            if(value != "\n")
                return null;

            return new Token(this, "");
        }
    }
}
