using kasm.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    internal class EOFToken : ITokenHandler
    {
        public string TypeName => "EOF";

        TokenType ITokenHandler.Type => TokenType.Terminator;

        public Token? TryParse(AssemblerContext context, string value)
        {
            throw new NotImplementedException();
        }
    }
}
