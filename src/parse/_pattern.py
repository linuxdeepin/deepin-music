#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2011~2012 Deepin, Inc.
#               2011~2012 Hou Shaohui
#
# Author:     Hou Shaohui <houshao55@gmail.com>
# Maintainer: Hou ShaoHui <houshao55@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import re

import utils
from utils import auto_decode as fsdecode

class _Dummy(dict):
    def comma(self, *args): return u"_"

# Token types.
(OPEN, CLOSE, TEXT, COND, EOF) = range(5)

class error(ValueError): pass
class ParseError(error): pass
class LexerError(error): pass

class PatternLexeme(object):
    _reverse = { OPEN: "OPEN", CLOSE: "CLOSE", TEXT: "TEXT", COND: "COND",
                 EOF: "EOF" }

    def __init__(self, typ, lexeme):
        self.type = typ
        self.lexeme = lexeme

    def __repr__(self):
        return (super(PatternLexeme, self).__repr__().split()[0] +
                " type=" + repr(self.type) + " (" +
                str(self._reverse[self.type]) +
                "), lexeme=" + repr(self.lexeme) + ">")

class PatternLexer(re.Scanner):
    def __init__(self, s):
        self.string = s.strip()
        re.Scanner.__init__(self,
                             [(r"([^<>|\\]|\\.)+", self.text),
                              (r"[<>|]", self.table),
                              ])

    def text(self, scanner, string):
        return PatternLexeme(TEXT, re.sub(r"\\(.)", r"\1", string))
    def table(self, scanner, string):
        return PatternLexeme(
            {"|": COND, "<": OPEN, ">": CLOSE}[string], string)

    def __iter__(self):
        s = self.scan(self.string)
        if s[1] != "": raise LexerError("characters left over in string")
        else: return iter(s[0] + [PatternLexeme(EOF, "")])

class PatternParser(object):
    def __init__(self, tokens):
        self.tokens = iter(tokens)
        self.lookahead = self.tokens.next()

    def Pattern(self, song):
        text = []
        while self.lookahead.type in [OPEN, TEXT]:
            la = self.lookahead
            self.match(TEXT, OPEN)
            if la.type == TEXT: text.append(la.lexeme)
            elif la.type == OPEN: text.append(self.Tags(song))
        return u"".join(text)

    def Tags(self, song):
        text = []
        tag = self.lookahead.lexeme
        if not tag.startswith("~") and "~" in tag: tag = "~" + tag
        try: self.match(TEXT)
        except ParseError:
            while self.lookahead.type not in [CLOSE, EOF]:
                text.append(self.lookahead.lexeme)
                self.match(self.lookahead.type)
            return u"".join(text)
        if self.lookahead.type == COND:
            self.match(COND)
            ifcase = self.Pattern(song)
            if self.lookahead.type == COND:
                self.match(COND)
                elsecase = self.Pattern(song)
            else: elsecase = u""

            if song.comma(tag): text.append(ifcase)
            else: text.append(elsecase)

            try: self.match(CLOSE)
            except ParseError:
                text.pop(-1)
                text.append("<")
                text.append("|".join(filter(None, [tag, ifcase, elsecase])))
                while self.lookahead.type not in [EOF, OPEN]:
                    text.append(self.lookahead.lexeme)
                    self.match(self.lookahead.type)
        else:
            text.append(song.comma(tag))
            try: self.match(CLOSE)
            except ParseError:
                text.pop(-1)
                text.append("<")
                text.append(tag)
                while self.lookahead.type not in [EOF, OPEN]:
                    text.append(self.lookahead.lexeme)
                    self.match(self.lookahead.type)
        return u"".join(text)

    def match(self, *tokens):
        if tokens != [EOF] and self.lookahead.type == EOF:
            raise ParseError("The search string ended, but more "
                             "tokens were expected.")
        try:
            if self.lookahead.type in tokens:
                self.lookahead = self.tokens.next()
            else:
                raise ParseError("The token '%s' is not the type exected." %(
                    self.lookahead.lexeme))
        except StopIteration:
            self.lookahead = PatternLexeme(EOF, "")

class Pattern(PatternParser):
    _formatters = []

    def __init__(self, string):
        self.__string = string
        self.__tokens = list(PatternLexer(self.__string))
        self.format(_Dummy()) # Validate string

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self.__string)

    class Song(object):
        def __init__(self, realsong, formatters):
            self.__song = realsong
            self.__formatters = formatters

        def comma(self, *args):
            value = self.__song.comma(*args)
            if isinstance(value, str):
                value = fsdecode(value)
            elif not isinstance(value, unicode):
                value = unicode(value)
            for f in self.__formatters:
                value = f(args[0], value)
            return value

    def format(self, song):
        p = PatternParser(self.__tokens)
        return self._post(p.Pattern(self.Song(song, self._formatters)), song)

    def _post(self, value, song): return value

    __mod__ = format

def _number(key, value):
    if key == "tracknumber":
        parts = value.split("/")
        try: return "%02d" % int(parts[0])
        except (TypeError, ValueError): return value
    elif key == "discnumber":
        parts = value.split("/")
        try: return "%02d" % int(parts[0])
        except (TypeError, ValueError): return value
    else: return value

class FileFromPattern(Pattern):
    _formatters = [_number,
                   (lambda k, s: s.lstrip(".")),
                   (lambda k, s: s.replace("/", "_")),
                   (lambda k, s: s.replace(u"\uff0f", "_")),
                   (lambda k, s: s.strip()),
                   (lambda k, s: (len(s) > 100 and s[:100] + "...") or s),
                   ]

    def _post(self, value, song):
        if value:
            fn = song.get("~filename", ".")
            ext = fn[fn.rfind("."):]
            if not value.endswith(ext): value += ext
            value = os.path.expanduser(value)
            if "/" in value and not value.startswith("/"):
                raise ValueError("Pattern is not rooted")
        return value

class XMLFromPattern(Pattern):
    _formatters = [lambda k, s: utils.xmlescape(s)]
