using kasm.Parsing;
using kasm.Resolving.AssemblerDirectives;
using kasm.Symbols;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving
{
    internal static class Resolver
    {
        

        public static MachineDirectiveSequence Resolve(AssemblerContext context)
        {
            Dictionary<string, IAssemblerDirectiveHandler> _assemblerDirectiveHandler = new()
            {
                ["def"] = new DefineDirectiveHandler()
            };

            MachineDirectiveSequence machineDirectives = new();

            foreach(Statement statement in context.Statements)
            {
                switch(statement.Type)
                {
                    case StatementType.Label:
                        Symbol labelSymbol = context.Symbols.Get(statement.Name);
                        labelSymbol.Value = context.CurrentAddress;
                        break;
                }
            }

            return machineDirectives;
        }
    }
}
