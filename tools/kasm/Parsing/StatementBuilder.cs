using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using static kasm.Parsing.Statement;

namespace kasm.Parsing
{
    internal sealed class StatementBuilder
    {
        // Values
        public StatementType Type;
        public uint SourceLine;

        public string Name = string.Empty;

        public List<Operand> Operands = [];


        // Constructor
        internal StatementBuilder() { }


        // Func
        internal Statement Build()
        {
            Statement statement = new(Type, SourceLine, Name, [.. Operands]);

            Type = StatementType.Unkown;
            SourceLine = 0;
            Name = string.Empty;
            Operands.Clear();

            return statement;
        }
    }
}
