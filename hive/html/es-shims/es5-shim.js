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


;

(function (root, factory) {
    'use strict';

    if (typeof define === 'function' && define.amd) {
        define(factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.returnExports = factory();
    }
}(this, function () {


var $Array = Array;
var ArrayPrototype = $Array.prototype;
var $Object = Object;
var ObjectPrototype = $Object.prototype;
var $Function = Function;
var FunctionPrototype = $Function.prototype;
var $String = String;
var StringPrototype = $String.prototype;
var $Number = Number;
var NumberPrototype = $Number.prototype;
var array_slice = ArrayPrototype.slice;
var array_splice = ArrayPrototype.splice;
var array_push = ArrayPrototype.push;
var array_unshift = ArrayPrototype.unshift;
var array_concat = ArrayPrototype.concat;
var array_join = ArrayPrototype.join;
var call = FunctionPrototype.call;
var apply = FunctionPrototype.apply;
var max = Math.max;
var min = Math.min;

var to_string = ObjectPrototype.toString;

var hasToStringTag = typeof Symbol === 'function' && typeof Symbol.toStringTag === 'symbol';
var isCallable;var fnToStr = Function.prototype.toString, constructorRegex = /\s*class /, isES6ClassFn = function isES6ClassFn(value) { try { var fnStr = fnToStr.call(value); var singleStripped = fnStr.replace(/\/\/.*\n/g, ''); var multiStripped = singleStripped.replace(/\/\*[.\s\S]*\*\//g, ''); var spaceStripped = multiStripped.replace(/\n/mg, ' ').replace(/ {2}/g, ' '); return constructorRegex.test(spaceStripped); } catch (e) { return false;} }, tryFunctionObject = function tryFunctionObject(value) { try { if (isES6ClassFn(value)) { return false; } fnToStr.call(value); return true; } catch (e) { return false; } }, fnClass = '[object Function]', genClass = '[object GeneratorFunction]', isCallable = function isCallable(value) { if (!value) { return false; } if (typeof value !== 'function' && typeof value !== 'object') { return false; } if (hasToStringTag) { return tryFunctionObject(value); } if (isES6ClassFn(value)) { return false; } var strClass = to_string.call(value); return strClass === fnClass || strClass === genClass; };

var isRegex;var regexExec = RegExp.prototype.exec, tryRegexExec = function tryRegexExec(value) { try { regexExec.call(value); return true; } catch (e) { return false; } }, regexClass = '[object RegExp]'; isRegex = function isRegex(value) { if (typeof value !== 'object') { return false; } return hasToStringTag ? tryRegexExec(value) : to_string.call(value) === regexClass; };
var isString;var strValue = String.prototype.valueOf, tryStringObject = function tryStringObject(value) { try { strValue.call(value); return true; } catch (e) { return false; } }, stringClass = '[object String]'; isString = function isString(value) { if (typeof value === 'string') { return true; } if (typeof value !== 'object') { return false; } return hasToStringTag ? tryStringObject(value) : to_string.call(value) === stringClass; };

var supportsDescriptors = $Object.defineProperty && (function () {
    try {
        var obj = {};
        $Object.defineProperty(obj, 'x', { enumerable: false, value: obj });
        for (var _ in obj) { return false; }
        return obj.x === obj;
    } catch (e) {
        return false;
    }
}());
var defineProperties = (function (has) {
  var defineProperty;
  if (supportsDescriptors) {
      defineProperty = function (object, name, method, forceAssign) {
          if (!forceAssign && (name in object)) { return; }
          $Object.defineProperty(object, name, {
              configurable: true,
              enumerable: false,
              writable: true,
              value: method
          });
      };
  } else {
      defineProperty = function (object, name, method, forceAssign) {
          if (!forceAssign && (name in object)) { return; }
          object[name] = method;
      };
  }
  return function defineProperties(object, map, forceAssign) {
      for (var name in map) {
          if (has.call(map, name)) {
            defineProperty(object, name, map[name], forceAssign);
          }
      }
  };
}(ObjectPrototype.hasOwnProperty));


var isPrimitive = function isPrimitive(input) {
    var type = typeof input;
    return input === null || (type !== 'object' && type !== 'function');
};

var isActualNaN = $Number.isNaN || function (x) { return x !== x; };

var ES = {
    ToInteger: function ToInteger(num) {
        var n = +num;
        if (isActualNaN(n)) {
            n = 0;
        } else if (n !== 0 && n !== (1 / 0) && n !== -(1 / 0)) {
            n = (n > 0 || -1) * Math.floor(Math.abs(n));
        }
        return n;
    },

    ToPrimitive: function ToPrimitive(input) {
        var val, valueOf, toStr;
        if (isPrimitive(input)) {
            return input;
        }
        valueOf = input.valueOf;
        if (isCallable(valueOf)) {
            val = valueOf.call(input);
            if (isPrimitive(val)) {
                return val;
            }
        }
        toStr = input.toString;
        if (isCallable(toStr)) {
            val = toStr.call(input);
            if (isPrimitive(val)) {
                return val;
            }
        }
        throw new TypeError();
    },

    ToObject: function (o) {
        if (o == null) {
            throw new TypeError("can't convert " + o + ' to object');
        }
        return $Object(o);
    },

    ToUint32: function ToUint32(x) {
        return x >>> 0;
    }
};



var Empty = function Empty() {};

defineProperties(FunctionPrototype, {
    bind: function bind(that) {
        var target = this;
        if (!isCallable(target)) {
            throw new TypeError('Function.prototype.bind called on incompatible ' + target);
        }
        var args = array_slice.call(arguments, 1);
        var bound;
        var binder = function () {

            if (this instanceof bound) {

                var result = apply.call(
                    target,
                    this,
                    array_concat.call(args, array_slice.call(arguments))
                );
                if ($Object(result) === result) {
                    return result;
                }
                return this;

            } else {

                return apply.call(
                    target,
                    that,
                    array_concat.call(args, array_slice.call(arguments))
                );

            }

        };


        var boundLength = max(0, target.length - args.length);

        var boundArgs = [];
        for (var i = 0; i < boundLength; i++) {
            array_push.call(boundArgs, '$' + i);
        }

        bound = $Function('binder', 'return function (' + array_join.call(boundArgs, ',') + '){ return binder.apply(this, arguments); }')(binder);

        if (target.prototype) {
            Empty.prototype = target.prototype;
            bound.prototype = new Empty();
            Empty.prototype = null;
        }




        return bound;
    }
});

var owns = call.bind(ObjectPrototype.hasOwnProperty);
var toStr = call.bind(ObjectPrototype.toString);
var arraySlice = call.bind(array_slice);
var arraySliceApply = apply.bind(array_slice);
var strSlice = call.bind(StringPrototype.slice);
var strSplit = call.bind(StringPrototype.split);
var strIndexOf = call.bind(StringPrototype.indexOf);
var pushCall = call.bind(array_push);
var isEnum = call.bind(ObjectPrototype.propertyIsEnumerable);
var arraySort = call.bind(ArrayPrototype.sort);


var isArray = $Array.isArray || function isArray(obj) {
    return toStr(obj) === '[object Array]';
};

var hasUnshiftReturnValueBug = [].unshift(0) !== 1;
defineProperties(ArrayPrototype, {
    unshift: function () {
        array_unshift.apply(this, arguments);
        return this.length;
    }
}, hasUnshiftReturnValueBug);

defineProperties($Array, { isArray: isArray });



var boxedString = $Object('a');
var splitString = boxedString[0] !== 'a' || !(0 in boxedString);

var properlyBoxesContext = function properlyBoxed(method) {
    var properlyBoxesNonStrict = true;
    var properlyBoxesStrict = true;
    var threwException = false;
    if (method) {
        try {
            method.call('foo', function (_, __, context) {
                if (typeof context !== 'object') { properlyBoxesNonStrict = false; }
            });

            method.call([1], function () {
                'use strict';

                properlyBoxesStrict = typeof this === 'string';
            }, 'x');
        } catch (e) {
            threwException = true;
        }
    }
    return !!method && !threwException && properlyBoxesNonStrict && properlyBoxesStrict;
};

defineProperties(ArrayPrototype, {
    forEach: function forEach(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var i = -1;
        var length = ES.ToUint32(self.length);
        var T;
        if (arguments.length > 1) {
          T = arguments[1];
        }

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.forEach callback must be a function');
        }

        while (++i < length) {
            if (i in self) {
                if (typeof T === 'undefined') {
                    callbackfn(self[i], i, object);
                } else {
                    callbackfn.call(T, self[i], i, object);
                }
            }
        }
    }
}, !properlyBoxesContext(ArrayPrototype.forEach));

defineProperties(ArrayPrototype, {
    map: function map(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);
        var result = $Array(length);
        var T;
        if (arguments.length > 1) {
            T = arguments[1];
        }

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.map callback must be a function');
        }

        for (var i = 0; i < length; i++) {
            if (i in self) {
                if (typeof T === 'undefined') {
                    result[i] = callbackfn(self[i], i, object);
                } else {
                    result[i] = callbackfn.call(T, self[i], i, object);
                }
            }
        }
        return result;
    }
}, !properlyBoxesContext(ArrayPrototype.map));

defineProperties(ArrayPrototype, {
    filter: function filter(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);
        var result = [];
        var value;
        var T;
        if (arguments.length > 1) {
            T = arguments[1];
        }

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.filter callback must be a function');
        }

        for (var i = 0; i < length; i++) {
            if (i in self) {
                value = self[i];
                if (typeof T === 'undefined' ? callbackfn(value, i, object) : callbackfn.call(T, value, i, object)) {
                    pushCall(result, value);
                }
            }
        }
        return result;
    }
}, !properlyBoxesContext(ArrayPrototype.filter));

defineProperties(ArrayPrototype, {
    every: function every(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);
        var T;
        if (arguments.length > 1) {
            T = arguments[1];
        }

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.every callback must be a function');
        }

        for (var i = 0; i < length; i++) {
            if (i in self && !(typeof T === 'undefined' ? callbackfn(self[i], i, object) : callbackfn.call(T, self[i], i, object))) {
                return false;
            }
        }
        return true;
    }
}, !properlyBoxesContext(ArrayPrototype.every));

