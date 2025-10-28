using kasm.ValueTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving
{
    internal sealed class MachineDirectiveBuilder()
    {
        public uint Address;
        public uint Size;

        public Instruction? Instruction;
        public IKasmValue[]? Operands;

        public MachineDirective Build()
        {
            if (Instruction == null || Operands == null)
                throw new InvalidOperationException("Machine directive was not ready!");

            MachineDirective machineDirective = new(Address, Size, Instruction, Operands);

            Address = 0;
            Size = 0;

            Instruction = null;
            Operands = null;

            return machineDirective;
        }
    }
}
