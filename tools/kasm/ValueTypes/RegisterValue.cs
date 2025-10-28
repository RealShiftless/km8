using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.ValueTypes
{
    public enum Register
    {
        Reg0 = 0,
        Reg1,
        Reg2,
        Reg3,
        Reg4,
        Reg5,
        Reg6,
        Reg7,
        Reg8,
        Reg9,
        Reg10,
        Reg11,
        SP = 7,
        PC = 9,
        Flag = 11
    }

    internal class RegisterValue(Register value) : IKasmValue
    {
        // Properties
        public Register Value { get; set; } = value;

        public OperandKind Kind => throw new NotImplementedException();


        // Interface
        object? IKasmValue.GetValue() => Value;
        public byte[] ToBytes() => [(byte)Value];


        // Overrides
        public override string ToString() => $"r{Value}";
    }
}
