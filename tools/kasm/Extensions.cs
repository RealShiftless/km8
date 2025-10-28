using kasm.Parsing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    public static class Extensions
    {
        public static uint GetSize(this OperandKind kind) => kind switch
        {
            OperandKind.Immediate => 1,
            OperandKind.Register => 1,
            OperandKind.Address => 2,
            _ => 0
        };
    }
}
