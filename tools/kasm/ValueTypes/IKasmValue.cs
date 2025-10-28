using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.ValueTypes
{
    internal interface IKasmValue
    {
        OperandKind Kind { get; }

        object? GetValue();

        byte[] ToBytes();
    }
}
