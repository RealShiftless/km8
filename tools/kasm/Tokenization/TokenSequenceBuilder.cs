using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace kasm.Tokenization
{
    internal sealed class TokenSequenceBuilder
    {
        private bool _isLocked = false;
        private List<Token> _tokens = [];

        public void Add(Token token)
        {
            if(_isLocked) throw new InvalidOperationException("Token sequence was locked!");

            _tokens.Add(token);
        }
        public TokenSequence Build()
        {
            Lock();
            return new TokenSequence([.. _tokens]);
        }

        public void Lock() => _isLocked = true;
    }

}