defineProperties(ArrayPrototype, {
    some: function some(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);
        var T;
        if (arguments.length > 1) {
            T = arguments[1];
        }

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.some callback must be a function');
        }

        for (var i = 0; i < length; i++) {
            if (i in self && (typeof T === 'undefined' ? callbackfn(self[i], i, object) : callbackfn.call(T, self[i], i, object))) {
                return true;
            }
        }
        return false;
    }
}, !properlyBoxesContext(ArrayPrototype.some));

var reduceCoercesToObject = false;
if (ArrayPrototype.reduce) {
    reduceCoercesToObject = typeof ArrayPrototype.reduce.call('es5', function (_, __, ___, list) { return list; }) === 'object';
}
defineProperties(ArrayPrototype, {
    reduce: function reduce(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.reduce callback must be a function');
        }

        if (length === 0 && arguments.length === 1) {
            throw new TypeError('reduce of empty array with no initial value');
        }

        var i = 0;
        var result;
        if (arguments.length >= 2) {
            result = arguments[1];
        } else {
            do {
                if (i in self) {
                    result = self[i++];
                    break;
                }

                if (++i >= length) {
                    throw new TypeError('reduce of empty array with no initial value');
                }
            } while (true);
        }

        for (; i < length; i++) {
            if (i in self) {
                result = callbackfn(result, self[i], i, object);
            }
        }

        return result;
    }
}, !reduceCoercesToObject);

var reduceRightCoercesToObject = false;
if (ArrayPrototype.reduceRight) {
    reduceRightCoercesToObject = typeof ArrayPrototype.reduceRight.call('es5', function (_, __, ___, list) { return list; }) === 'object';
}
defineProperties(ArrayPrototype, {
    reduceRight: function reduceRight(callbackfn) {
        var object = ES.ToObject(this);
        var self = splitString && isString(this) ? strSplit(this, '') : object;
        var length = ES.ToUint32(self.length);

        if (!isCallable(callbackfn)) {
            throw new TypeError('Array.prototype.reduceRight callback must be a function');
        }

        if (length === 0 && arguments.length === 1) {
            throw new TypeError('reduceRight of empty array with no initial value');
        }

        var result;
        var i = length - 1;
        if (arguments.length >= 2) {
            result = arguments[1];
        } else {
            do {
                if (i in self) {
                    result = self[i--];
                    break;
                }

                if (--i < 0) {
                    throw new TypeError('reduceRight of empty array with no initial value');
                }
            } while (true);
        }

        if (i < 0) {
            return result;
        }

        do {
            if (i in self) {
                result = callbackfn(result, self[i], i, object);
            }
        } while (i--);

        return result;
    }
}, !reduceRightCoercesToObject);

var hasFirefox2IndexOfBug = ArrayPrototype.indexOf && [0, 1].indexOf(1, 2) !== -1;
defineProperties(ArrayPrototype, {
    indexOf: function indexOf(searchElement) {
        var self = splitString && isString(this) ? strSplit(this, '') : ES.ToObject(this);
        var length = ES.ToUint32(self.length);

        if (length === 0) {
            return -1;
        }

        var i = 0;
        if (arguments.length > 1) {
            i = ES.ToInteger(arguments[1]);
        }

        i = i >= 0 ? i : max(0, length + i);
        for (; i < length; i++) {
            if (i in self && self[i] === searchElement) {
                return i;
            }
        }
        return -1;
    }
}, hasFirefox2IndexOfBug);

