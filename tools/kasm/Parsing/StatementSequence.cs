using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parsing
{
    public sealed class StatementSequence : IEnumerable
    {
        // Values
        private readonly Statement[] _statements;


        // Constructor
        internal StatementSequence(Statement[] statements)
        {
            _statements = statements;
        }


        // Interface
        IEnumerator IEnumerable.GetEnumerator() => _statements.GetEnumerator();
    }
}
