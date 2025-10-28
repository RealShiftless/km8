using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Exceptions
{
    public class KasmSyntaxException(string message, uint line) : Exception
    {
        private string _message = message;
        public uint Line = line;

        public override string Message => $"{_message} (at l: {Line + 1})";
    }
}
