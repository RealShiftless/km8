using kasm.ValueTypes;
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
        //String
    }

    internal class Symbol(SymbolType type, IKasmValue? value)
    {
        public readonly SymbolType Type = type;
        public IKasmValue? Value = value;
    }
}
