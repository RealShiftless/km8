using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Resolving
{
    internal class MachineDirectiveSequence() : IEnumerable
    {
        // Values
        private readonly List<MachineDirective> _machineDirectives = [];

        private MachineDirectiveBuilder _nextDirective = new();


        // Properties
        public MachineDirectiveBuilder Next => _nextDirective;


        // Func
        public void SubmitDirective()
        {
            _machineDirectives.Add(Next.Build());
        }


        // Interface
        IEnumerator IEnumerable.GetEnumerator() => _machineDirectives.GetEnumerator();
    }
}