var hasFirefox2LastIndexOfBug = ArrayPrototype.lastIndexOf && [0, 1].lastIndexOf(0, -3) !== -1;
defineProperties(ArrayPrototype, {
    lastIndexOf: function lastIndexOf(searchElement) {
        var self = splitString && isString(this) ? strSplit(this, '') : ES.ToObject(this);
        var length = ES.ToUint32(self.length);

        if (length === 0) {
            return -1;
        }
        var i = length - 1;
        if (arguments.length > 1) {
            i = min(i, ES.ToInteger(arguments[1]));
        }
        i = i >= 0 ? i : length - Math.abs(i);
        for (; i >= 0; i--) {
            if (i in self && searchElement === self[i]) {
                return i;
            }
        }
        return -1;
    }
}, hasFirefox2LastIndexOfBug);

var spliceNoopReturnsEmptyArray = (function () {
    var a = [1, 2];
    var result = a.splice();
    return a.length === 2 && isArray(result) && result.length === 0;
}());
defineProperties(ArrayPrototype, {
    splice: function splice(start, deleteCount) {
        if (arguments.length === 0) {
            return [];
        } else {
            return array_splice.apply(this, arguments);
        }
    }
}, !spliceNoopReturnsEmptyArray);

var spliceWorksWithEmptyObject = (function () {
    var obj = {};
    ArrayPrototype.splice.call(obj, 0, 0, 1);
    return obj.length === 1;
}());
defineProperties(ArrayPrototype, {
    splice: function splice(start, deleteCount) {
        if (arguments.length === 0) { return []; }
        var args = arguments;
        this.length = max(ES.ToInteger(this.length), 0);
        if (arguments.length > 0 && typeof deleteCount !== 'number') {
            args = arraySlice(arguments);
            if (args.length < 2) {
                pushCall(args, this.length - start);
            } else {
                args[1] = ES.ToInteger(deleteCount);
            }
        }
        return array_splice.apply(this, args);
    }
}, !spliceWorksWithEmptyObject);
var spliceWorksWithLargeSparseArrays = (function () {
    var arr = new $Array(1e5);
    arr[8] = 'x';
    arr.splice(1, 1);
    return arr.indexOf('x') === 7;
}());
var spliceWorksWithSmallSparseArrays = (function () {
    var n = 256;
    var arr = [];
    arr[n] = 'a';
    arr.splice(n + 1, 0, 'b');
    return arr[n] === 'a';
}());
defineProperties(ArrayPrototype, {
    splice: function splice(start, deleteCount) {
        var O = ES.ToObject(this);
        var A = [];
        var len = ES.ToUint32(O.length);
        var relativeStart = ES.ToInteger(start);
        var actualStart = relativeStart < 0 ? max((len + relativeStart), 0) : min(relativeStart, len);
        var actualDeleteCount = min(max(ES.ToInteger(deleteCount), 0), len - actualStart);

        var k = 0;
        var from;
        while (k < actualDeleteCount) {
            from = $String(actualStart + k);
            if (owns(O, from)) {
                A[k] = O[from];
            }
            k += 1;
        }

        var items = arraySlice(arguments, 2);
        var itemCount = items.length;
        var to;
        if (itemCount < actualDeleteCount) {
            k = actualStart;
            var maxK = len - actualDeleteCount;
            while (k < maxK) {
                from = $String(k + actualDeleteCount);
                to = $String(k + itemCount);
                if (owns(O, from)) {
                    O[to] = O[from];
                } else {
                    delete O[to];
                }
                k += 1;
            }
            k = len;
            var minK = len - actualDeleteCount + itemCount;
            while (k > minK) {
                delete O[k - 1];
                k -= 1;
            }
        } else if (itemCount > actualDeleteCount) {
            k = len - actualDeleteCount;
            while (k > actualStart) {
                from = $String(k + actualDeleteCount - 1);
                to = $String(k + itemCount - 1);
                if (owns(O, from)) {
                    O[to] = O[from];
                } else {
                    delete O[to];
                }
                k -= 1;
            }
        }
        k = actualStart;
        for (var i = 0; i < items.length; ++i) {
            O[k] = items[i];
            k += 1;
        }
        O.length = len - actualDeleteCount + itemCount;

        return A;
    }
}, !spliceWorksWithLargeSparseArrays || !spliceWorksWithSmallSparseArrays);

var originalJoin = ArrayPrototype.join;
var hasStringJoinBug;
try {
    hasStringJoinBug = Array.prototype.join.call('123', ',') !== '1,2,3';
} catch (e) {
    hasStringJoinBug = true;
}
if (hasStringJoinBug) {
    defineProperties(ArrayPrototype, {
        join: function join(separator) {
            var sep = typeof separator === 'undefined' ? ',' : separator;
            return originalJoin.call(isString(this) ? strSplit(this, '') : this, sep);
        }
    }, hasStringJoinBug);
}

var hasJoinUndefinedBug = [1, 2].join(undefined) !== '1,2';
if (hasJoinUndefinedBug) {
    defineProperties(ArrayPrototype, {
        join: function join(separator) {
            var sep = typeof separator === 'undefined' ? ',' : separator;
            return originalJoin.call(this, sep);
        }
    }, hasJoinUndefinedBug);
}

var pushShim = function push(item) {
    var O = ES.ToObject(this);
    var n = ES.ToUint32(O.length);
    var i = 0;
    while (i < arguments.length) {
        O[n + i] = arguments[i];
        i += 1;
    }
    O.length = n + i;
    return n + i;
};

var pushIsNotGeneric = (function () {
    var obj = {};
    var result = Array.prototype.push.call(obj, undefined);
    return result !== 1 || obj.length !== 1 || typeof obj[0] !== 'undefined' || !owns(obj, 0);
}());
defineProperties(ArrayPrototype, {
    push: function push(item) {
        if (isArray(this)) {
            return array_push.apply(this, arguments);
        }
        return pushShim.apply(this, arguments);
    }
}, pushIsNotGeneric);

var pushUndefinedIsWeird = (function () {
    var arr = [];
    var result = arr.push(undefined);
    return result !== 1 || arr.length !== 1 || typeof arr[0] !== 'undefined' || !owns(arr, 0);
}());
defineProperties(ArrayPrototype, { push: pushShim }, pushUndefinedIsWeird);

