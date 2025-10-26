using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization
{
    public interface ITokenType
    {
        string TypeName { get; }

        Token? TryParse(string value);
    }
}
