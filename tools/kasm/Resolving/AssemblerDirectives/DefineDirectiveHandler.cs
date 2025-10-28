using kasm.Exceptions;
using kasm.Parsing;
using kasm.Resolving;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving.AssemblerDirectives
{
    internal sealed class DefineDirectiveHandler : IAssemblerDirectiveHandler
    {
        void IAssemblerDirectiveHandler.Execute(AssemblerContext context, Statement.Operand[] operands, ref uint address)
        {
            if (operands.Length != 1)
                throw new KasmOperandException("Define directive expected exactly 1 argument!");

            switch (operands[0])
            {

            }
        }
    }
}