defineProperties(ArrayPrototype, {
    slice: function (start, end) {
        var arr = isString(this) ? strSplit(this, '') : this;
        return arraySliceApply(arr, arguments);
    }
}, splitString);

var sortIgnoresNonFunctions = (function () {
    try {
        [1, 2].sort(null);
        [1, 2].sort({});
        return true;
    } catch (e) {}
    return false;
}());
var sortThrowsOnRegex = (function () {
    try {
        [1, 2].sort(/a/);
        return false;
    } catch (e) {}
    return true;
}());
var sortIgnoresUndefined = (function () {
    try {
        [1, 2].sort(undefined);
        return true;
    } catch (e) {}
    return false;
}());
defineProperties(ArrayPrototype, {
    sort: function sort(compareFn) {
        if (typeof compareFn === 'undefined') {
            return arraySort(this);
        }
        if (!isCallable(compareFn)) {
            throw new TypeError('Array.prototype.sort callback must be a function');
        }
        return arraySort(this, compareFn);
    }
}, sortIgnoresNonFunctions || !sortIgnoresUndefined || !sortThrowsOnRegex);



var hasDontEnumBug = !({ 'toString': null }).propertyIsEnumerable('toString');
var hasProtoEnumBug = function () {}.propertyIsEnumerable('prototype');
var hasStringEnumBug = !owns('x', '0');
var equalsConstructorPrototype = function (o) {
    var ctor = o.constructor;
    return ctor && ctor.prototype === o;
};
var blacklistedKeys = {
    $window: true,
    $console: true,
    $parent: true,
    $self: true,
    $frame: true,
    $frames: true,
    $frameElement: true,
    $webkitIndexedDB: true,
    $webkitStorageInfo: true,
    $external: true
};
var hasAutomationEqualityBug = (function () {
    if (typeof window === 'undefined') { return false; }
    for (var k in window) {
        try {
            if (!blacklistedKeys['$' + k] && owns(window, k) && window[k] !== null && typeof window[k] === 'object') {
                equalsConstructorPrototype(window[k]);
            }
        } catch (e) {
            return true;
        }
    }
    return false;
}());
var equalsConstructorPrototypeIfNotBuggy = function (object) {
    if (typeof window === 'undefined' || !hasAutomationEqualityBug) { return equalsConstructorPrototype(object); }
    try {
        return equalsConstructorPrototype(object);
    } catch (e) {
        return false;
    }
};
var dontEnums = [
    'toString',
    'toLocaleString',
    'valueOf',
    'hasOwnProperty',
    'isPrototypeOf',
    'propertyIsEnumerable',
    'constructor'
];
var dontEnumsLength = dontEnums.length;

var isStandardArguments = function isArguments(value) {
    return toStr(value) === '[object Arguments]';
};
var isLegacyArguments = function isArguments(value) {
    return value !== null &&
        typeof value === 'object' &&
        typeof value.length === 'number' &&
        value.length >= 0 &&
        !isArray(value) &&
        isCallable(value.callee);
};
var isArguments = isStandardArguments(arguments) ? isStandardArguments : isLegacyArguments;

defineProperties($Object, {
    keys: function keys(object) {
        var isFn = isCallable(object);
        var isArgs = isArguments(object);
        var isObject = object !== null && typeof object === 'object';
        var isStr = isObject && isString(object);

        if (!isObject && !isFn && !isArgs) {
            throw new TypeError('Object.keys called on a non-object');
        }

        var theKeys = [];
        var skipProto = hasProtoEnumBug && isFn;
        if ((isStr && hasStringEnumBug) || isArgs) {
            for (var i = 0; i < object.length; ++i) {
                pushCall(theKeys, $String(i));
            }
        }

        if (!isArgs) {
            for (var name in object) {
                if (!(skipProto && name === 'prototype') && owns(object, name)) {
                    pushCall(theKeys, $String(name));
                }
            }
        }

        if (hasDontEnumBug) {
            var skipConstructor = equalsConstructorPrototypeIfNotBuggy(object);
            for (var j = 0; j < dontEnumsLength; j++) {
                var dontEnum = dontEnums[j];
                if (!(skipConstructor && dontEnum === 'constructor') && owns(object, dontEnum)) {
                    pushCall(theKeys, dontEnum);
                }
            }
        }
        return theKeys;
    }
});

var keysWorksWithArguments = $Object.keys && (function () {
    return $Object.keys(arguments).length === 2;
}(1, 2));
var keysHasArgumentsLengthBug = $Object.keys && (function () {
    var argKeys = $Object.keys(arguments);
    return arguments.length !== 1 || argKeys.length !== 1 || argKeys[0] !== 1;
}(1));
var originalKeys = $Object.keys;
defineProperties($Object, {
    keys: function keys(object) {
        if (isArguments(object)) {
            return originalKeys(arraySlice(object));
        } else {
            return originalKeys(object);
        }
    }
}, !keysWorksWithArguments || keysHasArgumentsLengthBug);


var hasNegativeMonthYearBug = new Date(-3509827329600292).getUTCMonth() !== 0;
var aNegativeTestDate = new Date(-1509842289600292);
var aPositiveTestDate = new Date(1449662400000);
var hasToUTCStringFormatBug = aNegativeTestDate.toUTCString() !== 'Mon, 01 Jan -45875 11:59:59 GMT';
var hasToDateStringFormatBug;
var hasToStringFormatBug;
var timeZoneOffset = aNegativeTestDate.getTimezoneOffset();
if (timeZoneOffset < -720) {
    hasToDateStringFormatBug = aNegativeTestDate.toDateString() !== 'Tue Jan 02 -45875';
    hasToStringFormatBug = !(/^Thu Dec 10 2015 \d\d:\d\d:\d\d GMT[-\+]\d\d\d\d(?: |$)/).test(aPositiveTestDate.toString());
} else {
    hasToDateStringFormatBug = aNegativeTestDate.toDateString() !== 'Mon Jan 01 -45875';
    hasToStringFormatBug = !(/^Wed Dec 09 2015 \d\d:\d\d:\d\d GMT[-\+]\d\d\d\d(?: |$)/).test(aPositiveTestDate.toString());
}

var originalGetFullYear = call.bind(Date.prototype.getFullYear);
var originalGetMonth = call.bind(Date.prototype.getMonth);
var originalGetDate = call.bind(Date.prototype.getDate);
var originalGetUTCFullYear = call.bind(Date.prototype.getUTCFullYear);
var originalGetUTCMonth = call.bind(Date.prototype.getUTCMonth);
var originalGetUTCDate = call.bind(Date.prototype.getUTCDate);
var originalGetUTCDay = call.bind(Date.prototype.getUTCDay);
var originalGetUTCHours = call.bind(Date.prototype.getUTCHours);
var originalGetUTCMinutes = call.bind(Date.prototype.getUTCMinutes);
var originalGetUTCSeconds = call.bind(Date.prototype.getUTCSeconds);
var originalGetUTCMilliseconds = call.bind(Date.prototype.getUTCMilliseconds);
var dayName = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
var monthName = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
var daysInMonth = function daysInMonth(month, year) {
    return originalGetDate(new Date(year, month, 0));
};

