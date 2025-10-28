using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization
{
    internal sealed class TokenSequence : IEnumerable
    {
        // Values
        private Token[] _tokens;


        // Constructor
        internal TokenSequence(Token[] tokens) => _tokens = tokens;


        // Func
        public Token Get(int index) => _tokens[index];


        // Interface
        IEnumerator IEnumerable.GetEnumerator() => _tokens.GetEnumerator();
    }
}
