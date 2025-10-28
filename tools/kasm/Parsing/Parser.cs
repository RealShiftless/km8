using kasm.Exceptions;
using kasm.Symbols;
using kasm.Tokenization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Parsing
{
    internal static class Parser
    {
        public static void Parse(AssemblerContext context, TokenSequence tokens)
        {
            uint line = 0;
            foreach(Token token in tokens)
            {
                if(token.Handler.Type != TokenType.Terminator && context.Statements.Next.Type == StatementType.Unkown)
                {
                    if (token.Handler.GetStatementType() == StatementType.Unkown)
                        throw new KasmSyntaxException("Could not parse statement type!", line);

                    context.Statements.Next.Type = token.Handler.GetStatementType();
                    context.Statements.Next.Name = token.Value;

                    SymbolType symbolType = token.Handler.GetSymbolType();

                    if (symbolType != SymbolType.Unknown)
                        context.Symbols.Add(context.Statements.Next.Name, new Symbol(symbolType, null));

                    continue;
                }

                if (token.Handler.Type == TokenType.StatementType)
                    throw new KasmSyntaxException("Invallid statement!", line);

                if(token.Handler.Type == TokenType.Terminator)
                {
                    context.Statements.SubmitStatement();
                    line++;
                    continue;
                }

                if (context.Statements.Next.Type == StatementType.Label)
                    throw new KasmSyntaxException("Label expected EOL!", line);

                if(token.Handler.Type == TokenType.Operand)
                {
                    OperandType type = token.Handler.GetOperandType();

                    if (type == OperandType.Unkown)
                        throw new KasmInternalException($"Token type handler {token.Handler.TypeName} was op type Operand, but did not define an operand type.");

                    context.Statements.Next.Operands.Add(new Statement.Operand(type, token.Value));
                    continue;
                }

                throw new KasmInternalException("Reached end of parser block!");
            }
        }
    }
}