defineProperties(Date.prototype, {
    getFullYear: function getFullYear() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetFullYear(this);
        if (year < 0 && originalGetMonth(this) > 11) {
            return year + 1;
        }
        return year;
    },
    getMonth: function getMonth() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetFullYear(this);
        var month = originalGetMonth(this);
        if (year < 0 && month > 11) {
            return 0;
        }
        return month;
    },
    getDate: function getDate() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetFullYear(this);
        var month = originalGetMonth(this);
        var date = originalGetDate(this);
        if (year < 0 && month > 11) {
            if (month === 12) {
                return date;
            }
            var days = daysInMonth(0, year + 1);
            return (days - date) + 1;
        }
        return date;
    },
    getUTCFullYear: function getUTCFullYear() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetUTCFullYear(this);
        if (year < 0 && originalGetUTCMonth(this) > 11) {
            return year + 1;
        }
        return year;
    },
    getUTCMonth: function getUTCMonth() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetUTCFullYear(this);
        var month = originalGetUTCMonth(this);
        if (year < 0 && month > 11) {
            return 0;
        }
        return month;
    },
    getUTCDate: function getUTCDate() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var year = originalGetUTCFullYear(this);
        var month = originalGetUTCMonth(this);
        var date = originalGetUTCDate(this);
        if (year < 0 && month > 11) {
            if (month === 12) {
                return date;
            }
            var days = daysInMonth(0, year + 1);
            return (days - date) + 1;
        }
        return date;
    }
}, hasNegativeMonthYearBug);

defineProperties(Date.prototype, {
    toUTCString: function toUTCString() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var day = originalGetUTCDay(this);
        var date = originalGetUTCDate(this);
        var month = originalGetUTCMonth(this);
        var year = originalGetUTCFullYear(this);
        var hour = originalGetUTCHours(this);
        var minute = originalGetUTCMinutes(this);
        var second = originalGetUTCSeconds(this);
        return dayName[day] + ', ' +
            (date < 10 ? '0' + date : date) + ' ' +
            monthName[month] + ' ' +
            year + ' ' +
            (hour < 10 ? '0' + hour : hour) + ':' +
            (minute < 10 ? '0' + minute : minute) + ':' +
            (second < 10 ? '0' + second : second) + ' GMT';
    }
}, hasNegativeMonthYearBug || hasToUTCStringFormatBug);

defineProperties(Date.prototype, {
    toDateString: function toDateString() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var day = this.getDay();
        var date = this.getDate();
        var month = this.getMonth();
        var year = this.getFullYear();
        return dayName[day] + ' ' +
            monthName[month] + ' ' +
            (date < 10 ? '0' + date : date) + ' ' +
            year;
    }
}, hasNegativeMonthYearBug || hasToDateStringFormatBug);

if (hasNegativeMonthYearBug || hasToStringFormatBug) {
    Date.prototype.toString = function toString() {
        if (!this || !(this instanceof Date)) {
            throw new TypeError('this is not a Date object.');
        }
        var day = this.getDay();
        var date = this.getDate();
        var month = this.getMonth();
        var year = this.getFullYear();
        var hour = this.getHours();
        var minute = this.getMinutes();
        var second = this.getSeconds();
        var timezoneOffset = this.getTimezoneOffset();
        var hoursOffset = Math.floor(Math.abs(timezoneOffset) / 60);
        var minutesOffset = Math.floor(Math.abs(timezoneOffset) % 60);
        return dayName[day] + ' ' +
            monthName[month] + ' ' +
            (date < 10 ? '0' + date : date) + ' ' +
            year + ' ' +
            (hour < 10 ? '0' + hour : hour) + ':' +
            (minute < 10 ? '0' + minute : minute) + ':' +
            (second < 10 ? '0' + second : second) + ' GMT' +
            (timezoneOffset > 0 ? '-' : '+') +
            (hoursOffset < 10 ? '0' + hoursOffset : hoursOffset) +
            (minutesOffset < 10 ? '0' + minutesOffset : minutesOffset);
    };
    if (supportsDescriptors) {
        $Object.defineProperty(Date.prototype, 'toString', {
            configurable: true,
            enumerable: false,
            writable: true
        });
    }
}

var negativeDate = -62198755200000;
var negativeYearString = '-000001';
var hasNegativeDateBug = Date.prototype.toISOString && new Date(negativeDate).toISOString().indexOf(negativeYearString) === -1;
var hasSafari51DateBug = Date.prototype.toISOString && new Date(-1).toISOString() !== '1969-12-31T23:59:59.999Z';

var getTime = call.bind(Date.prototype.getTime);

defineProperties(Date.prototype, {
    toISOString: function toISOString() {
        if (!isFinite(this) || !isFinite(getTime(this))) {
            throw new RangeError('Date.prototype.toISOString called on non-finite value.');
        }

        var year = originalGetUTCFullYear(this);

        var month = originalGetUTCMonth(this);
        year += Math.floor(month / 12);
        month = (month % 12 + 12) % 12;

        var result = [month + 1, originalGetUTCDate(this), originalGetUTCHours(this), originalGetUTCMinutes(this), originalGetUTCSeconds(this)];
        year = (
            (year < 0 ? '-' : (year > 9999 ? '+' : '')) +
            strSlice('00000' + Math.abs(year), (0 <= year && year <= 9999) ? -4 : -6)
        );

        for (var i = 0; i < result.length; ++i) {
          result[i] = strSlice('00' + result[i], -2);
        }
        return (
            year + '-' + arraySlice(result, 0, 2).join('-') +
            'T' + arraySlice(result, 2).join(':') + '.' +
            strSlice('000' + originalGetUTCMilliseconds(this), -3) + 'Z'
        );
    }
}, hasNegativeDateBug || hasSafari51DateBug);

var dateToJSONIsSupported = (function () {
    try {
        return Date.prototype.toJSON &&
            new Date(NaN).toJSON() === null &&
            new Date(negativeDate).toJSON().indexOf(negativeYearString) !== -1 &&
            Date.prototype.toJSON.call({
                toISOString: function () { return true; }
            });
    } catch (e) {
        return false;
    }
}());
if (!dateToJSONIsSupported) {
    Date.prototype.toJSON = function toJSON(key) {

        var O = $Object(this);
        var tv = ES.ToPrimitive(O);
        if (typeof tv === 'number' && !isFinite(tv)) {
            return null;
        }
        var toISO = O.toISOString;
        if (!isCallable(toISO)) {
            throw new TypeError('toISOString property is not callable');
        }
        return toISO.call(O);


    };
}

