using kasm.ValueTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving
{
    internal sealed class MachineDirective(uint address, uint size, Instruction instruction, IKasmValue[] operands)
    {
        public readonly uint Address = address;
        public readonly uint Size = size;

        public readonly Instruction Instruction = instruction;
        public readonly IKasmValue[] Operands = operands;
    }
}
