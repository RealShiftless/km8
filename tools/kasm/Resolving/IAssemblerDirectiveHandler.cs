using kasm.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving
{
    internal interface IAssemblerDirectiveHandler
    {
        void Execute(AssemblerContext context, Statement.Operand[] operands, ref uint address);
    }
}
