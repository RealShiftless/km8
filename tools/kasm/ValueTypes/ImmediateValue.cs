using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.ValueTypes
{
    internal class ImmediateValue(byte? value) : IKasmValue
    {
        // Properties
        public byte? Value { get; set; } = value;

        OperandKind IKasmValue.Kind => OperandKind.Immediate;


        // Interface
        object? IKasmValue.GetValue() => Value;
        byte[] IKasmValue.ToBytes()
        {
            if (Value == null)
                return [];

            return [Value.Value];
        }


        // Overrides
        public override string ToString() => $"0x{Convert.ToString(Value ?? 0, 16)}";
    }
}
