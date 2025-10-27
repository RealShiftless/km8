using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Exceptions
{
    public class KasmInternalException : Exception
    {
        public KasmInternalException(string message) : base(message) { }
    }
}