var supportsExtendedYears = Date.parse('+033658-09-27T01:46:40.000Z') === 1e15;
var acceptsInvalidDates = !isNaN(Date.parse('2012-04-04T24:00:00.500Z')) || !isNaN(Date.parse('2012-11-31T23:59:59.000Z')) || !isNaN(Date.parse('2012-12-31T23:59:60.000Z'));
var doesNotParseY2KNewYear = isNaN(Date.parse('2000-01-01T00:00:00.000Z'));
if (doesNotParseY2KNewYear || acceptsInvalidDates || !supportsExtendedYears) {
    var maxSafeUnsigned32Bit = Math.pow(2, 31) - 1;
    var hasSafariSignedIntBug = isActualNaN(new Date(1970, 0, 1, 0, 0, 0, maxSafeUnsigned32Bit + 1).getTime());
    Date = (function (NativeDate) {
        var DateShim = function Date(Y, M, D, h, m, s, ms) {
            var length = arguments.length;
            var date;
            if (this instanceof NativeDate) {
                var seconds = s;
                var millis = ms;
                if (hasSafariSignedIntBug && length >= 7 && ms > maxSafeUnsigned32Bit) {
                    var msToShift = Math.floor(ms / maxSafeUnsigned32Bit) * maxSafeUnsigned32Bit;
                    var sToShift = Math.floor(msToShift / 1e3);
                    seconds += sToShift;
                    millis -= sToShift * 1e3;
                }
                date = length === 1 && $String(Y) === Y ?
                    new NativeDate(DateShim.parse(Y)) :
                    length >= 7 ? new NativeDate(Y, M, D, h, m, seconds, millis) :
                    length >= 6 ? new NativeDate(Y, M, D, h, m, seconds) :
                    length >= 5 ? new NativeDate(Y, M, D, h, m) :
                    length >= 4 ? new NativeDate(Y, M, D, h) :
                    length >= 3 ? new NativeDate(Y, M, D) :
                    length >= 2 ? new NativeDate(Y, M) :
                    length >= 1 ? new NativeDate(Y) :
                                  new NativeDate();
            } else {
                date = NativeDate.apply(this, arguments);
            }
            if (!isPrimitive(date)) {
              defineProperties(date, { constructor: DateShim }, true);
            }
            return date;
        };

        var isoDateExpression = new RegExp('^' +
            '(\\d{4}|[+-]\\d{6})' +
            '(?:-(\\d{2})' +
            '(?:-(\\d{2})' +
            '(?:' +
                'T(\\d{2})' +
                ':(\\d{2})' +
                '(?:' +
                    ':(\\d{2})' +
                    '(?:(\\.\\d{1,}))?' +
                ')?' +
            '(' +
                'Z|' +
                '(?:' +
                    '([-+])' +
                    '(\\d{2})' +
                    ':(\\d{2})' +
                ')' +
            ')?)?)?)?' +
        '$');

        var months = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365];

        var dayFromMonth = function dayFromMonth(year, month) {
            var t = month > 1 ? 1 : 0;
            return (
                months[month] +
                Math.floor((year - 1969 + t) / 4) -
                Math.floor((year - 1901 + t) / 100) +
                Math.floor((year - 1601 + t) / 400) +
                365 * (year - 1970)
            );
        };

        var toUTC = function toUTC(t) {
            var s = 0;
            var ms = t;
            if (hasSafariSignedIntBug && ms > maxSafeUnsigned32Bit) {
                var msToShift = Math.floor(ms / maxSafeUnsigned32Bit) * maxSafeUnsigned32Bit;
                var sToShift = Math.floor(msToShift / 1e3);
                s += sToShift;
                ms -= sToShift * 1e3;
            }
            return $Number(new NativeDate(1970, 0, 1, 0, 0, s, ms));
        };

        for (var key in NativeDate) {
            if (owns(NativeDate, key)) {
                DateShim[key] = NativeDate[key];
            }
        }

        defineProperties(DateShim, {
            now: NativeDate.now,
            UTC: NativeDate.UTC
        }, true);
        DateShim.prototype = NativeDate.prototype;
        defineProperties(DateShim.prototype, {
            constructor: DateShim
        }, true);

        var parseShim = function parse(string) {
            var match = isoDateExpression.exec(string);
            if (match) {
                var year = $Number(match[1]),
                    month = $Number(match[2] || 1) - 1,
                    day = $Number(match[3] || 1) - 1,
                    hour = $Number(match[4] || 0),
                    minute = $Number(match[5] || 0),
                    second = $Number(match[6] || 0),
                    millisecond = Math.floor($Number(match[7] || 0) * 1000),
                    isLocalTime = Boolean(match[4] && !match[8]),
                    signOffset = match[9] === '-' ? 1 : -1,
                    hourOffset = $Number(match[10] || 0),
                    minuteOffset = $Number(match[11] || 0),
                    result;
                var hasMinutesOrSecondsOrMilliseconds = minute > 0 || second > 0 || millisecond > 0;
                if (
                    hour < (hasMinutesOrSecondsOrMilliseconds ? 24 : 25) &&
                    minute < 60 && second < 60 && millisecond < 1000 &&
                    month > -1 && month < 12 && hourOffset < 24 &&
                    minuteOffset < 60 &&
                    day > -1 &&
                    day < (dayFromMonth(year, month + 1) - dayFromMonth(year, month))
                ) {
                    result = (
                        (dayFromMonth(year, month) + day) * 24 +
                        hour +
                        hourOffset * signOffset
                    ) * 60;
                    result = (
                        (result + minute + minuteOffset * signOffset) * 60 +
                        second
                    ) * 1000 + millisecond;
                    if (isLocalTime) {
                        result = toUTC(result);
                    }
                    if (-8.64e15 <= result && result <= 8.64e15) {
                        return result;
                    }
                }
                return NaN;
            }
            return NativeDate.parse.apply(this, arguments);
        };
        defineProperties(DateShim, { parse: parseShim });

        return DateShim;
    }(Date));
}

if (!Date.now) {
    Date.now = function now() {
        return new Date().getTime();
    };
}


var hasToFixedBugs = NumberPrototype.toFixed && (
  (0.00008).toFixed(3) !== '0.000' ||
  (0.9).toFixed(0) !== '1' ||
  (1.255).toFixed(2) !== '1.25' ||
  (1000000000000000128).toFixed(0) !== '1000000000000000128'
);

