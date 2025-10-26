using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    public enum OperandKind { Register, Immediate, Address }

    public sealed record Instruction(
        string Mnemonic,
        byte Opcode,
        OperandKind[] Operands);

    public sealed record InstructionSet(
        string Name,
        int Version,
        Instruction[] Instructions);
}
