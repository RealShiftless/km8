using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Symbols
{
    public sealed class SymbolTable : IEnumerable<KeyValuePair<string, Symbol>>
    {
        // Values
        private Dictionary<string, Symbol> _symbols = [];


        // Func
        public Symbol Get(string name) => _symbols[name];

        internal void Add(string name, Symbol symbol)
        {
            _symbols.Add(name, symbol);
        }


        // Interface
        IEnumerator<KeyValuePair<string, Symbol>> IEnumerable<KeyValuePair<string, Symbol>>.GetEnumerator() => _symbols.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => _symbols.GetEnumerator();
    }
}