var toFixedHelpers = {
  base: 1e7,
  size: 6,
  data: [0, 0, 0, 0, 0, 0],
  multiply: function multiply(n, c) {
      var i = -1;
      var c2 = c;
      while (++i < toFixedHelpers.size) {
          c2 += n * toFixedHelpers.data[i];
          toFixedHelpers.data[i] = c2 % toFixedHelpers.base;
          c2 = Math.floor(c2 / toFixedHelpers.base);
      }
  },
  divide: function divide(n) {
      var i = toFixedHelpers.size;
      var c = 0;
      while (--i >= 0) {
          c += toFixedHelpers.data[i];
          toFixedHelpers.data[i] = Math.floor(c / n);
          c = (c % n) * toFixedHelpers.base;
      }
  },
  numToString: function numToString() {
      var i = toFixedHelpers.size;
      var s = '';
      while (--i >= 0) {
          if (s !== '' || i === 0 || toFixedHelpers.data[i] !== 0) {
              var t = $String(toFixedHelpers.data[i]);
              if (s === '') {
                  s = t;
              } else {
                  s += strSlice('0000000', 0, 7 - t.length) + t;
              }
          }
      }
      return s;
  },
  pow: function pow(x, n, acc) {
      return (n === 0 ? acc : (n % 2 === 1 ? pow(x, n - 1, acc * x) : pow(x * x, n / 2, acc)));
  },
  log: function log(x) {
      var n = 0;
      var x2 = x;
      while (x2 >= 4096) {
          n += 12;
          x2 /= 4096;
      }
      while (x2 >= 2) {
          n += 1;
          x2 /= 2;
      }
      return n;
  }
};

var toFixedShim = function toFixed(fractionDigits) {
    var f, x, s, m, e, z, j, k;

    f = $Number(fractionDigits);
    f = isActualNaN(f) ? 0 : Math.floor(f);

    if (f < 0 || f > 20) {
        throw new RangeError('Number.toFixed called with invalid number of decimals');
    }

    x = $Number(this);

    if (isActualNaN(x)) {
        return 'NaN';
    }

    if (x <= -1e21 || x >= 1e21) {
        return $String(x);
    }

    s = '';

    if (x < 0) {
        s = '-';
        x = -x;
    }

    m = '0';

    if (x > 1e-21) {
        e = toFixedHelpers.log(x * toFixedHelpers.pow(2, 69, 1)) - 69;
        z = (e < 0 ? x * toFixedHelpers.pow(2, -e, 1) : x / toFixedHelpers.pow(2, e, 1));
        z *= 0x10000000000000;
        e = 52 - e;

        if (e > 0) {
            toFixedHelpers.multiply(0, z);
            j = f;

            while (j >= 7) {
                toFixedHelpers.multiply(1e7, 0);
                j -= 7;
            }

            toFixedHelpers.multiply(toFixedHelpers.pow(10, j, 1), 0);
            j = e - 1;

            while (j >= 23) {
                toFixedHelpers.divide(1 << 23);
                j -= 23;
            }

            toFixedHelpers.divide(1 << j);
            toFixedHelpers.multiply(1, 1);
            toFixedHelpers.divide(2);
            m = toFixedHelpers.numToString();
        } else {
            toFixedHelpers.multiply(0, z);
            toFixedHelpers.multiply(1 << (-e), 0);
            m = toFixedHelpers.numToString() + strSlice('0.00000000000000000000', 2, 2 + f);
        }
    }

    if (f > 0) {
        k = m.length;

        if (k <= f) {
            m = s + strSlice('0.0000000000000000000', 0, f - k + 2) + m;
        } else {
            m = s + strSlice(m, 0, k - f) + '.' + strSlice(m, k - f);
        }
    } else {
        m = s + m;
    }

    return m;
};
defineProperties(NumberPrototype, { toFixed: toFixedShim }, hasToFixedBugs);

var hasToPrecisionUndefinedBug = (function () {
    try {
        return 1.0.toPrecision(undefined) === '1';
    } catch (e) {
        return true;
    }
}());
var originalToPrecision = NumberPrototype.toPrecision;
defineProperties(NumberPrototype, {
    toPrecision: function toPrecision(precision) {
        return typeof precision === 'undefined' ? originalToPrecision.call(this) : originalToPrecision.call(this, precision);
    }
}, hasToPrecisionUndefinedBug);




if (
    'ab'.split(/(?:ab)*/).length !== 2 ||
    '.'.split(/(.?)(.?)/).length !== 4 ||
    'tesst'.split(/(s)*/)[1] === 't' ||
    'test'.split(/(?:)/, -1).length !== 4 ||
    ''.split(/.?/).length ||
    '.'.split(/()()/).length > 1
) {
    (function () {
        var compliantExecNpcg = typeof (/()??/).exec('')[1] === 'undefined';
        var maxSafe32BitInt = Math.pow(2, 32) - 1;

        StringPrototype.split = function (separator, limit) {
            var string = String(this);
            if (typeof separator === 'undefined' && limit === 0) {
                return [];
            }

            if (!isRegex(separator)) {
                return strSplit(this, separator, limit);
            }

            var output = [];
            var flags = (separator.ignoreCase ? 'i' : '') +
                        (separator.multiline ? 'm' : '') +
                        (separator.unicode ? 'u' : '') +
                        (separator.sticky ? 'y' : ''),
                lastLastIndex = 0,
                separator2, match, lastIndex, lastLength;
            var separatorCopy = new RegExp(separator.source, flags + 'g');
            if (!compliantExecNpcg) {
                separator2 = new RegExp('^' + separatorCopy.source + '$(?!\\s)', flags);
            }
            var splitLimit = typeof limit === 'undefined' ? maxSafe32BitInt : ES.ToUint32(limit);
            match = separatorCopy.exec(string);
            while (match) {
                lastIndex = match.index + match[0].length;
                if (lastIndex > lastLastIndex) {
                    pushCall(output, strSlice(string, lastLastIndex, match.index));
                    if (!compliantExecNpcg && match.length > 1) {
                        match[0].replace(separator2, function () {
                            for (var i = 1; i < arguments.length - 2; i++) {
                                if (typeof arguments[i] === 'undefined') {
                                    match[i] = void 0;
                                }
                            }
                        });
                    }
                    if (match.length > 1 && match.index < string.length) {
                        array_push.apply(output, arraySlice(match, 1));
                    }
                    lastLength = match[0].length;
                    lastLastIndex = lastIndex;
                    if (output.length >= splitLimit) {
                        break;
                    }
                }
                if (separatorCopy.lastIndex === match.index) {
                    separatorCopy.lastIndex++;
                }
                match = separatorCopy.exec(string);
            }
            if (lastLastIndex === string.length) {
                if (lastLength || !separatorCopy.test('')) {
                    pushCall(output, '');
                }
            } else {
                pushCall(output, strSlice(string, lastLastIndex));
            }
            return output.length > splitLimit ? arraySlice(output, 0, splitLimit) : output;
        };
    }());

} else if ('0'.split(void 0, 0).length) {
    StringPrototype.split = function split(separator, limit) {
        if (typeof separator === 'undefined' && limit === 0) { return []; }
        return strSplit(this, separator, limit);
    };
}

