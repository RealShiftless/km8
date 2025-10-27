using kasm.Exceptions;
using kasm.Parsing.Exceptions;
using kasm.Symbols;
using kasm.Tokenization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parsing
{
    public sealed record AsmInstruction(
        string Mnemonic,
        IReadOnlyList<Token> Operands,
        int SourceLine);

    public static class Parser
    {
        public static void Parse(AssemblerContext context)
        {
            StatementSequenceBuilder statements = new();

            uint line = 0;
            foreach(Token token in context.Tokens)
            {
                if(token.Handler.Type != TokenType.Terminator && statements.Current.Type == StatementType.Unkown)
                {
                    if (token.Handler.GetStatementType() == StatementType.Unkown)
                        throw new KasmSyntaxException("Could not parse statement type!", line);

                    statements.Current.Type = token.Handler.GetStatementType();
                    statements.Current.Name = token.Value;

                    SymbolType symbolType = token.Handler.GetSymbolType();

                    if (symbolType != SymbolType.Unknown)
                        context.Symbols.Add(statements.Current.Name, new Symbol(symbolType, null));

                    continue;
                }

                if (token.Handler.Type == TokenType.StatementType)
                    throw new KasmSyntaxException("Invallid statement!", line);

                if(token.Handler.Type == TokenType.Terminator)
                {
                    statements.Next();
                    line++;
                    continue;
                }

                if (statements.Current.Type == StatementType.Label)
                    throw new KasmSyntaxException("Label expected EOL!", line);

                if(token.Handler.Type == TokenType.Operand)
                {
                    OperandType type = token.Handler.GetOperandType();

                    if (type == OperandType.Unkown)
                        throw new KasmInternalException($"Token type handler {token.Handler.TypeName} was op type Operand, but did not define an operand type.");

                    statements.Current.Operands.Add(new Statement.Operand(type, token.Value));
                    continue;
                }

                throw new KasmInternalException("Reached end of parser block!");
            }

            context.BindStatementSequence(statements.Build());
        }
    }
}
