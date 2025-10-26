using Kasm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization.TokenTypes
{
    public sealed class InstructionToken : ITokenType
    {
        public string TypeName => "Instruction";

        public Token? TryParse(string value)
        {
            Instruction? instruction = Program.InstructionSet.Instructions.FirstOrDefault(inst => string.Equals(inst.Mnemonic, value, StringComparison.OrdinalIgnoreCase));

            return instruction != null ? new Token(this, value) : null;
        }
    }
}
