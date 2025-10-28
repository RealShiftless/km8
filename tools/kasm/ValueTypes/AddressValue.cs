using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.ValueTypes
{
    internal class AddressValue(ushort? value) : IKasmValue
    {
        // Values
        public ushort? Value = value;


        // Properties
        OperandKind IKasmValue.Kind => OperandKind.Address;


        // Interface
        object? IKasmValue.GetValue() => Value;
        byte[] IKasmValue.ToBytes()
        {
            if (Value == null)
                return [];

            return [(byte)Value, (byte)(Value >> 8)];
        }


        // Overrides
        public override string ToString() => $"$0x{Convert.ToString(Value, 16)}";
    }
}
