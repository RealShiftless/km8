using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parsing
{
    /*
    internal class StatementSequenceBuilder
    {
        // Values
        public List<Statement> _statements = [];
        private bool _isLocked = false;

        private StatementBuilder _current = new();


        // Properties
        public StatementBuilder Current => _current;


        // Func
        public void Next()
        {
            if (_isLocked) throw new InvalidOperationException("Builder was locked!");

            if (Current.Type == StatementType.Unkown)
                return;

            _statements.Add(Current.Build());
        }

        public StatementSequence Build()
        {
            _isLocked = true;
            return new([.. _statements]);
        }
    }
    */
}
