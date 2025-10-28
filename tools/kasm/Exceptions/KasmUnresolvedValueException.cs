using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Exceptions
{
    internal class KasmUnresolvedValueException(string msg) : Exception(msg)
    {
    }
}
