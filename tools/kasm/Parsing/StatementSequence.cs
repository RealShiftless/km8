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
        private readonly List<Statement> _statements = [];

        private StatementBuilder _nextStatement = new();


        // Properties
        internal StatementBuilder Next => _nextStatement;


        // Constructor
        internal StatementSequence() { }


        // Func
        public void SubmitStatement()
        {
            if (_nextStatement.Type == StatementType.Unkown)
                return;

            _statements.Add(_nextStatement.Build());
        }


        // Interface
        IEnumerator IEnumerable.GetEnumerator() => _statements.GetEnumerator();
    }
}
