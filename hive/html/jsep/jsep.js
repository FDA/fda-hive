/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

(function (root) {
    'use strict';

    var COMPOUND = 'Compound',
        IDENTIFIER = 'Identifier',
        MEMBER_EXP = 'MemberExpression',
        LITERAL = 'Literal',
        THIS_EXP = 'ThisExpression',
        CALL_EXP = 'CallExpression',
        UNARY_EXP = 'UnaryExpression',
        BINARY_EXP = 'BinaryExpression',
        LOGICAL_EXP = 'LogicalExpression',
        CONDITIONAL_EXP = 'ConditionalExpression',
        ARRAY_EXP = 'ArrayExpression',

        PERIOD_CODE = 46,
        COMMA_CODE  = 44,
        SQUOTE_CODE = 39,
        DQUOTE_CODE = 34,
        OPAREN_CODE = 40,
        CPAREN_CODE = 41,
        OBRACK_CODE = 91,
        CBRACK_CODE = 93,
        QUMARK_CODE = 63,
        SEMCOL_CODE = 59,
        COLON_CODE  = 58,

        throwError = function(message, index) {
            var error = new Error(message + ' at character ' + index);
            error.index = index;
            error.description = message;
            throw error;
        },


        t = true,
        unary_ops = {'-': t, '!': t, '~': t, '+': t},
        binary_ops = {
            '||': 1, '&&': 2, '|': 3,  '^': 4,  '&': 5,
            '==': 6, '!=': 6, '===': 6, '!==': 6,
            '<': 7,  '>': 7,  '<=': 7,  '>=': 7,
            '<<':8,  '>>': 8, '>>>': 8,
            '+': 9, '-': 9,
            '*': 10, '/': 10, '%': 10
        },
        getMaxKeyLen = function(obj) {
            var max_len = 0, len;
            for(var key in obj) {
                if((len = key.length) > max_len && obj.hasOwnProperty(key)) {
                    max_len = len;
                }
            }
            return max_len;
        },
        max_unop_len = getMaxKeyLen(unary_ops),
        max_binop_len = getMaxKeyLen(binary_ops),
        literals = {
            'true': true,
            'false': false,
            'null': null
        },
        this_str = 'this',
        binaryPrecedence = function(op_val) {
            return binary_ops[op_val] || 0;
        },
        createBinaryExpression = function (operator, left, right) {
            var type = (operator === '||' || operator === '&&') ? LOGICAL_EXP : BINARY_EXP;
            return {
                type: type,
                operator: operator,
                left: left,
                right: right
            };
        },
        isDecimalDigit = function(ch) {
            return (ch >= 48 && ch <= 57);
        },
        isIdentifierStart = function(ch) {
            return (ch === 36) || (ch === 95) ||
                    (ch >= 65 && ch <= 90) ||
                    (ch >= 97 && ch <= 122) ||
                    (ch >= 128 && !binary_ops[String.fromCharCode(ch)]);
        },
        isIdentifierPart = function(ch) {
            return (ch === 36) || (ch === 95) ||
                    (ch >= 65 && ch <= 90) ||
                    (ch >= 97 && ch <= 122) ||
                    (ch >= 48 && ch <= 57) ||
                    (ch >= 128 && !binary_ops[String.fromCharCode(ch)]);
        },

        jsep = function(expr) {
            var index = 0,
                charAtFunc = expr.charAt,
                charCodeAtFunc = expr.charCodeAt,
                exprI = function(i) { return charAtFunc.call(expr, i); },
                exprICode = function(i) { return charCodeAtFunc.call(expr, i); },
                length = expr.length,

                gobbleSpaces = function() {
                    var ch = exprICode(index);
                    while(ch === 32 || ch === 9 || ch === 10 || ch === 13) {
                        ch = exprICode(++index);
                    }
                },

                gobbleExpression = function() {
                    var test = gobbleBinaryExpression(),
                        consequent, alternate;
                    gobbleSpaces();
                    if(exprICode(index) === QUMARK_CODE) {
                        index++;
                        consequent = gobbleExpression();
                        if(!consequent) {
                            throwError('Expected expression', index);
                        }
                        gobbleSpaces();
                        if(exprICode(index) === COLON_CODE) {
                            index++;
                            alternate = gobbleExpression();
                            if(!alternate) {
                                throwError('Expected expression', index);
                            }
                            return {
                                type: CONDITIONAL_EXP,
                                test: test,
                                consequent: consequent,
                                alternate: alternate
                            };
                        } else {
                            throwError('Expected :', index);
                        }
                    } else {
                        return test;
                    }
                },

                gobbleBinaryOp = function() {
                    gobbleSpaces();
                    var biop, to_check = expr.substr(index, max_binop_len), tc_len = to_check.length;
                    while(tc_len > 0) {
                        if(binary_ops.hasOwnProperty(to_check)) {
                            index += tc_len;
                            return to_check;
                        }
                        to_check = to_check.substr(0, --tc_len);
                    }
                    return false;
                },

                gobbleBinaryExpression = function() {
                    var ch_i, node, biop, prec, stack, biop_info, left, right, i;

                    left = gobbleToken();
                    biop = gobbleBinaryOp();

                    if(!biop) {
                        return left;
                    }

                    biop_info = { value: biop, prec: binaryPrecedence(biop)};

                    right = gobbleToken();
                    if(!right) {
                        throwError("Expected expression after " + biop, index);
                    }
                    stack = [left, biop_info, right];

                    while((biop = gobbleBinaryOp())) {
                        prec = binaryPrecedence(biop);

                        if(prec === 0) {
                            break;
                        }
                        biop_info = { value: biop, prec: prec };

                        while ((stack.length > 2) && (prec <= stack[stack.length - 2].prec)) {
                            right = stack.pop();
                            biop = stack.pop().value;
                            left = stack.pop();
                            node = createBinaryExpression(biop, left, right);
                            stack.push(node);
                        }

                        node = gobbleToken();
                        if(!node) {
                            throwError("Expected expression after " + biop, index);
                        }
                        stack.push(biop_info, node);
                    }

                    i = stack.length - 1;
                    node = stack[i];
                    while(i > 1) {
                        node = createBinaryExpression(stack[i - 1].value, stack[i - 2], node);
                        i -= 2;
                    }
                    return node;
                },

                gobbleToken = function() {
                    var ch, to_check, tc_len;

                    gobbleSpaces();
                    ch = exprICode(index);

                    if(isDecimalDigit(ch) || ch === PERIOD_CODE) {
                        return gobbleNumericLiteral();
                    } else if(ch === SQUOTE_CODE || ch === DQUOTE_CODE) {
                        return gobbleStringLiteral();
                    } else if(isIdentifierStart(ch) || ch === OPAREN_CODE) {
                        return gobbleVariable();
                    } else if (ch === OBRACK_CODE) {
                        return gobbleArray();
                    } else {
                        to_check = expr.substr(index, max_unop_len);
                        tc_len = to_check.length;
                        while(tc_len > 0) {
                            if(unary_ops.hasOwnProperty(to_check)) {
                                index += tc_len;
                                return {
                                    type: UNARY_EXP,
                                    operator: to_check,
                                    argument: gobbleToken(),
                                    prefix: true
                                };
                            }
                            to_check = to_check.substr(0, --tc_len);
                        }

                        return false;
                    }
                },
                gobbleNumericLiteral = function() {
                    var number = '', ch, chCode;
                    while(isDecimalDigit(exprICode(index))) {
                        number += exprI(index++);
                    }

                    if(exprICode(index) === PERIOD_CODE) {
                        number += exprI(index++);

                        while(isDecimalDigit(exprICode(index))) {
                            number += exprI(index++);
                        }
                    }

                    ch = exprI(index);
                    if(ch === 'e' || ch === 'E') {
                        number += exprI(index++);
                        ch = exprI(index);
                        if(ch === '+' || ch === '-') {
                            number += exprI(index++);
                        }
                        while(isDecimalDigit(exprICode(index))) {
                            number += exprI(index++);
                        }
                        if(!isDecimalDigit(exprICode(index-1)) ) {
                            throwError('Expected exponent (' + number + exprI(index) + ')', index);
                        }
                    }


                    chCode = exprICode(index);
                    if(isIdentifierStart(chCode)) {
                        throwError('Variable names cannot start with a number (' +
                                    number + exprI(index) + ')', index);
                    } else if(chCode === PERIOD_CODE) {
                        throwError('Unexpected period', index);
                    }

                    return {
                        type: LITERAL,
                        value: parseFloat(number),
                        raw: number
                    };
                },

                gobbleStringLiteral = function() {
                    var str = '', quote = exprI(index++), closed = false, ch;

                    while(index < length) {
                        ch = exprI(index++);
                        if(ch === quote) {
                            closed = true;
                            break;
                        } else if(ch === '\\') {
                            ch = exprI(index++);
                            switch(ch) {
                                case 'n': str += '\n'; break;
                                case 'r': str += '\r'; break;
                                case 't': str += '\t'; break;
                                case 'b': str += '\b'; break;
                                case 'f': str += '\f'; break;
                                case 'v': str += '\x0B'; break;
                                default : str += '\\' + ch;
                            }
                        } else {
                            str += ch;
                        }
                    }

                    if(!closed) {
                        throwError('Unclosed quote after "'+str+'"', index);
                    }

                    return {
                        type: LITERAL,
                        value: str,
                        raw: quote + str + quote
                    };
                },

                gobbleIdentifier = function() {
                    var ch = exprICode(index), start = index, identifier;

                    if(isIdentifierStart(ch)) {
                        index++;
                    } else {
                        throwError('Unexpected ' + exprI(index), index);
                    }

                    while(index < length) {
                        ch = exprICode(index);
                        if(isIdentifierPart(ch)) {
                            index++;
                        } else {
                            break;
                        }
                    }
                    identifier = expr.slice(start, index);

                    if(literals.hasOwnProperty(identifier)) {
                        return {
                            type: LITERAL,
                            value: literals[identifier],
                            raw: identifier
                        };
                    } else if(identifier === this_str) {
                        return { type: THIS_EXP };
                    } else {
                        return {
                            type: IDENTIFIER,
                            name: identifier
                        };
                    }
                },

                gobbleArguments = function(termination) {
                    var ch_i, args = [], node, closed = false;
                    while(index < length) {
                        gobbleSpaces();
                        ch_i = exprICode(index);
                        if(ch_i === termination) {
                            closed = true;
                            index++;
                            break;
                        } else if (ch_i === COMMA_CODE) {
                            index++;
                        } else {
                            node = gobbleExpression();
                            if(!node || node.type === COMPOUND) {
                                throwError('Expected comma', index);
                            }
                            args.push(node);
                        }
                    }
                    if (!closed) {
                        throwError('Expected ' + String.fromCharCode(termination), index);
                    }
                    return args;
                },

                gobbleVariable = function() {
                    var ch_i, node;
                    ch_i = exprICode(index);

                    if(ch_i === OPAREN_CODE) {
                        node = gobbleGroup();
                    } else {
                        node = gobbleIdentifier();
                    }
                    gobbleSpaces();
                    ch_i = exprICode(index);
                    while(ch_i === PERIOD_CODE || ch_i === OBRACK_CODE || ch_i === OPAREN_CODE) {
                        index++;
                        if(ch_i === PERIOD_CODE) {
                            gobbleSpaces();
                            node = {
                                type: MEMBER_EXP,
                                computed: false,
                                object: node,
                                property: gobbleIdentifier()
                            };
                        } else if(ch_i === OBRACK_CODE) {
                            node = {
                                type: MEMBER_EXP,
                                computed: true,
                                object: node,
                                property: gobbleExpression()
                            };
                            gobbleSpaces();
                            ch_i = exprICode(index);
                            if(ch_i !== CBRACK_CODE) {
                                throwError('Unclosed [', index);
                            }
                            index++;
                        } else if(ch_i === OPAREN_CODE) {
                            node = {
                                type: CALL_EXP,
                                'arguments': gobbleArguments(CPAREN_CODE),
                                callee: node
                            };
                        }
                        gobbleSpaces();
                        ch_i = exprICode(index);
                    }
                    return node;
                },

                gobbleGroup = function() {
                    index++;
                    var node = gobbleExpression();
                    gobbleSpaces();
                    if(exprICode(index) === CPAREN_CODE) {
                        index++;
                        return node;
                    } else {
                        throwError('Unclosed (', index);
                    }
                },

                gobbleArray = function() {
                    index++;
                    return {
                        type: ARRAY_EXP,
                        elements: gobbleArguments(CBRACK_CODE)
                    };
                },

                nodes = [], ch_i, node;

            while(index < length) {
                ch_i = exprICode(index);

                if(ch_i === SEMCOL_CODE || ch_i === COMMA_CODE) {
                    index++;
                } else {
                    if((node = gobbleExpression())) {
                        nodes.push(node);
                    } else if(index < length) {
                        throwError('Unexpected "' + exprI(index) + '"', index);
                    }
                }
            }

            if(nodes.length === 1) {
                return nodes[0];
            } else {
                return {
                    type: COMPOUND,
                    body: nodes
                };
            }
        };

    jsep.version = '0.3.2';
    jsep.toString = function() { return 'JavaScript Expression Parser (JSEP) v' + jsep.version; };

    jsep.addUnaryOp = function(op_name) {
        max_unop_len = Math.max(op_name.length, max_unop_len);
        unary_ops[op_name] = t; return this;
    };

    jsep.addBinaryOp = function(op_name, precedence) {
        max_binop_len = Math.max(op_name.length, max_binop_len);
        binary_ops[op_name] = precedence;
        return this;
    };

    jsep.addLiteral = function(literal_name, literal_value) {
        literals[literal_name] = literal_value;
        return this;
    };

    jsep.removeUnaryOp = function(op_name) {
        delete unary_ops[op_name];
        if(op_name.length === max_unop_len) {
            max_unop_len = getMaxKeyLen(unary_ops);
        }
        return this;
    };

    jsep.removeAllUnaryOps = function() {
        unary_ops = {};
        max_unop_len = 0;
        
        return this;
    };

    jsep.removeBinaryOp = function(op_name) {
        delete binary_ops[op_name];
        if(op_name.length === max_binop_len) {
            max_binop_len = getMaxKeyLen(binary_ops);
        }
        return this;
    };

    jsep.removeAllBinaryOps = function() {
        binary_ops = {};
        max_binop_len = 0;
        
        return this;
    };

    jsep.removeLiteral = function(literal_name) {
        delete literals[literal_name];
        return this;
    };

    jsep.removeAllLiterals = function() {
        literals = {};
        
        return this;
    };

    if (typeof exports === 'undefined') {
        var old_jsep = root.jsep;
        root.jsep = jsep;
        jsep.noConflict = function() {
            if(root.jsep === jsep) {
                root.jsep = old_jsep;
            }
            return jsep;
        };
    } else {
        if (typeof module !== 'undefined' && module.exports) {
            exports = module.exports = jsep;
        } else {
            exports.parse = jsep;
        }
    }
}(this));
