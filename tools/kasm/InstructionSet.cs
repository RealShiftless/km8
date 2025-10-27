using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    public enum OperandKind { Register, Immediate, Address }

    public sealed record Instruction(
        string Mnemonic,
        byte Opcode,
        
        //[JsonConverter(typeof(StringEnumConverter))]
        OperandKind[] Operands);

    public class InstructionSet
    {
        public string Name = string.Empty;
        public int Version;

        [JsonProperty("Instructions")]
        private Instruction[]? _instructions;

        public Instruction[] Get(string mnemonic)
        {
            if (_instructions == null)
                throw new NullReferenceException("No instructions where in the set!");

            return _instructions.Where(instr => instr.Mnemonic == mnemonic).ToArray();
        }

        public Instruction? Get(byte opcode)
        {
            if (_instructions == null)
                throw new NullReferenceException("No instructions where in the set!");

            return _instructions.FirstOrDefault(instr => instr.Opcode == opcode);
        }

        public bool Contains(string mnemonic) => Get(mnemonic).Length > 0;
        public bool Contains(byte opcode) => Get(opcode) != null;
    }
}