var str_replace = StringPrototype.replace;
var replaceReportsGroupsCorrectly = (function () {
    var groups = [];
    'x'.replace(/x(.)?/g, function (match, group) {
        pushCall(groups, group);
    });
    return groups.length === 1 && typeof groups[0] === 'undefined';
}());

if (!replaceReportsGroupsCorrectly) {
    StringPrototype.replace = function replace(searchValue, replaceValue) {
        var isFn = isCallable(replaceValue);
        var hasCapturingGroups = isRegex(searchValue) && (/\)[*?]/).test(searchValue.source);
        if (!isFn || !hasCapturingGroups) {
            return str_replace.call(this, searchValue, replaceValue);
        } else {
            var wrappedReplaceValue = function (match) {
                var length = arguments.length;
                var originalLastIndex = searchValue.lastIndex;
                searchValue.lastIndex = 0;
                var args = searchValue.exec(match) || [];
                searchValue.lastIndex = originalLastIndex;
                pushCall(args, arguments[length - 2], arguments[length - 1]);
                return replaceValue.apply(this, args);
            };
            return str_replace.call(this, searchValue, wrappedReplaceValue);
        }
    };
}

var string_substr = StringPrototype.substr;
var hasNegativeSubstrBug = ''.substr && '0b'.substr(-1) !== 'b';
defineProperties(StringPrototype, {
    substr: function substr(start, length) {
        var normalizedStart = start;
        if (start < 0) {
            normalizedStart = max(this.length + start, 0);
        }
        return string_substr.call(this, normalizedStart, length);
    }
}, hasNegativeSubstrBug);

var ws = '\x09\x0A\x0B\x0C\x0D\x20\xA0\u1680\u180E\u2000\u2001\u2002\u2003' +
    '\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u202F\u205F\u3000\u2028' +
    '\u2029\uFEFF';
var zeroWidth = '\u200b';
var wsRegexChars = '[' + ws + ']';
var trimBeginRegexp = new RegExp('^' + wsRegexChars + wsRegexChars + '*');
var trimEndRegexp = new RegExp(wsRegexChars + wsRegexChars + '*$');
var hasTrimWhitespaceBug = StringPrototype.trim && (ws.trim() || !zeroWidth.trim());
defineProperties(StringPrototype, {
    trim: function trim() {
        if (typeof this === 'undefined' || this === null) {
            throw new TypeError("can't convert " + this + ' to object');
        }
        return $String(this).replace(trimBeginRegexp, '').replace(trimEndRegexp, '');
    }
}, hasTrimWhitespaceBug);
var trim = call.bind(String.prototype.trim);

var hasLastIndexBug = StringPrototype.lastIndexOf && 'abcあい'.lastIndexOf('あい', 2) !== -1;
defineProperties(StringPrototype, {
    lastIndexOf: function lastIndexOf(searchString) {
        if (typeof this === 'undefined' || this === null) {
            throw new TypeError("can't convert " + this + ' to object');
        }
        var S = $String(this);
        var searchStr = $String(searchString);
        var numPos = arguments.length > 1 ? $Number(arguments[1]) : NaN;
        var pos = isActualNaN(numPos) ? Infinity : ES.ToInteger(numPos);
        var start = min(max(pos, 0), S.length);
        var searchLen = searchStr.length;
        var k = start + searchLen;
        while (k > 0) {
            k = max(0, k - searchLen);
            var index = strIndexOf(strSlice(S, k, start + searchLen), searchStr);
            if (index !== -1) {
                return k + index;
            }
        }
        return -1;
    }
}, hasLastIndexBug);

var originalLastIndexOf = StringPrototype.lastIndexOf;
defineProperties(StringPrototype, {
    lastIndexOf: function lastIndexOf(searchString) {
        return originalLastIndexOf.apply(this, arguments);
    }
}, StringPrototype.lastIndexOf.length !== 1);

if (parseInt(ws + '08') !== 8 || parseInt(ws + '0x16') !== 22) {
    parseInt = (function (origParseInt) {
        var hexRegex = /^[\-+]?0[xX]/;
        return function parseInt(str, radix) {
            var string = trim(str);
            var defaultedRadix = $Number(radix) || (hexRegex.test(string) ? 16 : 10);
            return origParseInt(string, defaultedRadix);
        };
    }(parseInt));
}

if (1 / parseFloat('-0') !== -Infinity) {
    parseFloat = (function (origParseFloat) {
        return function parseFloat(string) {
            var inputString = trim(string);
            var result = origParseFloat(inputString);
            return result === 0 && strSlice(inputString, 0, 1) === '-' ? -0 : result;
        };
    }(parseFloat));
}

if (String(new RangeError('test')) !== 'RangeError: test') {
    var errorToStringShim = function toString() {
        if (typeof this === 'undefined' || this === null) {
            throw new TypeError("can't convert " + this + ' to object');
        }
        var name = this.name;
        if (typeof name === 'undefined') {
            name = 'Error';
        } else if (typeof name !== 'string') {
            name = $String(name);
        }
        var msg = this.message;
        if (typeof msg === 'undefined') {
            msg = '';
        } else if (typeof msg !== 'string') {
            msg = $String(msg);
        }
        if (!name) {
            return msg;
        }
        if (!msg) {
            return name;
        }
        return name + ': ' + msg;
    };
    Error.prototype.toString = errorToStringShim;
}

if (supportsDescriptors) {
    var ensureNonEnumerable = function (obj, prop) {
        if (isEnum(obj, prop)) {
            var desc = Object.getOwnPropertyDescriptor(obj, prop);
            desc.enumerable = false;
            Object.defineProperty(obj, prop, desc);
        }
    };
    ensureNonEnumerable(Error.prototype, 'message');
    if (Error.prototype.message !== '') {
      Error.prototype.message = '';
    }
    ensureNonEnumerable(Error.prototype, 'name');
}

if (String(/a/mig) !== '/a/gim') {
    var regexToString = function toString() {
        var str = '/' + this.source + '/';
        if (this.global) {
            str += 'g';
        }
        if (this.ignoreCase) {
            str += 'i';
        }
        if (this.multiline) {
            str += 'm';
        }
        return str;
    };
    RegExp.prototype.toString = regexToString;
}

}));
