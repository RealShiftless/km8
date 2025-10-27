using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Symbols
{
    public enum SymbolType
    {
        Unknown = 0,
        Address,
        Integer,
        String
    }

    public class Symbol(SymbolType type, object? value)
    {
        public readonly SymbolType Type = type;
        public object? Value = value;
    }
}
