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
module.exports = {
  'extends': ['plugin:import/errors'],
  'plugins': ['import'],
  'env': {
    'es6': true,
    'node': true
  },
  'parserOptions': {
    'ecmaVersion': 6,
    'sourceType': 'module',
    'ecmaFeatures': {
      'impliedStrict': true,
      'objectLiteralDuplicateProperties': false
    }
  },
  'rules': {
    'array-bracket-spacing': ['error', 'never'],

    'camelcase': ['error', {
      'properties': 'never'
    }],

    'comma-dangle': ['error', 'never'],

    'curly': ['error', 'all'],

    'eol-last': ['error'],

    'indent': ['error', 2, {
      'SwitchCase': 1
    }],

    'keyword-spacing': ['error'],

    'max-len': ['error', {
      'code': 180,
      'ignoreComments': true,
      'ignoreRegExpLiterals': true
    }],

    'no-else-return': ['error'],

    'no-mixed-spaces-and-tabs': ['error'],

    'no-multiple-empty-lines': ['error'],

    'no-spaced-func': ['error'],

    'no-trailing-spaces': ['error'],

    'no-undef': ['error'],

    'no-unexpected-multiline': ['error'],

    'no-unused-vars': ['error', {
      'args': 'none',
      'vars': 'all'
    }],

    'quotes': ['error', 'single', {
      'allowTemplateLiterals': true,
      'avoidEscape': true
    }],

    'semi': ['error', 'never'],

    'space-before-blocks': ['error', 'always'],

    'space-before-function-paren': ['error', 'never'],

    'space-in-parens': ['error', 'never'],

    'space-unary-ops': ['error', {
      'nonwords': false,
      'overrides': {}
    }],



    'arrow-body-style': ['error', 'as-needed', {
      'requireReturnForObjectLiteral': false
    }],

    'arrow-parens': ['error', 'always'],

    'arrow-spacing': ['error', {
      'after': true,
      'before': true
    }],

    'no-class-assign': ['error'],

    'no-const-assign': ['error'],

    'no-dupe-class-members': ['error'],

    'no-duplicate-imports': ['error'],

    'no-new-symbol': ['error'],

    'no-useless-rename': ['error'],

    'no-var': ['error'],

    'object-shorthand': ['error', 'always', {
      'avoidQuotes': true,
      'ignoreConstructors': false
    }],

    'prefer-arrow-callback': ['error', {
      'allowNamedFunctions': false,
      'allowUnboundThis': true
    }],

    'prefer-const': ['error'],

    'prefer-rest-params': ['error'],

    'prefer-template': ['error'],

    'template-curly-spacing': ['error', 'never']
  }
};
