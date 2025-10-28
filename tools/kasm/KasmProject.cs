using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm
{
    public sealed record KasmProject(
        string Name,
        string InstructionSet,
        int RomSizeCode,
        int RamSizeCode,

        string[] SourceFiles,
        string Output
        );
}
