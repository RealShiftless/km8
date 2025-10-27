using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization
{
    public enum TokenType
    {
        StatementType,
        Operand,
        Terminator
    }

    public readonly struct Token(ITokenHandler handler, string value)
    {
        public readonly ITokenHandler Handler = handler;
        public readonly string Value = value;
    }
}
