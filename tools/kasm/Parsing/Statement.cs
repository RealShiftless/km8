using kasm.ValueTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parsing
{
    public enum OperandType
    {
        Unkown = 0,
        Immediate,
        Address,
        Register,
        Identifier,
        String
    }

    public enum StatementType
    {
        Unkown = 0,
        Instruction,
        Directive,
        Label
    }

    internal sealed class Statement(StatementType type, uint sourceLine, string name, Statement.Operand[] operands)
    {
        public readonly StatementType Type = type;
        public readonly uint SourceLine = sourceLine;

        public readonly string Name = name;

        public readonly Operand[] Operands = operands;
        
        public readonly struct Operand(OperandType type, string value)
        {
            public readonly OperandType Type = type;
            public readonly string ValueString = value;

            public IKasmValue? GetValue() => Type switch
            {
                OperandType.Address => new AddressValue(Convert.ToUInt16(ValueString)),
                OperandType.Register => new RegisterValue((Register)Convert.ToByte(ValueString)),
                OperandType.Immediate => new ImmediateValue(Convert.ToByte(ValueString)),
                OperandType.Identifier => null,
                _ => throw new NotImplementedException($"Operand of type {Enum.GetName(Type) ?? "???"} was not implemented!")
            };
        }
    }
}
