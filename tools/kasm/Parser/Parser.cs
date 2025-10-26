using kasm.Tokenization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parser
{
    public sealed record AsmInstruction(
        string Mnemonic,
        IReadOnlyList<Token> Operands,
        int SourceLine);

    public static class Parser
    {
    }
}
