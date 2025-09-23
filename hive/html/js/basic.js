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
var gDebug = 0;
if (gDebug) {
    alert("WARNING!!\n----------------------------\n" +
        "DEVELOPER ALERT:  loading in debug mode\n In order to change mode\n edit gDebug variable found in basic.js");
}
var gMenuOver = null;

function isok(obj) {
    if (!obj || obj.length == 0) return false;
    return true;
}

function verobj(obj) {
    if (!obj || !obj.length) return 0;
    return obj;
}

function verarr(obj) {
    if (!obj) return new Array();
    if ((obj instanceof Array)) return obj;
    return [obj];
}

function parseBool(s) {
    if (!s)
        return false;
    if (Array.isArray(s))
        return s.length > 0;
    if (typeof (s) == "string")
        return s.length && !s.match(/^(false|off|0)$/i);
    if (typeof (s) == "object") {
        for (var field in s)
            return true;
        return false;
    }
    return true;
}

function parseIBool(s) {
    if (!s) {
        return 0;
    }

    if (s === true) {
        return 1;
    }

    if (Array.isArray(s)) {
        return s.length;
    }

    if (typeof (s) == "string") {
        if (s === "" || s.match(/^(false|off)$/i)) {
            return 0;
        }
    } else if (typeof (s) == "object") {
        var ret = 0;
        for (var field in s) {
            ret++;
        }
        return ret;
    }

    var ret = parseInt(s);
    return isNaN(ret) ? 1 : ret;
}

function setDefaults(obj, defvalue) {
    for (var f in defvalue) {
        if (obj[f] === undefined)
            obj[f] = defvalue[f];
    }
}

function eventStop(event) {
    if (event.stopPropagation) { event.stopPropagation(); }
    event.cancelBubble = true;
    return 0;
}
let parseJSON = function (value) {
    let toReturn;
    try {
        toReturn = JSON.parse(value);
    }
    catch (err) {
        toReturn = value
    }
    return toReturn;
};


function findJsonField(obj, fieldPath, tokenize) {
    if (!fieldPath || typeof (obj) != "object")
        return null;

    var field;
    if (tokenize) {
        fieldPath = fieldPath.split(".");
        field = fieldPath[0];
        fieldPath.splice(0, 1);
    } else
        field = fieldPath;


    for (var f in obj) {
        if (f == field) {
            if (tokenize && fieldPath.length)
                return findJsonField(obj[f], fieldPath.join("."), tokenize);
            return { parent: obj, field: f, node: obj[f] };
        }
        else if (!tokenize) {
            var res = findJsonField(obj[f], fieldPath);
            if (res)
                return res
        }
    }
    return null;
}


function parseHiveId(id) {
    ret = { domainId: "", objId: 0, ionId: 0, modify: false };

    if (typeof (id) == "number") {
        ret.objId = id ? parseInt(id) : 0;
    } if (typeof (id) == "string") {
        var match = id.match(/^\s*(-)?(?:(https?:\/\/[\]\[a-zA-Z0-9:/?#@!$&'()*+,;=._~-]{1,2040})\||(?:([_a-zA-Z][a-zA-Z0-9]{0,7})\.))?(\d+)(?:\.(\d+))?\s*$/);
        if (match) {
            ret.modify = match[1] ? true : false;
            if (match[2]) {
                ret.domainId = match[2];
            } else if (match[3]) {
                ret.domainId = match[3].toLowerCase();
            } else {
                ret.domainId = "";
            }
            ret.objId = match[4] ? parseInt(match[4]) : 0;
            ret.ionId = match[5] ? parseInt(match[5]) : 0;
        }
        return ret;
    } else if (id && typeof (id) == "object") {
        if (id.domainId)
            ret.domainId = id.domainId;
        if (id.objId)
            ret.objId = id.objId;
        if (id.ionId)
            ret.ionId = ret.ionId;
    }
    if (ret.objId < 0) {
        ret.objId *= -1;
        ret.modify = true;
    }
    return ret;
}

function printHiveId(id) {
    if (!id)
        return "";

    var ret = "";
    if (typeof (id) == "string") {
        id = parseHiveId(id);
    }
    if (typeof (id) == "object") {
        if (id.domainId) {
            ret += id.domainId + ".";
        }
        ret += id.objId;
        if (id.ionId) {
            ret += "." + id.ionId;
        }
    }
    return ret;
}

function parseHexByte(c) {
    c = "" + c;
    var result = 0;
    for (var i = 0; i < 2; i++) {
        if (c[i] >= '0' && c[i] <= '9') {
            result += c[i] - '0';
        } else if (c[i] >= 'a' && c[i] <= 'f') {
            result += 10 + c.charCodeAt(i) - 'a'.charCodeAt(0);
        } else if (c[i] >= 'A' && c[i] <= 'F') {
            result += 10 + c.charCodeAt(i) - 'A'.charCodeAt(0);
        } else {
            return NaN;
        }
        if (!i) {
            result *= 16;
        }
    }
    return result;
}

function parseCString(s) {
    s = "" + s;
    return s.replace(/\\([\\abfnrtv'"?]|[0-8]{3}|x[0-9a-fA-F]{2})/g, function (match, submatch, offset, string) {
        if (submatch == '\\') {
            return '\\';
        } else if (submatch == 'a') {
            return "\x07";
        } else if (submatch == 'b') {
            return "\b";
        } else if (submatch == 'f') {
            return "\f";
        } else if (submatch == 'n') {
            return "\n";
        } else if (submatch == 'r') {
            return "\r";
        } else if (submatch == 't') {
            return "\t";
        } else if (submatch == 'v') {
            return "\v";
        } else if (submatch.length == 1) {
            return submatch;
        } else if (submatch.length == 3 && submatch[0] != 'x') {
            return String.fromCharCode(64 * (submatch[0] - "0") + 8 * (submatch[1] - "0") + (submatch[2] - "0"));
        } else {
            return String.fromCharCode(parseHexByte(submatch.substring(1)));
        }
    });
}

function sanitizeElementId(s) {
    s = "" + s;
    if (!s.length) {
        s = "id" + Math.random();
    }
    s = s.replace(/^[^A-Za-z]/, "_");
    s = s.replace(/[^A-Za-z0-9_.:-]/g, "_");
    return s;
}

function sanitizeElementAttr(s) {
    s = "" + s;
    s = s.replace(/&/g, "&amp;");
    s = s.replace(/'/g, "&#x27;");
    s = s.replace(/"/g, "&#x22;");
    return s;
}

function sanitizeStringJS(s) {
    s = "" + s;
    s = s.replace(/\\/g, "\\\\");
    s = s.replace(/'/g, "\\'");
    s = s.replace(/"/g, "\\\"");
    return s;
}

function sanitizeStringRE(s) {
    s = "" + s;
    s = s.replace(/([.\\\[\]()^|$*?+{}])/g, "\\$1");
    return s;
}

function sanitizeElementAttrJS(s) {
    s = "" + s;
    s = s.replace(/\\/g, "\\\\");
    s = s.replace(/&/g, "\\&amp;");
    s = s.replace(/'/g, "\\&#x27;");
    s = s.replace(/"/g, "\\&#x22;");
    return s;
}

function sanitizeInnerHTML(s) {
    s = s.replaceAll("&", "&amp;");
    s = s.replaceAll("<", "&lt;");
    s = s.replaceAll(">", "&gt;");
    s = s.replaceAll("\"", "&quot;");
    s = s.replaceAll("'", "&#39;");
    s = s.replaceAll("/", "&#x2F;");
    s = s.replaceAll("`", "&#x60;");
    s = s.replaceAll("=", "&#x3D;");
    return s;
}

function htmlDecode(inp) {
    var replacements = {
        '&lt;': '<',
        '&gt;': '>',
    };
    for (var r in replacements) {
        inp = inp.replace(new RegExp(r, 'g'), replacements[r]);
    }

    return inp.replace(/&#(\d+);/g, function (match, dec) {
        return String.fromCharCode(dec);
    });
}

var entityMap = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;',
    '/': '&#x2F;',
    '`': '&#x60;',
    '=': '&#x3D;'
};

function escapeHtml(string) {
    return String(string).replace(/[&<>"'`=\/]/g, function (s) {
        return entityMap[s];
    });
}

function unsanitizeElementAttr(s) {
    s = "" + s;
    s = s.replace(/&#x22;/g, '"');
    s = s.replace(/&quot;/g, '"');
    s = s.replace(/&#x27;/g, "'");
    s = s.replace(/&amp;/g, "&");
    return s;
}

function cmpNatural(a, b) {
    var aa = parseFloat(a);
    var bb = parseFloat(b);
    if (!isNaN(a) && !isNaN(aa) && !isNaN(b) && !isNaN(bb)) {
        if (aa < bb) return -1;
        else if (bb < aa) return 1;
        return 0;
    }
    var as = ("" + a).split(/(\d+|\s+|\.|-|_)/).filter(function (s) { return s.length; });
    var bs = ("" + b).split(/(\d+|\s+|\.|-|_)/).filter(function (s) { return s.length; });
    var i = 0;
    for (; i < as.length && i < bs.length; i++) {
        var av = as[i];
        var bv = bs[i];
if (av < bv) {
            return -1;
        } else if (av > bv) {
            return 1;
        }
    }

    return 0;
}

function cmpCaseNatural(a, b) {
    return cmpNatural(("" + a).toLowerCase(), ("" + b).toLowerCase());
}

function cpyObj()
{
    var dst = new Object(), deep = 0;
    for (var ff = 0; ff < arguments.length; ++ff) {
        var ii = 0;
        for (ii in arguments[ff]) {
            if (ii == 'cpyObjParams') {
                deep = arguments[ff][ii].deep;
            }
        }
    }

    for (var ff = 0; ff < arguments.length; ++ff) {
        var ii = 0;
        for (ii in arguments[ff]) {
            var o = arguments[ff][ii];

            if ((("" + o).indexOf("function") == -1) && (o instanceof Object) && !(o instanceof Array))
                dst[ii] = deep ? cpyObj(o) : new Object(o);
            else if ((o instanceof Array)) {
                dst[ii] = new Array();
                for (var ia = 0; ia < o.length; ++ia) {
                    if ((("" + o[ia]).indexOf("function") == -1) && (o[ia] instanceof Object) && !(o[ia] instanceof Array))
                        dst[ii][ia] = deep ? cpyObj(o[ia]) : new Object(o[ia]);
                    else if ((o[ia] instanceof Array)) {
                        dst[ii][ia] = new Array();
                        for (var ib = 0; ib < o[ia].length; ++ib) {
                            dst[ii][ia][ib] = deep ? cpyObj(o[ia][ib]) : o[ia][ib];
                        }
                    }
                    else
                        dst[ii][ia] = o[ia];
                }
            }
            else
                dst[ii] = o;
        }
    }
    return dst;
}

function mergeObj() {
    var o1 = undefined, o2;
    for (var i = arguments.length - 1; i > 0; --i) {
        o2 = o1 ? o1 : arguments[i];
        o1 = mergeObjPair(arguments[i - 1], o2);
    }
    return o1;
}

function mergeObjPair(obj_dst, obj_src, stacked) {
    stacked = stacked || [];
    if (typeof (obj_dst) === "undefined") {
        if (typeof (obj_src) !== "undefined")
            return obj_src;
        else
            return;
    }

    for (var prop in obj_src) {
        if (!obj_dst[prop]) {
            obj_dst[prop] = obj_src[prop];
        }
        else if (typeof (obj_src[prop]) === "object" && obj_src[prop] !== obj_dst[prop]) {
            var isInfiniteLoop = false;
            for (var ii in stacked) {
                if (stacked[ii] == obj_dst[prop]) {
                    isInfiniteLoop = true;
                    break;
                }
            }
            if (isInfiniteLoop)
                continue;
            stacked.push(obj_dst[prop]);
            mergeObjPair(obj_dst[prop], obj_src[prop], stacked);
            stacked.pop();
        }
    }
    return obj_dst;
}

function parseTableFromCell(text, column, hdr, criteria) {

    var tblArr = new vjTable(text, 0, vjTable_propCSV);
    var XYvalues = hdr + "\n";

    for (var il = 0; il < tblArr.rows.length; ++il) {
        var cols = tblArr.rows[il].cols[column].split(",");
        if (!eval(criteria)) continue;
        XYvalues += tblArr.rows[il].cols[1] + ",";
        for (var ic = 0; ic < cols.length; ic += 2) {
            XYvalues += cols[ic] + "," + cols[ic + 1];
            XYvalues += "\n";
        }
    }
    return XYvalues;
}

function alerJ(descr, o, dofunc) {
    var ii = 0, t = "";
    if (descr) t += descr + "\n-------------\n";
    for (ii in o) {
        if (!dofunc && ("" + o[ii]).indexOf("function") != -1) continue;
        t += (ii + "=" + o[ii] + "\n");
    }
    alert(t);
}

function funcUrl(func) {
    if (!func || typeof func != "string") { return false };

    if (func.indexOf("function:") == 0) {
        return false;
    } else if (func.indexOf("javascript:") == 0) {
        return false;
    } else {
        if (gCGI && func.substring(0, 1) == '?') {
            func = gCGI + func;
        }
        return func;
    }
}

function funcLink(func) {
    if (!func) return;

    var clsfunc = 0;

    var args = new Array();
    for (var ff = 1; ff < arguments.length; ++ff) {
        args.push(arguments[ff]);
    }
    if (typeof (func) == "function")
        ;
    else if (func.indexOf("function:") == 0) {
        clsfunc = eval(func.substring(9));
    }
    else if (func.indexOf("javascript:") == 0) {
        if (args && args[0]) {
            var thisObj = args[0];
            var node = args[1];
        }
        return eval(func.substring(11));
    }
    else {
        if (gCGI && func.substring(0, 1) == '?') {
            func = gCGI + func;
        }
        document.location = func;
        return;
    }

    if (clsfunc)
        return (clsfunc.func).apply(clsfunc.obj, args);
    else {
        return func.apply(args[0], args);
    }
}


function matchObj(obj, tomatch) {
    if (!tomatch) return false;
    var lst = verarr(tomatch);
    var ifound, ff = 0;
    for (ifound = 0; ifound < lst.length; ++ifound) {
        var matches = 0, allmatch = 0;
        for (ff in lst[ifound]) ++allmatch;
        for (ff in lst[ifound]) {
            var patt = lst[ifound][ff];
            if (patt == null && !obj[ff])
                ++matches;
            else if (("" + obj[ff]).match(patt))
                ++matches;
        }
        if (matches == allmatch) return lst[ifound];
    }
    return false;
}

function evalSingleVar(expr, orig_expr, objcls, computeFunctionCallback) {
    var isglobal = false;
    var isstring = false;
    var isPrompt = false;
    var isCookie = false;
    if (expr.substring(0, 2) == "::") {
        isglobal = true;
        expr = expr.substring(2);
    }
    if (expr.substring(0, 2) == "s:") {
        isstring = true;
        expr = expr.substring(2);
    }
    if (expr.substring(0, 2) == "?:") {
        isPrompt = true;
        expr = expr.substring(2);
    }
    if (expr.substring(0, 2) == "c:") {
        isCookie = true;
        expr = expr.substring(2);
    }

    var ts = "";
    if (computeFunctionCallback) {
        if (objcls) {
            ts = objcls[computeFunctionCallback](expr, orig_expr);
        } else {
            ts = computeFunctionCallback(expr, orig_expr);
        }
        if (!ts) {
            ts = "";
        }
    } else if (isCookie) {
        ts = cookieGet(expr);
    } else if (!isglobal && objcls) {
        ts = objcls[expr];
        if (!ts && isPrompt) {
            ts = prompt(expr);
            if (!ts) {
                return { fail: true };
            }
        }
    } else {
        ts = eval(expr);
        if (!ts && isPrompt) {
            ts = prompt(expr);
            if (!ts) {
                return { fail: true };
            }
        }
    }

    if (isstring) {
        ts = "\"" + ts + "\"";
    }
    return ts;
}

function evalExpr(text, starters, ender, objcls, computeFunctionCallback) {
    if (!text || typeof (text) == "function") {
        return text;
    }
    starters = verarr(starters).concat();
    starters.forEach(function (s, i) {
        if (s.indexOf('(') + 1 != s.length) {
            alert("DEVELOPER WARNING: evalExpr: only starters with '(' are supported");
            return text;
        }
        starters[i] = s.substring(0, s.length - 1);
    });
    if (ender && ender != ')') {
        alert("DEVELOPER WARNING: evalExpr: only ender === ')' is supported");
        return text;
    }

    var ast;
    try {
        ast = jsep(text);
    } catch (e) {
        return text;
    }

    return function eval_node(node) {
        if (node.type === "Literal") {
            return node.value;
        } else if (node.type === "UnaryExpression") {
            var val = eval_node(node.argument);
            if (node.operator === "-") {
                return -val;
            } else if (node.operator === '!') {
                return !val;
            } else if (node.operator === '~') {
                return ~val;
            } else if (node.operator === '+') {
                return +val;
            }
        } else if (node.type === "BinaryExpression") {
            var val1 = eval_node(node.left);
            var val2 = eval_node(node.right);
            if (node.operator === "|") {
                return val1 | val2;
            } else if (node.operator === "^") {
                return val1 ^ val2;
            } else if (node.operator === "&") {
                return val1 & val2;
            } else if (node.operator === "==") {
                return val1 == val2;
            } else if (node.operator === "!=") {
                return val1 != val2;
            } else if (node.operator === "===") {
                return val1 === val2;
            } else if (node.operator === "!==") {
                return val1 !== val2;
            } else if (node.operator === "<") {
                return val1 < val2;
            } else if (node.operator === ">") {
                return val1 > val2;
            } else if (node.operator === "<=") {
                return val1 <= val2;
            } else if (node.operator === ">=") {
                return val1 >= val2;
            } else if (node.operator === "<<") {
                return val1 << val2;
            } else if (node.operator === ">>") {
                return val1 >> val2;
            } else if (node.operator === ">>>") {
                return val1 >>> val2;
            } else if (node.operator === "+") {
                return val1 + val2;
            } else if (node.operator === "-") {
                return val1 - val2;
            } else if (node.operator === "*") {
                return val1 * val2;
            } else if (node.operator === "/") {
                return val1 / val2;
            } else if (node.operator === "%") {
                return val1 % val2;
            }
        } else if (node.type === "LogicalExpression") {
            if (node.operator === "||") {
                return eval_node(node.left) || eval_node(node.right);
            } else if (node.operator === "&&") {
                return eval_node(node.left) && eval_node(node.right);
            }
        } else if (node.type === "ConditionalExpression") {
            return eval_node(node.test) ? eval_node(node.consequent) : eval_node(node.alternate);
        } else if (node.type === "CallExpression" && node.callee.type === "Identifier" && starters.indexOf(node.callee.name) >= 0 && node.arguments.length === 1 && node.arguments[0].type === "Identifier") {
            var val = evalSingleVar(node.arguments[0].name, node.callee.name + "(" + node.arguments[0].name + ")", objcls, computeFunctionCallback);
            return val.fail ? 0 : val;
        }
        return 0;
    }(ast);
}

function evalVars(text, starters, ender, objcls, computeFunctionCallback) {
    if (!text || typeof (text) == "function")
        return text;
    starters = verarr(starters);
    var starter = 0;
    for (var j = 0; j < starters.length; ++j) {
        starter = starters[j];
        var spl = text.split(starter);
        var split_start = 1;
        text = spl[0];
        if (text.indexOf(starter) == 0) {
            split_start = 0;
            text = "";
        }
        if (spl.length > 1) {
            for (var i = split_start; i < spl.length; ++i) {
                var posend = spl[i].indexOf(ender);

                if (posend == -1) { text += spl[i]; continue; }
                var expr = spl[i].substring(0, posend);

                var ts = evalSingleVar(expr, starter + expr + ender, objcls, computeFunctionCallback);
                if (ts && ts.fail) {
                    return 0;
                }

                text += ts + spl[i].substring(posend + ender.length);
            }
        }
        if (text && text.length) break;
    }
    return text;
}

function isDateInYear(check, d)
{
    if (!check)
        return;
    var t_check = new Date(check);
    var t_d = d ? new Date(d) : new Date();
    t_d.setHours(0, 0, 0, 0); t_check.setHours(0, 0, 0, 0);
    return t_d.setMonth(0, 0) == t_check.setMonth(0, 0);

}

function isDateInMonth(check, d)
{
    if (!check)
        return;
    var t_check = new Date(check);
    var t_d = d ? new Date(d) : new Date();
    t_d.setHours(0, 0, 0, 0); t_check.setHours(0, 0, 0, 0);
    return t_d.setDate(0) == t_check.setDate(0);
}

function isDateInDate(check, d)
{
    if (!check)
        return;
    var t_check = new Date(check);
    var t_d = d ? new Date(d) : new Date();
    return t_d.setHours(0, 0, 0, 0) == t_check.setHours(0, 0, 0, 0);
}
function formatToDate(s) {
    let d;
    if (typeof (s) === "number" || typeof (s) === "string" && s.match(/^\s*\d*(\.\d*)?\s*$/)) {
        var sec = parseInt(s);
        d = new Date(sec * 1000);
    } else {
        if (typeof (s) == "string") {
            s = s.replace(/^(\d{4}-\d{2}-\d{2}) (\d{2}:\d{2})/, "$1T$2");
        }
        d = new Date(s);
    }
    return d;
}
function formatTimespan(tt) {
    var realText = "";
    if (tt > 24 * 3600) {
        realText += parseInt(tt / (24 * 3600)) + "d ";
        tt = parseInt(tt % (24 * 3600));
    }
    if (tt > 3600) { realText += (parseInt(tt / (3600)) + "") + "h "; tt = parseInt(tt % (3600)); }
    if (tt > 60) { realText += (parseInt(tt / (60)) + "") + "m "; tt = parseInt(tt % (60)); }
    if (!realText || tt > 0) { realText += (parseInt(tt) + "s") + ""; }
    return realText;
};

function formaDatetime(s, absoluteOnly, forceDate, forceTime, completeDate) {
    var value = '';
    var d = formatToDate(s);

    if (d && d.getTime() && !isNaN(d.getTime())) {
        var now = new Date();
        var timeDiff = now.getTime() - d.getTime();

        if (parseInt(timeDiff) < 0) {
            return d.toLocaleDateString();
        }

        if (parseInt(timeDiff) < 60000 && !absoluteOnly)
            return "1 min ago";

        var min = parseInt((timeDiff) / 60000);
        now.setHours(0);
        now.setMinutes(0);
        now.setSeconds(0);
        now.setMilliseconds(0);
        if (min < 3 * 60 && !absoluteOnly) {
            var x = parseInt(min / 60);
            if (x) {
                value = x + ' hrs ';
            }
            x = parseInt(min % 60);
            if (x) {
                value += x + ' min ';
            }
            value += 'ago';
            if (value == 'ago') {
                value = 'recently';
            }
        } else if (!isDateInDate(d) && completeDate) {
            value = d.toLocaleDateString() + " " + d.toLocaleTimeString();
        } else if (isDateInDate(d) && !forceDate) {
            value = d.toLocaleTimeString();
        } else if (!isDateInDate(d) && !forceTime) {
            value = d.toLocaleDateString();
        } else {
            value = d.toLocaleString();
        }
    }
    return value;
}

function dateNow() {
    var d = new Date();
    return d.toISOString().split("T")[0];
}

function timeNow() {
    var d = new Date();
    return d.toLocaleTimeString();
}
function dateObjetToJSON(date, i) {
    let json = date.toJSON()
    i = i ? i : json.length
    return json.substring(0, i)
}

function formatBytes(bytes, decimals = 2) {
    if (bytes == 0) return '0 Bytes';

    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
}

function arraySortUnique(arr) {
    arr = arr.sort(function (a, b) { return a * 1 - b * 1; });
    var ret = [arr[0]];
    for (var i = 1; i < arr.length; i++) {
        if (arr[i - 1] !== arr[i]) {
            ret.push(arr[i]);
        }
    }
    return ret;
}
function arrayCross(arr1, arr2, what) {
    arr1 = verarr(arr1);
    arr2 = verarr(arr2);

    for (var i = 0; i < arr1.length; ++i) {
        for (var j = 0; j < arr2.length; ++j) {
            if (arr1[i] == arr2[j])
                return true;
        }
    }
    return false;
}
function isArrayChild(arr1, arr2, sizeDifferenceRequirement)
{
    arr1 = verarr(arr1);
    arr2 = verarr(arr2);
    if (sizeDifferenceRequirement !== undefined && sizeDifferenceRequirement > 0) {
        if (Math.abs(arr1.length - arr2.length) != sizeDifferenceRequirement) return false;
    }

    var arrParent = arr1;
    var arrChild = arr2;
    if (arr2.length > arr1.length) {
        arrParent = arr2;
        arrChild = arr1;
    }
    var needToMatch = arrChild.length;
    var alreadyMatch = 0;
    for (var i = 0; i < arrChild.length; ++i) {
        for (var j = 0; j < arrParent.length; ++j) {
            if (arrChild[i] == arrParent[j]) {
                ++alreadyMatch;
                break;
            }
        }
    }
    return (alreadyMatch == needToMatch) ? true : false;
}

function filterArrayOfObjects(array, key, value) {
    let filtered_array = []
    array.forEach(function (item) {
        if (item[key] === value) filtered_array.push(item)
    })

    return filtered_array
}

function strpbrk(haystack, char_list) {
    for (var i = 0, len = haystack.length; i < len; ++i) {
        if (char_list.indexOf(haystack.charAt(i)) >= 0) {
            return haystack.slice(i);
        }
    }
    return false;
}

function compareToStringArray(myString, arrayOfStringToCompare, ORorAND, exact) {
    if (exact === undefined) exact = true;
    arrayOfStringToCompare = verarr(arrayOfStringToCompare);
    if (arrayOfStringToCompare.length == 0) return false;
    var condition = "or";
    if (ORorAND !== undefined && ORorAND.toLowerCase() == "and") {
        condition = "and";
    }
    var result = false;
    for (var iword = 0; iword < arrayOfStringToCompare.length; ++iword) {

        if (exact) result = (myString == arrayOfStringToCompare[iword]) ? true : false;
        else if (exact == false) result = (arrayOfStringToCompare[iword].indexOf(myString) != -1) ? true : false;

        if (result == true && condition == "or") {
            return result
        }
        if (result == false && condition == "and") {
            return result;
        }
    }
    return result;
}
function textContent(elt_or_html) {
    if (elt_or_html == null || elt_or_html == undefined) return "";
    try {
        var elt;
        if (typeof elt_or_html === "string") {
            if (elt_or_html.indexOf("<") < 0 && elt_or_html.indexOf("&") < 0)
                return elt_or_html;

            elt = document.createElement("div");
            elt.innerHTML = elt_or_html;
        } else {
            elt = elt_or_html;
        }
        return elt.textContent || elt.innerText || "";
    } catch (error) {
        console.error(error)
        return ""
    }

}

function reportSubjectQuery(ds, txt, arr) {
    var tblArr = new vjTable(txt, 0, vjTable_propCSV);
    var newText = txt;

    var newArr = [];
    for (var iA = 0; iA < arr.length; ++iA) {
        var val = docLocValue(arr[iA]);
        if (val.length) {
            var key_value = { "key": arr[iA], "value": val };
            newArr.push(key_value);
        }
    }
    var has_changed = 0;
    if (newArr.length) {
        for (var i = 0; i < tblArr.rows.length; ++i) {
            for (var ih = 0; ih < tblArr.hdr.length; ++ih) {
                var hdr = tblArr.hdr[ih].name;
                if (hdr == "name") {
                    has_changed = 1;
                    for (var ik = 0; ik < newArr.length; ++ik) {
                        tblArr.rows[i][hdr] += "&" + newArr[ik].key + "=" + newArr[ik].value;
                    }
                }
            }
        }
    }
    if (has_changed) {
        newText = "";
        for (var i = 0; i < tblArr.hdr.length; ++i) {
            if (i) newText += ",";
            newText += tblArr.hdr[i].name;
        }
        newText += "\n";
        for (var k = 0; k < tblArr.rows.length; ++k) {
            for (var ih = 0; ih < tblArr.hdr.length; ++ih) {
                var hdr = tblArr.hdr[ih].name;
                if (ih) newText += ",";
                var cell = tblArr.rows[k][hdr];
                if (hdr == "name") {
                    cell = "\"" + cell + "\"";
                    cell = quoteForCSV(cell);
                }
                newText += cell;
            }
            newText += "\n";
        }
    }
    return newText;
}

var gHome = "/";
var gCGI = "";




var gMoX = 0;
var gMoY = 0;
var gPgW = 2;
var gPgH = 2;
var gPgScrlX = 0;
var gPgScrlY = 0;
var gMoHookPrc = "";
var gMoShowXY = false;
var gMoWheelCallback;
var gKeyShift = 0;
var gKeyCtrl = 0;
var gMouseButton = 0;
var gTextSizer = 8;
var gOnMouseMoveCallback = null;
var gDragEvent = false;
var gResizeableElement = null;

function nullFunc() {

}

function gGetPgParams() {
    if (typeof window.innerWidth != 'undefined') {
        gPgW = window.innerWidth;
        gPgH = window.innerHeight;
    }
    else if (typeof document.documentElement != 'undefined' && typeof document.documentElement.clientWidth != 'undefined' && document.documentElement.clientWidth != 0) {
        gPgW = document.documentElement.clientWidth;
        gPgH = document.documentElement.clientHeight;
    }
    else
    {
        gPgW = document.getElementsByTagName('body')[0].clientWidth;
        gPgH = document.getElementsByTagName('body')[0].clientHeight;
    }
    gPgCX = Math.round(gPgW / 2);
    gPgCY = Math.round(gPgH / 2);

    gMoX = gPgCX;
    gMoY = gPgCY;

    var o = document.getElementById('basic-textsizer');
    if (o) {
        gTextSizer = parseInt(o.offsetWidth / textContent(o).length) + 1;
        o.className = "sectHid";
    }
}
gGetPgParams();

function gOnMouseUp(e) {
    gResizeableElement = null;
}
function gOnMouseDown(e) {
}
function gOnMouseMove(e) {
    if (document.all)
        e = event;
    gKeyShift = (e.shiftKey) ? 1 : 0;
    gKeyCtrl = (e.ctrlKey || e.metaKey) ? 1 : 0;
    gMouseButton = (e.which);

    if (gMoHookPrc != "") gMoHookPrc(gMoX, gMoY);
    if (typeof (e.pageX) == 'number') {
        gMoX = e.pageX;
        gMoY = e.pageY;
        if (document.body) {
            gPgScrlX = document.documentElement.scrollLeft;
            gPgScrlY = document.documentElement.scrollTop;
        }
    } else {
        gMoX = e.clientX; gMoY = e.clientY;

        if (!((window.navigator.userAgent.indexOf('Opera') + 1) || (window.ScriptEngine && ScriptEngine().indexOf('InScript') + 1) || window.navigator.vendor == 'KDE')) {
            if (document.documentElement && (document.documentElement.scrollTop || document.documentElement.scrollLeft)) {
                gPgScrlX = document.documentElement.scrollLeft; gPgScrlY = document.documentElement.scrollTop;
                gMoX += document.documentElement.scrollLeft; gMoY += document.documentElement.scrollTop;

            } else if (document.body && (document.body.scrollTop || document.body.scrollLeft)) {
                gPgScrlX = document.body.scrollLeft; gPgScrlY = document.body.scrollTop;
                gMoX += document.body.scrollLeft; gMoY += document.body.scrollTop;
            }
        }
    }


    if (gResizeableElement && gMouseButton == 1 && (gMoX != gResizeableElement.originalX || gMoY != gResizeableElement.originalY)) {


        var diffW = gMoX - gResizeableElement.originalX;
        var diffH = gMoY - gResizeableElement.originalY;
        var el = 0;
        for (var ie = 0; gResizeableElement.objClses.length == 0 && ie < gResizeableElement.elements.length; ++ie) {
            var elid = gResizeableElement.elements[ie];
            el = gObject(elid);

            if (gResizeableElement.type.indexOf('r') != -1 || gResizeableElement.type.indexOf('l') != -1) {
                var oldW = parseInt(el.style.width); if (!oldW || isNaN(oldW))
                    oldW = el.clientWidth;
                el.style.width = (oldW + diffW) + "px";
            }

            if (gResizeableElement.type.indexOf('t') != -1 || gResizeableElement.type.indexOf('b') != -1) {
                var oldH = parseInt(el.style.height); if (!oldH || isNaN(oldH))
                    oldH = el.clientHeight;
                el.style.height = (oldH + diffH) + "px";
            }
            if (gResizeableElement.callback)
                funcLink(gResizeableElement.callback, el, gResizeableElement);
        }

        for (var iv = 0; iv < gResizeableElement.objClses.length; iv++) {
            var v = vjObj[gResizeableElement.objClses[iv]];
            if (!v || !v.resize)
                continue;

            var vDiffW = undefined, vDiffH = undefined;
            if (gResizeableElement.type.indexOf('r') != -1 || gResizeableElement.type.indexOf('l') != -1)
                vDiffW = diffW;
            if (gResizeableElement.type.indexOf('t') != -1 || gResizeableElement.type.indexOf('b') != -1)
                vDiffH = diffH;

            v.resize(vDiffW, vDiffH);
            if (gResizeableElement.callback)
                funcLink(gResizeableElement.callback, v, gResizeableElement);
        }

        gResizeableElement.originalX = gMoX;
        gResizeableElement.originalY = gMoY;
        elPos = gObjPos(el);
        var o = gObject('basic-resizer');
        if (gResizeableElement.type.indexOf('r') != -1 || gResizeableElement.type.indexOf('l') != -1)
            o.style.left = (elPos.x + el.clientWidth - 8) + "px";
        if (gResizeableElement.type.indexOf('t') != -1 || gResizeableElement.type.indexOf('b') != -1)
            o.style.top = (elPos.y + el.clientHeight - 8) + "px";
        stopDefault(event);

    }

    if (gOnMouseMoveCallback) gOnMouseMoveCallback(e, gMoX, gMoY);

}

function gResizerText(elements, objClses, type, callback) {
    var sym = "";
    var cursor = "";
    if (type == 'r' || type == 'l') { sym += "&harr;"; cursor = "w-resize"; }
    else if (type == 't' || type == 'b') { sym += "&#8597;"; cursor = "n-resize"; }
    else { sym += "<img src='img/resize-all.png' draggable=false width=16 border=0>"; cursor = "nwse-resize"; }


    var t = "<span onMouseDown='gOnResizeStart(\"" + elements + "\",\"" + objClses + "\",\"" + type + "\",\"" + sanitizeElementAttrJS(callback ? callback : "") + "\")' ";
    t += " style='cursor: " + cursor + ";-webkit-touch-callout: none;-webkit-user-select: none;-khtml-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;'>";
    t += sym;
    t += "</span>";

    return t;
}

function gShowResizer(container, objCls, type, callback) {
    var t = gResizerText(container, objCls, type, callback);
    var o = gObject(container);
    var pos = gObjPos(o, false);
    var x = pos.x + o.clientWidth / 2;
    var y = pos.y + o.clientHeight / 2;


    if (type.indexOf('r') != -1) { x = pos.x + o.clientWidth; }
    if (type.indexOf('l') != -1) { x = pos.x; }
    if (type.indexOf('b') != -1) { y = pos.y + o.clientHeight; }
    if (type.indexOf('t') != -1) { y = pos.y; }

    gObjectSet('basic-resizer', x - 8, y - 8, t, 'show', '-', '-');
}

function gOnResizeStart(elements, objClses, type, callback) {
    elements = elements && elements.length ? verarr(elements.split(",")) : [];
    objClses = objClses && objClses.length ? verarr(objClses.split(",")) : [];
    gResizeableElement = {
        'elements': elements,
        'objClses': objClses,
        'callback': callback,
        originalX: gMoX,
        originalY: gMoY,
        'type': type
    };
    return gResizeableElement;
}


var gOnDocumentClickCallbacksStack = new Array();
var gOnDocumentClickCallbacks = "";
function gOnDocumentClick(e) {
    var res = 0;
    if (document.all)
        e = event;

    gKeyShift = (e.shiftKey) ? 1 : 0;
    gKeyCtrl = (e.ctrlKey || e.metaKey) ? 1 : 0;

    if (gOnDocumentClickCallbacksStack.length) {
        for (var iv = gOnDocumentClickCallbacksStack.length - 1; iv >= 0; --iv) {
            var isinsidelayer = gValidAncestorOnProp(e.target, "id", gOnDocumentClickCallbacksStack[iv].layer);
            if (isinsidelayer) continue;

            if (gOnDocumentClickCallbacksStack[iv].clickCounter === "-") continue;
            --gOnDocumentClickCallbacksStack[iv].clickCounter;
            if (gOnDocumentClickCallbacksStack[iv].clickCounter >= 0) continue;
            if (gOnDocumentClickCallbacksStack[iv].clickCallback)
                res = eval(gOnDocumentClickCallbacksStack[iv].clickCallback);
            if (res)
                break;
        }
    }

    if (res == 0 && isok(gOnDocumentClickCallbacks))
        eval(gOnDocumentClickCallbacks);

}

function gModalOpen(layername, clickFun, x, y, clickCount) {
    if (typeof (clickCount) !== "number" && !clickCount) clickCount = 1;

    gObjectSet(layername, x, y, '-', 'show', '-', '-');
    ++gObjZindex;
    gObject(layername).style.zIndex = gObjZindex;

    gOnDocumentClickCallbacksStack.push({ layer: layername, clickCallback: clickFun, clickCounter: clickCount });
}

function gModalClose(layername) {
    gObjectSet(layername, '-', '-', '-', 'hide', '-', '-');
    gOnDocumentClickCallbacksStack.pop();
}



function gOnMouseWheel(event) {
    if (!gMoWheelCallback) {
        event.returnValue = true;
        return;
    }
    gKeyShift = (event.shiftKey) ? 1 : 0;
    gKeyCtrl = (event.ctrlKey || event.metaKey) ? 1 : 0;

    var delta = 0;
    if (!event)
        event = window.event;
    if (event.wheelDelta) {
        delta = event.wheelDelta / 120;
        if (window.opera)
            delta = -delta;
    } else if (event.detail) {
        delta = -event.detail / 3;
    }
    var ret = 0;
    if (delta)
        ret = gMoWheelCallback(delta);
    stopDefault(event);
}

var gForm_onEnterCallback;
function gOnFormEnter(e) {
    var ev = e || event;
    if (((ev.keyCode || ev.which || ev.charCode || 0) != 13) ||
        !gForm_onEnterCallback || gForm_onEnterCallback == '') {
        return true;
    }
    return gForm_onEnterCallback(ev.srcElement || ev.target, ev);
}

function gObject(objnme) {
    var objnm = "";
    pos = objnme.indexOf("->");
    if (pos != -1) {
        doc = objnme.substr(0, pos);
        d = eval(doc);
        objnm = objnme.substr(pos + 2);
    }
    else { d = document; doc = "document"; objnm = objnme; }

    return d.getElementById(objnm);

}


var gObjZindex = 100;
function gObjectSet(lrnm, lrx, lry, lrcont, lrvis, lrcx, lrcy, setZMax) {
    lr = gObject(lrnm);
    if (!lr) return;
    if (lrx != "-" || lry != "-") {
        if (lrx != "-") lr.style.left = lrx + "px";
        if (lry != "-") lr.style.top = lry + "px";
    }

    if (lrcx != "-" && lrcy != "-") {
        lr.style.width = lrcx;
        lr.style.height = lrcy;
    }
    if (lrvis != "-") {
        if (lrvis == "show") { lr.style.visibility = "visible"; lr.style.display = ""; }
        if (lrvis == "hide") { lr.style.visibility = "hidden"; lr.style.display = "none"; }
        if (lrvis == "toggle") {
            if (lr.style.visibility != "hidden") { lr.style.visibility = "hidden"; lr.style.display = "none"; }
            else { lr.style.visibility = "visible"; lr.style.display = ""; }
        }
        if (setZMax && lr.style.visibility == 'visible') {
            lr.style.zIndex = gObjZindex + setZMax;
            ++gObjZindex;
        }
    }
    if (lrcont && lrcont != "-") {
        if (lrcont.indexOf("<s>", 0) == 0) {
            lrfunc = lrcont.substr(3);
            lrcont = eval(lrfunc);
        }
        lr.innerHTML = lrcont;

    }
}

function gObjPos(el, relative) {
    if (!el) return { x: gMoX, y: gMoY, cx: 0, cy: 0 };
    var rval = { cx: el.offsetWidth, cy: el.offsetHeight };
    for (var lx = 0, ly = 0; el != null; lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent)
        ;
    rval.x = lx;
    rval.y = ly;
    return rval;
}

function gObjectPositionShow(lrname, posx, posy, zind) {
    gObjectSet(lrname, posx, posy, "-", "show", "-", "-");
    var o = gObject(lrname); if (!o) return;
    var pos = gObjPos(o);
    if (zind) o.style.zIndex = (zind);
    if (pos.x + pos.cx > gPgW) pos.x = gPgW - pos.cx;
    if (pos.x < 0) pos.x = 0;
    gObjectSet(lrname, pos.x, "-", "-", "-", "-", "-");
}

var gObjVis_arr = "";
function visRemember(id, toggle) {
    var newarr = "";
    if (gObjVis_arr && gObjVis_arr.length) {
        var spl = gObjVis_arr.split("
        for (var i = 0; i < spl.length; ++i) {
            var lin = spl[i]; if (!lin || !lin.length) continue;
            if (newarr.length) newarr += "
            if (lin.indexOf(id) != 0) newarr += lin;
        }
    }
    if (newarr.length) newarr += "
    newarr += id + "=" + toggle;
    gObjVis_arr = newarr;
}

function visRestore() {
    return;
}


function visibool(idlist, condition) {

    if (condition > 0) vis(idlist, "sectVis");
    else if (condition == 0) vis(idlist, "sectHid");
    else vis(idlist);
}

function vis(idlist, toggle0) {
    var o;
    if (idlist) {
        var jarr = idlist.split(",");
        for (var i = 0; i < jarr.length; ++i) {
            var toggle;
            var id = jarr[i];
            o = document.getElementById(id);

            if (!toggle0) {
                if (!o) toggle = "sectVis";
                else if (o.className == "sectHid") toggle = "sectVis";
                else toggle = "sectHid";
            } else toggle = toggle0;
            if (o) o.className = toggle;
        }

    }
}

function ovis(id, defvis, plusminus) {
    if (!plusminus || plusminus.length == 0)
        plusminus = "<img border=0 width=20 src='img/recExpand.gif' />|<img border=0 width=20 src='img/recCollapse.gif' />";
    var plmn = plusminus.split("|");
    if (!defvis) defvis = "sectVis";

    var t = "<span id='" + id + "_vis_open'  class='" + (defvis == "sectVis" ? "sectHid" : "sectVis") + "'><a href='javascript:vis(\"" + id + "," + id + "_vis_open," + id + "_vis_close\");'>" + plmn[0] + "</a></span>" +
        "<span id='" + id + "_vis_close' class='" + (defvis == "sectVis" ? "sectVis" : "sectHid") + "' ><a href='javascript:vis(\"" + id + "," + id + "_vis_close," + id + "_vis_open\");'>" + plmn[1] + "</a></span>";
    return t;
}

var gFloatPermanent = 0;
var gFloatButtons = 3;
var gFloatTimeout = 10000;
var gFloatBGColor = "bgcolor=#ffffdf";
var gFloatShiftX = -16;
var gFloatShiftY = -16;
var gFloatLastText = "";
var gFloatLastTitle = "";
var gFloatTimer = 0;
var gFloatOpen = 0;
function floatDivOpen(title, text, internal) {
    if (gFloatTimer) clearTimeout(gFloatTimer); gFloatTimer = 0;

    if (gFloatButtons) {
        var btn1 = "", btn2 = "";
        if (gFloatButtons & 0x2) btn2 += "<a href='javascript:gFloatPermanent=1-gFloatPermanent;floatDivOpen(\"-\",\"-\",1)'><img border=0 width=16 src='img/pin" + (gFloatPermanent ? "down" : "up") + ".gif'/></a>&nbsp;";
        if (gFloatButtons & 0x1) btn1 += "<a href='javascript:floatDivClose()'><img border=0 width=16 src='img/xlose.gif'/></a>";
        if (title.length) text = "<table class='VISUAL_popup'  ><tr border=0><td valign=top align=right>" + btn2 + "</td><td><small><b>" + title + "</b></small></td><td valign=top align=right>&nbsp;" + btn1 + "</td></tr><tr><td colspan=2>" + text + "</td></tr><table>";
        else text = "<table class='VISUAL_popup'  border=0 " + gFloatBGColor + "><tr><td>" + text + "<td><td valign=top>" + btn + "</td></tr><table>";
    } else if (title.length) {
        if (title.length) text = "<table class='VISUAL_popup'  ><tr border=0><td><small><b>" + title + "</b></small></td><td colspan=2>" + text + "</td></tr><table>";
    }
    gObjectSet("floatDiv", (gMoX + gFloatShiftX), (gMoY + gFloatShiftX), text, "show", "-", "-");

    if (!gFloatPermanent) gFloatTimer = setTimeout("floatDivClose(1)", gFloatTimeout);
}
function floatDivClose(isauto) {
    if (gFloatPermanent == 1) return;
    gObjectSet("floatDiv", "-", "-", "-", "hide", "-", "-");
    if (gFloatTimer) clearTimeout(gFloatTimer); gFloatTimer = 0;
    gFloatOpen = 0;
}



var gWaitLoad = 0;
function gWait(how, txt) {
    var v = gObject("waiting"); if (!v) return;
    if (txt) v.innerHTML = txt;
    gWaitLoad += how; if (gWaitLoad < 0) gWaitLoad = 0;
    if (gWaitLoad) v.className = "sectVis";
    else v.className = "sectHid";
}



var gObjectClsList = new Array();


function gObjectClass(sect) {
    if (gObjectClsList[sect] == "sectVis") return "sectVis";
    return "sectHid";
}

function gObjectClassToggle(sect, which) {
    if (which && which.length != 0) {
        gObjectClsList[sect] = which;
        return;
    }

    var o = gObject(sect); if (!o) return;
    o.className = (o.className == 'sectVis') ? 'sectHid' : 'sectVis';
    gObjectClsList[sect] = o.className;
}

function cookieSet(sNm, sValue, days) {
    var expires = ";";
    if (days) {
        var date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        expires += " expires=" + date.toGMTString();
    }
    document.cookie = sNm + "=" + escape(sValue) + "; Secure; SameSite=Strict; Path=" + gHome + expires;
}

function cookieDelete(sNm) {
    cookieSet(sNm, "", -1);
}

function cookieGet(sNm, deflt) {
    var aCookie = document.cookie.split(/\s*;\s*/);
    for (var i = 0; i < aCookie.length; i++) {
        var aCrumb = aCookie[i].split("=");
        if (sNm == aCrumb[0]) {
            return unescape(aCrumb[1]);
        }
    }
    return deflt;
}

function protectFields(url) {
    return encodeURIComponent(url);
}

function unprotectFields(url) {
    url = decodeURIComponent(url);
    return url.replace(/[+]/g, " ");
}

function urlExchangeParameter(url, parname, newvalue, doNotForce) {
    url = "" + url;
    if (url.indexOf("static:
        return url;

    var sepRe = "(&|\\?|
    var parRe = sepRe + parname + "=[^&]*";

    if (doNotForce && url.search(new RegExp(parRe)) == -1)
        return url;

    if (newvalue == "-") {
        return url.replace(new RegExp(parRe + "(&?)"), function (match, sep, endsep) {
            return (sep == "&") ? endsep : sep;
        });
    }

    var replacement = parname + "=" + newvalue;
    var parFound = false;



    url = url.replace(new RegExp(parRe), function (match, sep) {
        parFound = true;
        return sep + replacement;
    });

    if (parFound)
        return url;

    if (url == "")
        return "?" + replacement;

    url = url.replace(new RegExp(sepRe + "?$"), function (match, sep) {
        if (!sep)
            sep = "&";
        return sep + replacement;
    });
    return url;
}


function getEscapedObjQry(text) {
    if (typeof text === 'string') {
        text = `${text}`
            .replace(/%/mg, "%25")
            .replace(/\r?\n/mg, "%0A")
            .replace(/ /mg, "%20")
            .replace(/[\r\n]/mg, "")
            .replace(/#/mg, "%23")
            .replace(/\+/mg, "%2B")
            .replace(/=/mg, "%3D")
            .replace(/&/mg, "%26");
        return text
    }
    return null;
}

function constructObjQryUrl(tmplt, values, prefix) {
    if (!tmplt || !tmplt.length)
        return "static:
    if (typeof (tmplt) == "string") {
        try {
            tmplt = JSON.parse(tmplt);
        } catch (e) {
            return "static:
        }
    }
    if (!prefix) {
        prefix = `http:
    }
    var qry = "";
    for (var i = 0; i < tmplt.length; i++) {
        if (tmplt[i].s)
            qry += tmplt[i].s;
        else if (tmplt[i].v)
            qry += values[tmplt[i].v] ? values[tmplt[i].v] : "";
    }
    return prefix + vjDS.escapeQueryLanguage(qry);
}

function getObjQryCmd() {
    return "cmd=objQry&qry="
}


var bustcachevar = 1;
var bustcacheparameter = "";
var gAjaxPreSubmissionCallback;

function ajaxDynaRequestPage(url, parameter, callbackfun, ispost, formData) {
    var page_request = false;
    if (window.XMLHttpRequest) {
        page_request = new XMLHttpRequest();
    } else if (window.ActiveXObject) {
        try {
            page_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
                page_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {
            }
        }
    } else {
        return false;
    }
    page_request.parameter = parameter;
    page_request.onreadystatechange = function () {
        ajaxDynaLoadPage(page_request, parameter, callbackfun);
    };

    var rawURL = parameter && parameter.isCORS;
    if (rawURL) {
        if (parameter.withCredentials) {
            page_request.withCredentials = true
        }
    } else if (gCGI && url.substring(0, 1) == '?') {
        url = gCGI + url;
    }
    var posQuestion = url.indexOf('?');
    if (bustcachevar) {
        page_request.sessionID = new Date().getTime();
        bustcacheparameter = (posQuestion != -1) ? "&bust=" + page_request.sessionID : "";
    }
    if (sessionStorage.getItem("projectID") && posQuestion != -1) {
        url += `&projectID=${sessionStorage.getItem("projectID")}`
    }
    if (gAjaxPreSubmissionCallback) {
        funcLink(gAjaxPreSubmissionCallback, url, parameter, callbackfun, ispost);
    }
    if (formData) {
        page_request.method = "POST";
        page_request.open('POST', url + bustcacheparameter, true);
        page_request.send(formData);

    } else if (ispost || url.length >= 2000) {
        var paramset = "";
        if (posQuestion != -1) {
            paramset = url.substring(posQuestion + 1);
            url = posQuestion < 1 ? "?" : url.substring(0, posQuestion);
        }
        page_request.method = "POST";
        paramset += bustcacheparameter;
        page_request.open('POST', url, true);
        page_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        page_request.send(paramset);
    } else {
        page_request.method = "GET";
        page_request.open('GET', url + bustcacheparameter, true);
        page_request.send(null);
    }
    return page_request;
}

function ajaxAutoUpdateLayer(container, text) {
    var v = gObject(container); if (!v) return;
    if (container.indexOf("pre") == 0) v.innerHTML = "<pre>" + text + "</pre>";
    else v.innerHTML = text;
}


function ajaxDynaLoadPage(page_request, parameter, callbackfun) {
    if (page_request.readyState == 4 && ((page_request.status >= 200) || window.location.href.indexOf("http") == -1)) {
        var sanitized_responseText = page_request.responseText;

        if (callbackfun != "") {
            callbackfun(parameter, sanitized_responseText, page_request);
        }
        else ajaxAutoUpdateLayer(parameter, sanitized_responseText, page_request);
    }
}


function ajaxDynaRequestAction(url, ispost) {
    var page_request = false;
    if (window.XMLHttpRequest)
        page_request = new XMLHttpRequest();
    else if (window.ActiveXObject) {
        try {
            page_request = new ActiveXObject("Msxml2.XMLHTTP");
        }
        catch (e) {
            try {
                page_request = new ActiveXObject("Microsoft.XMLHTTP");
            }
            catch (e) { }
        }
    } else {
        return false;
    }
    if (gCGI && url.substring(0, 1) == '?') {
        url = gCGI + url;
    }
    var posQuestion = url.indexOf('?');
    if (bustcachevar) {
        bustcacheparameter = (url.indexOf("?") != -1) ? "&bust=" + new Date().getTime() : "";
    }
    if (sessionStorage.getItem("projectID") && posQuestion != -1) {
        url += `&projectID=${sessionStorage.getItem("projectID")}`
    }
    if (gAjaxPreSubmissionCallback) {
        funcLink(gAjaxPreSubmissionCallback, url, ispost);
    }

    if (ispost || url.length >= 2000) {
        var paramset = "";
        if (posQuestion != -1) {
            paramset = url.substring(posQuestion + 1);
            url = url.substring(0, posQuestion);
        }
        paramset += bustcacheparameter;

        page_request.open('POST', url, true);
        page_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        page_request.send(paramset + bustcacheparameter);

    } else {
        page_request.open('GET', url + bustcacheparameter, true);
        page_request.send(null);
    }
}

function fileUpload(form, action_url, div_id) {
    var iframe = document.createElement("iframe");
    iframe.setAttribute("id", "upload_iframe");
    iframe.setAttribute("name", "upload_iframe");
    iframe.setAttribute("width", "0");
    iframe.setAttribute("height", "0");
    iframe.setAttribute("border", "0");
    iframe.setAttribute("style", "width: 0; height: 0; border: none;");

    form.parentNode.appendChild(iframe);
    window.frames['upload_iframe'].name = "upload_iframe";

    iframeId = document.getElementById("upload_iframe");

    var eventHandler = function () {

        if (iframeId.detachEvent) iframeId.detachEvent("onload", eventHandler);
        else iframeId.removeEventListener("load", eventHandler, false);

        if (iframeId.contentDocument) {
            content = iframeId.contentDocument.body.innerHTML;
        } else if (iframeId.contentWindow) {
            content = iframeId.contentWindow.document.body.innerHTML;
        } else if (iframeId.document) {
            content = iframeId.document.body.innerHTML;
        }

        document.getElementById(div_id).innerHTML = content;

        setTimeout('iframeId.parentNode.removeChild(iframeId)', 250);
    };

    if (iframeId.addEventListener) iframeId.addEventListener("load", eventHandler, true);
    if (iframeId.attachEvent) iframeId.attachEvent("onload", eventHandler);

    form.setAttribute("target", "upload_iframe");
    form.setAttribute("action", action_url);
    form.setAttribute("method", "post");
    form.setAttribute("enctype", "multipart/form-data");
    form.setAttribute("encoding", "multipart/form-data");

    form.submit();

    document.getElementById(div_id).innerHTML = "Uploading...";
}




function alertI(txt, id, description) {
    if (id === undefined) id = 'basic-alert';
    if (description === undefined) description = new Object();

    var alertTextDiv = gObject(id + "_content");
    if (!alertTextDiv) {
        var t = gCreateAlert(id, description);
        gCreateFloatingElements(t);
        alertTextDiv = gObject(id + "_content");
        if (!alertTextDiv)
            return;
    }
    alertTextDiv.innerHTML = txt;
    showAlert(id);
    positionAlert(id + "_container", description.coordinates);
}

function showAlert(id) {
    displayAlert('visible', id);
}

function hideAlert(id) {
    displayAlert('hidden', id);
}

function displayAlert(status, id) {
    if (id === undefined) id = 'basic-alert';
    var alertDiv = gObject(id);
    var alertInnerDiv = gObject(id + "_container");
    alertDiv.style.zIndex = ++gObjZindex;
    alertInnerDiv.style.zIndex = ++gObjZindex;
    alertDiv.style.visibility = status;
    gObjZindex -= 2;
}

function positionAlert(id, coordinates) {
    if (id === undefined) id = 'basic-alert';
    if (coordinates === undefined) coordinates = new Object();
    if (coordinates.x === undefined) coordinates.x = gPgW / 2;
    if (coordinates.y === undefined) coordinates.y = gPgH / 2;


    var alertBox = gObject(id);
    if (!alertBox) {
        return;
    }
    var x = coordinates.x;
    var y = coordinates.y;
    var height = parseInt(__getCurrentComputedStyle(alertBox, 'height'));
    var width = parseInt(__getCurrentComputedStyle(alertBox, 'width'));
    alertBox.style.left = (x - width / 2) + "px";
    alertBox.style.top = (y - height / 2) + "px";
}

function gCreateAlert(id, description) {
    if (id === undefined) id = 'basic-alert';
    if (description === undefined) description = new Object();
    if (description.position === undefined) description.position = 'absolute';
    if (description.coordinates === undefined) description.coordinates = new Object();
    if (description.coordinates.x === undefined) description.coordinates.x = gPgW / 2;
    if (description.coordinates.y === undefined) description.coordinates.y = gPgH / 2;
    if (description.hidden === undefined) description.hidden = false;
    if (description.icon === undefined) description.icon = "img/yellowInfo.png";
    if (description.button === undefined) description.button = new Object();
    if (description.button.value === undefined) description.button.value = "OK";
    if (description.button.onclick === undefined) description.button.onclick = "hideAlert('" + id + "');";

    var t = "<div class='overlay'  id='" + id + "' style='visibility:" + (description.hidden ? "hidden" : "visible") + "'  > " +
        "<div id='" + id + "_container' class='Alert' style='opacity:1;position:" + (description.position ? description.position : "absolute") + ";left:" + description.coordinates.x + "px;top:" + description.coordinates.y + "px ;'>" +
        "<table><tr ><td class='Alert_content' width='10%' ><img id='" + id + "_icon' src='" + description.icon + "' height='24' width='24'></span></td>" +
        "<td class='Alert_content' align='right' width ='90%'><pre id='" + id + "_content'></pre></td>" +
        "</tr><tr><td  class='Alert_button' colspan='2' align='right' > <input type='button' " +
        "onclick=\"" + sanitizeElementAttr(description.button.onclick) + "\" " +
        "onkeypress=\"alert(event.keyCode);if(event.keyCode == 13)" + sanitizeElementAttr(description.button.onclick) + "\" " +
        "value=\"" + sanitizeElementAttr(description.button.value) + "\"> </td></tr>" +
        "</div></div>";
    return t;
}

function docMakeSelect(txt, curval, namebase, onchangefun) {
    var arrt = txt.split("|");

    var selt = "<select class='inputEditable' name='" + sanitizeElementAttr(namebase) + "' onchange='" + sanitizeElementAttr(onchangefun) + "' >";
    for (var im = 1; im < arrt.length; ++im) {
        selt += "<option value='" + sanitizeElementAttr(arrt[im]) + "' " + ((curval == arrt[im]) ? "selected" : "") + ">" + arrt[im] + "</option>";
    }
    selt += "</select>";
    return selt;
}

function docLocValue(fld, defval, loc) {
    if (!loc) loc = "" + document.location;
    var isrch = loc.indexOf(fld + "=");
    if (isrch != -1 && (isrch == 0 || loc.charAt(isrch - 1) == '&' || loc.charAt(isrch - 1) == '?')) {
        var pc = loc.substring(isrch + fld.length + 1);
        var esrch = pc.indexOf("&");
        if (esrch != -1) pc = pc.substring(0, esrch);
        return unescape(pc);
    }
    return defval ? defval : "";
}

function removeAllSpecificArgInURL(fld, url) {
    var newUrl = url;
    while (newUrl.length != 0) {
        var isrch = url.indexOf(fld);
        if (isrch != -1) {
            var pc = url.substring(isrch, url.length);
            var esrch = pc.indexOf("&");
            if (esrch != -1) {
                newUrl = url.substring(0, isrch - 1) + url.substring(isrch + esrch, url.length);
                url = newUrl;
            }
            else {
                newUrl = "";
                url = url.substring(0, isrch - 1);
            }
        }
        else {
            newUrl = "";
        }
    }
    return url;
}

function docFileNameCopy(gform, hiddenFileInput, descriptionInput) {
    var flnm = "";
    if (gform) flnm = gform.elements[hiddenFileInput].value;
    else hiddenFileInput;

    var ppos = flnm.lastIndexOf("/");
    if (ppos == -1) ppos = flnm.lastIndexOf("\\");
    flnm = flnm.substring(ppos + 1);
    if (!gform) return flnm;

    if (!descriptionInput) descriptionInput = hiddenFileInput + "-name";
    if (gform.elements[descriptionInput]) gform.elements[descriptionInput].value = flnm;
    return flnm;

}

function docCreateAppendIFrame(userKey, iframeName, iframeSrc, iframeParent, w, h) {
    var fr = document.createElement("iframe");
    fr.setAttribute("src", iframeSrc + "?userKey=" + userKey);
    fr.setAttribute("frameBorder", 0);
    fr.setAttribute("scrolling", 'no');
    fr.setAttribute("width", w);
    fr.setAttribute("height", h);
    gObject(iframeParent).appendChild(fr);

}

function docZoomShift(zoomX, what, zmin) {

    var op = what.substring(0, 1);
    var coef = parseFloat(what.substring(1));

    if (op == '0') {
        zoomX[0] = 0;
        zoomX[1] = 1;
        return zoomX;
    }

    zoomwin = zoomX[1] - zoomX[0];
    zoomctr = (zoomX[1] + zoomX[0]) / 2;
    if (!zmin) zmin = 0.001;


    if (op == '*') zoomwin *= coef;
    else if (op == '/') zoomwin /= coef;
    else if (op == '+') zoomctr += zoomwin * coef;
    else if (op == '-') zoomctr -= zoomwin * coef;
    else if (op == '=') zoomctr = coef;
    if (zoomwin > 1) zoomwin = 1;
    if (zoomwin < zmin) zoomwin = zmin;
    zoomX[0] = zoomctr - zoomwin / 2;
    zoomX[1] = zoomctr + zoomwin / 2;

    if (zoomX[0] < 0) {
        zoomX[0] = 0;
        zoomX[1] = zoomX[0] + zoomwin;
    }
    if (zoomX[1] > 1) {
        zoomX[1] = 1;
        zoomX[0] = zoomX[1] - zoomwin;
    }

    return zoomX;
}

var gDoc_assocArray = new Array();
function docAssocArray(keylist, vallist, spliter, doempty) {
    if (doempty)
        gDoc_assocArray = new Array();

    var j, ia, iv;
    var jrr;
    if (!spliter) jrr = keylist.split(/\s,/);
    else jrr = keylist.split(spliter);

    if (!spliter) vrr = vallist.split(/\s,/);
    else vrr = vallist.split(spliter);

    for (j = 0; j < jrr.length; ++j) {
        if (!jrr[j] || jrr[j].length == 0) continue;

        for (ia = 0; ia < gDoc_assocArray.length; ++ia) {
            if (gDoc_assocArray[ia][0] == jrr[j])
                break;
        }

        if (ia == gDoc_assocArray.length) {
            gDoc_assocArray.push(new Array());
            gDoc_assocArray[ia][0] = jrr[j];
        }

        for (var il = 0; il < vrr.length; ++il) {
            var val = vrr[il];
            if (!val || !val.length) continue;

            for (iv = 1; iv < gDoc_assocArray[ia].length; ++iv) {
                if (gDoc_assocArray[ia][iv] == val)
                    break;
            }
            if (iv == gDoc_assocArray[ia].length)
                gDoc_assocArray[ia].push(val);
        }
    }

}

function printAssocArray(assArr) {
    var t = "";
    for (var i = 0; i < assArr.length; ++i) {
        t += "#" + i + " " + assArr[i][0] + "[" + assArr[i].length + "] = ";
        for (var j = 1; j < assArr[i].length; ++j) {
            t += "#" + j + " " + assArr[i][j] + "  ";
        }
        t += "\n";
    }
    return t;
}

function listAssocArray(assArr, doIncl, inum) {
    var totArr = new Array();
    var t = "";
    for (var i = 0; i < assArr.length; ++i) {
        if (doIncl > 0 && i != inum) continue;
        if (doIncl < 0 && i == inum) continue;

        for (var j = 1; j < assArr[i].length; ++j) {

            var k;
            for (k = 0; k < totArr.length; ++k)
                if (totArr[k] == assArr[i][j]) break;
            if (k < totArr.length) continue;
            if (t.length) t += ",";
            t += assArr[i][j];
            totArr[totArr.length] = assArr[i][j];
        }
    }
    return t;
}

function gDoc_flags() { }
gDoc_flags.display = { extended: 0x00000001, ordinal: 0x00000002, showall: 0x00000004 };

function docAssArr_makeTable(prefix, assArr, cbFunc, container, dispFlags, filterlist) {
    if (assArr.length == 0) return;

    var frr = new Array();
    if (filterlist) frr = filterlist.split(/\s/);

    var t = "<table class='QP_svcTbl'><tr>";
    if (dispFlags & gDoc_flags.display.ordinal)
        t += "<th>&bull;</th>";
    t += "<th>";
    if (dispFlags & gDoc_flags.display.showall)
        t += "<a href='javascript:" + cbFunc + "(-1,0,\"all\")' >";
    t += prefix;
    if (dispFlags & gDoc_flags.display.showall)
        t += "</a>";
    t += "</th>";
    t += "</tr>";

    for (var i = 0; i < assArr.length; ++i) {

        if (frr.length) {
            for (var il = 0; il < frr.length; ++il) if (frr[il].length && frr[il] == assArr[i][0]) break;
            if (il == frr.length) continue;
        }

        var idnam = sanitizeElementId(prefix + "_" + assArr[i][0]);
        t += "<tr>";
        if (dispFlags & gDoc_flags.display.ordinal)
            t += "<td><input type=checkbox name='" + prefix + "_chk_" + sanitizeElementId(assArr[i][0]) + "' >" + (i + 1) + "</td>";

        t += "<td>";
        if (dispFlags & gDoc_flags.display.extended)
            t += ovis(idnam + "_extended", "sectHid") + "&nbsp;";
        t += "<a href='javascript:" + cbFunc + "(" + i + "," + 0 + ",\"" + idnam + "\")' >" + assArr[i][0] + "</a>";
        if (dispFlags & gDoc_flags.display.extended) {
            t += "<span id='" + idnam + "_extended' class='sectHid' ><table border=0>";
            for (var ia = 1; ia < assArr[i].length; ++ia)
                t += "<tr><td width=16></td><td><a href='javascript:" + cbFunc + "(" + i + "," + ia + ",\"" + sanitizeElementAttrJS(prefix + assArr[i][ia]) + "\")' >" + assArr[i][ia] + "</a></td></tr>";
            t += "</table></span>";
        }
        t += "</td>";
        t += "</tr>";
    }
    t += "</table>";

    if (container) gObject(container).innerHTML = t;
    else return t;
}


function docAssArr_tabSetMakeChoice(prefix, isel, cnt, tohide, toshow) {
    var hid = tohide.split(",");
    var sho = toshow.split(",");

    for (var i = 0; i < hid.length; ++i) {
        var o = gObject(hid[i]); if (!o) continue;
        o.className = "sectHid";
    }
    for (var i = 0; i < sho.length; ++i) {
        var o = gObject(sho[i]); if (!o) continue;
        o.className = "sectVis";
    }
    for (var i = 0; i < cnt; ++i) {
        var o = gObject(prefix + i); if (!o) continue;
        if (i == isel) o.className = "tab_selected";
        else o.className = "tab_unselected";
    }

}



function docAssArr_makeTabset(prefix, assArr, cbFunc, container, filterlist) {
    if (assArr.length == 0) return;
    if (!cbFunc) cbFunc = "docAssArr_tabSetMakeChoice";

    var frr = new Array();
    if (filterlist) frr = filterlist.split(/\s/);

    var t = "<table cellspacing=0 cellpadding=0><tr>";
    prefix = sanitizeElementId(prefix);

    for (var i = 0; i < assArr.length; ++i) {
        var tohide = listAssocArray(assArr, -1, i);
        var toshow = listAssocArray(assArr, +1, i);

        if (frr.length) {
            for (var il = 0; il < frr.length; ++il) if (frr[il].length && frr[il] == assArr[i][0]) break;
            if (il == frr.length) continue;
        }

        t += "<td>";
        t += "<table border=0 cellspacing=0 cellpadding=0  class='tab_unselected' id='" + prefix + "" + i + "' ><tr>";

        t += "<td width=2  ></td>";
        t += "<td ><a href='javascript:" + cbFunc + "(\"" + prefix + "\"," + i + "," + assArr.length + ",\"" + sanitizeElementAttrJS(tohide) + "\",\"" + sanitizeElementAttrJS(toshow) + "\")' >" + assArr[i][0] + "</a></td>";
        t += "<td width=2 >&nbsp;</td>";
        t += "</tr></table>";

        t += "</td>";

    }
    t += "</tr></table>";
    if (container) gObject(container).innerHTML = t;
    else return t;
}

var gInitList = "";
var gCreateScriptElements = "";
var gFloatingElements = "";
var gIdleSearchList = "";
var gDataFormName = "sCgi_DataForm";
var gDataForm;
var gSysVersion = 0;

function getBaseUrl() {
    var loc = window.location;
    return loc.protocol + "
}

function makeCmdSafe(aCmd) {
    var url = aCmd;
    if (aCmd.indexOf("?cmd=") != 0 && aCmd.indexOf("?cmdr=") != 0) {
        url = "?cmd=" + url;
    }
    return url;
}

function linkSelf(aSect, newin) {
    var url = makeCmdSafe(aSect);
    url = gCGI + url;
    if (newin) {
        window.open(url, newin);
    } else {
        window.location.href = url;
    }
}

function linkURL(url, newin, callback) {
    if (newin == 'ajax') {
        if (callback == "-" || !callback) ajaxDynaRequestAction(url);
        else ajaxDynaRequestPage(url, undefined, callback);
    }
    else if (newin) window.open(url, newin);
    else window.location.href = url;

}

function linkCmd(cmd, container, callback) {
    if (callback == "-") ajaxDynaRequestPage(makeCmdSafe(cmd), container, "");
    else if (callback) ajaxDynaRequestPage(makeCmdSafe(cmd), container, callback);
    else ajaxDynaRequestAction(makeCmdSafe(cmd));
}

function linkCGI(url, container, callback) {
    if (callback == "-") ajaxDynaRequestPage("?" + url, container, "");
    else if (callback) ajaxDynaRequestPage("?" + url, container, callback);
    else ajaxDynaRequestAction("?" + url);
}


function setLocationTitle(tt) {
    var txt = (typeof process_ID === 'undefined') ? tt : `${process_ID} ${tt}`;
    var v = gObject("cgiLocation");
    if (v) { v.innerHTML = txt; }
    document.title = txt;
}

let projectHandler = (function () {
    return {
        setProjectName: function (name) {
            sessionStorage.setItem("projectName", name);
            return this;
        },
        setProjectNickname: function (nickname) {
            sessionStorage.setItem("projectNickname", nickname);
            return this;
        },
        setProjectID: function (id) {
            sessionStorage.setItem("projectID", id);
            return this;
        },
        removeProjectInfo: function () {
            sessionStorage.removeItem("projectID");
            sessionStorage.removeItem("projectName");
            sessionStorage.removeItem("projectNickname");
            return this;
        },
        getProjectID: function () {
            return sessionStorage.getItem("projectID");
        },
        getProjectNickname: function () {
            return sessionStorage.getItem("projectNickname");
        },
        getProjectName: function () {
            return sessionStorage.getItem("projectName");
        }
    };
})();

function gInit() {
    if (!Array.prototype.indexOf) {
        Array.prototype.indexOf = function (obj, start) {
            for (var i = (start || 0), j = this.length; i < j; i++) {
                if (this[i] === obj) { return i; }
            }
            return -1;
        };
    }
    document.onmousemove = gOnMouseMove;
    document.onmouseup = gOnMouseUp;
    document.onmousedown = gOnMouseDown;


    document.onclick = gOnDocumentClick;

    gGetPgParams();
    gDataForm = document.forms[gDataFormName];

    if (gInitList.length) {
        eval(gInitList);
    }
}

function gUserGetInfo(what, redirect) {
    if (!what) what = 'name';
    var val;
    if (what == 'name') {
        val = cookieGet('userName');
    } else if (what == 'email') {
        val = cookieGet('email');
    } else {
        return '';
    }

    window.onfocus = function () { gUserLoginAccess(0); };

    return val;
}

function gUserLoginAccess(follow_url, params) {
    var uName = gUserGetInfo("name");
    if (!uName || uName == "Guest") {
        if (!follow_url) {
            follow_url = "" + document.location;
        }
        if (params && typeof (params) == "object") {
            for (var i in params) {
                urlExchangeParameter(follow_url, i, params[i]);
            }
        }
        if (follow_url.indexOf("?cmd=") != 0) {
            follow_url = follow_url.split("?cmd=")[1];
        }

        var cmd_url = "login" + "&follow=" + protectFields(follow_url);
        linkSelf(cmd_url);
    }
}

function gCreateFloatingDiv(add, div) {
    if (!div) div = "dvFloatingContainerDiv";

    if (add.length) {
        var o = gObject(div);


        if (!o) return;

        if (o.innerHTML) o.insertAdjacentHTML("beforeend", add);
        else o.innerHTML = add;
    }
}

function gCreateFloatingElements(add, div) {
    gFloatingElements = add;

    if (!div) div = "basic-floatingElements";

    if (gFloatingElements.length) {
        var o = gObject(div);


        if (!o) {
            return;
        }
        if (o.innerHTML) o.insertAdjacentHTML("beforeend", gFloatingElements);
        else o.innerHTML = gFloatingElements;
    }
}

function gIdleSearch() {

    if (gIdleSearchList.length)
        eval(gIdleSearchList);
}
setInterval("gIdleSearch()", 3000);


function formData(elem) {
    if (!elem) return "";
    var elem = gDataForm.elements[elem]; if (!elem) return "";
    return elem.value;
}



function asFindSWF(movieName) {
    if (navigator.appName.indexOf("Microsoft") != -1) {
        return window[movieName];
    } else {
        return document[movieName];
    }
}



function jjj(ssstr) { return JSON.stringify(ssstr); }




function trim(s) {
    s = s.replace(/(^\s*)|(\s*$)/gi, "");
    s = s.replace(/[ ]{2,}/gi, " ");
    s = s.replace(/\n /, "\n");
    return s;
}

function quoteForCSV(string) {
    if (string === undefined || string === null)
        string = "";
    else if (typeof (string) !== "string")
        string += "";

    if (/[,"\r\n]/.test(string)) {
        string = string.replace(/"/g, '""');
        string = '"' + string + '"';
    }
    return string;
}

function arr2Dmaxrow(arr, icol, skiprow, cntrow) {
    var imax = skiprow;
    for (var ir = skiprow + 1; ir < arr.length; ++ir) {
        if (cntrow && ir >= skiprow + cntrow) break;
        if (arr[ir][icol] > arr[imax][icol]) imax = ir;
    }
    return imax;
}



function CSVToArray(strData, strDelimiter) {
    if (strData === undefined || strData.length < 1) {
        return strData;
    }
    strDelimiter = (strDelimiter || ",");

    var objPattern = new RegExp(
        (
            "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +

            "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +

            "([^\"\\" + strDelimiter + "\\r\\n]*))"
        ),
        "gi"
    );


    var arrData = [[]];

    var arrMatches = null;


    while (arrMatches = objPattern.exec(strData)) {

        var strMatchedDelimiter = arrMatches[1];

        if (
            strMatchedDelimiter.length &&
            (strMatchedDelimiter != strDelimiter)
        ) {

            arrData.push([]);

        }


        var strMatchedValue = 0;
        if (arrMatches[2]) {

            strMatchedValue = arrMatches[2].replace(
                new RegExp("\"\"", "g"),
                "\""
            );

        } else {

            strMatchedValue = arrMatches[3];

        }


        arrData[arrData.length - 1].push(strMatchedValue);
    }

    return (arrData);
}

function CSVToArray2(strData, strDelimiter) {
    if (strData === undefined || strData.length < 1) {
        return strData;
    }
    strDelimiter = (strDelimiter || ",");

    var objPattern = new RegExp(
        (
            "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +

            "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +

            "([^\"\\" + strDelimiter + "\\r\\n]*))"
        ),
        "gi"
    );


    var arrData = [[]];

    var arrMatches = null;


    while (arrMatches = objPattern.exec(strData)) {

        var strMatchedDelimiter = arrMatches[1];

        if (strMatchedDelimiter.length && (strMatchedDelimiter != strDelimiter) && arrMatches) {

            arrData.push([]);

        }


        var strMatchedValue = 0;
        if (arrMatches[2]) {

            strMatchedValue = arrMatches[2].replace(
                new RegExp("\"\"", "g"),
                "\""
            );

        } else {

            strMatchedValue = arrMatches[3];

        }

        if (arrMatches) arrData[arrData.length - 1].push(strMatchedValue);
    }
    if (arrData[0].length !== arrData[arrData.length - 1].length && arrData[arrData.length - 1].length < 2) {
        arrData = arrData.slice(0, -1)
    }
    return (arrData);
}

function ArrayToObjArray(array, key_index = 0, combine_by_id = false) {
    let new_object = [];

    array.map(function (item, i) {
        if (i != key_index) {
            let obj = {};
            array[key_index].map(function (key, j) {
                obj[key] = item[j];
            })
            new_object.push(obj)
        }
    })
    let id_sorted_obj = {}
    if (combine_by_id) {
        new_object.map(function (obj, i) {
            const id = obj.id ? obj.id : 'none';
            if (!id_sorted_obj[id]) id_sorted_obj[id] = {};

            id_sorted_obj[id][obj.name] = obj.value
        })
        new_object = id_sorted_obj;
    }

    return new_object;

}

function matrixCopy(m) {
    var n = [];
    for (var i = 0; i < m.length; i++) {
        n[i] = [];
        for (var j = 0; j < m[i].length; j++)
            n[i].push(m[i][j]);
    }
    return n;
}

function matrixDiag(d) {
    var i, i1, j, n = d.length, A = Array(n), Ai;
    for (i = n - 1; i >= 0; i--) {
        Ai = Array(n);
        i1 = i + 2;
        for (j = n - 1; j >= i1; j -= 2) {
            Ai[j] = 0;
            Ai[j - 1] = 0;
        }
        Ai = d[i];
        if (j > i) { Ai[j] = 0; }
        for (j = i - 1; j >= 1; j -= 2) {
            Ai[j] = 0;
            Ai[j - 1] = 0;
        }
        if (j === 0) { Ai[0] = 0; }
        A[i] = Ai;
    }
    return A;
}

function matrixRep(s, v, k) {
    if (typeof k === "undefined") { k = 0; }
    var n = s[k], ret = Array(n), i;
    if (k === s.length - 1) {
        for (i = n - 2; i >= 0; i -= 2) { ret[i + 1] = v; ret[i] = v; }
        if (i === -1) { ret[0] = v; }
        return ret;
    }
    for (i = n - 1; i >= 0; i--) { ret[i] = numeric.rep(s, v, k + 1); }
    return ret;
}

function arrayIdentity(n) {
    return arrayDiag(arrayRep([n], 1));
}


function matrixInverse(a) {
    var x = a.slice(0);
    if (x.length === 0) {
        return null;
    }
    if (!matrixIsSquare(x) || matrixIsSingular(x)) {
        return null;
    }
    var rows = x.length, ir = rows, ic, jr;
    var matrixRT = matrixRightTriangular(matrixAugment(x, matrixIdentity(rows)));
    var cols = matrixRT[0].length;
    var inverted = [], cell, scol, divisor;

    while (ir--) {

        scol = [];
        inverted[ir] = [];
        divisor = matrixRT[ir][ir];
        for (ic = 0; ic < cols; ++ic) {
            cell = matrixRT[ir][ic] / divisor;
            scol.push(cell);
            if (ic >= rows) {
                inverted[ir].push(cell);
            }
        }
        matrixRT[ir] = scol;

        jr = ir;
        while (jr--) {
            scol = [];
            for (ic = 0; ic < cols; ic++) {
                scol.push(matrixRT[jr][ic] - matrixRT[ir][ic]
                    * matrixRT[jr][ir]);
            }
            matrixRT[jr] = scol;
        }
    }
    return inverted;
}

function matrixIsSquare(x) {
    var cols = (x.length === 0) ? 0 : x[0].length;
    return (x.length === cols);
}

function matrixIsSingular(x) {
    return (matrixIsSquare(x) && matrixDeterminant(x) === 0);
}

function matrixDeterminant(x) {
    if (x.length === 0) {
        return 1;
    }
    if (!matrixIsSquare(x)) {
        return null;
    }
    var matrixRT = matrixRightTriangular(x);
    var det = matrixRT[0][0], rows = matrixRT.length;
    for (var i = 1; i < rows; i++) {
        det = det * matrixRT[i][i];
    }
    return det;
}

function matrixRightTriangular(x) {
    if (x.length === 0)
        return new Array();
    var M = x.slice(0), els;
    var rows = x.length, ir, jr, cols = x[0].length, ic;
    for (ir = 0; ir < rows; ir++) {
        if (M[ir][ir] === 0) {
            for (jr = ir + 1; jr < rows; jr++) {
                if (M[jr][ir] !== 0) {
                    els = [];
                    for (ic = 0; ic < cols; ic++) {
                        els.push(M[ir][ic] + M[jr][ic]);
                    }
                    M[ir] = els;
                    break;
                }
            }
        }
        if (M[ir][ir] !== 0) {
            for (jr = ir + 1; jr < rows; jr++) {
                var coef = M[jr][ir] / M[ir][ir];
                els = [];
                for (ic = 0; ic < cols; ic++) {
                    els.push(ic <= ir ? 0 : M[jr][ic] - M[ir][ic] * coef);
                }
                M[jr] = els;
            }
        }
    }
    return M;
}

function matrixAugment(x, y) {
    if (x.length === 0) {
        return x;
    }
    var M = y.slice(0);

    if (typeof (y[0][0]) === 'undefined') {
        for (var i = 0; i < M.length; ++i)
            M[i] = [M[i]];
    }
    var xcols = x[0].length, ir = x.length;
    var ycols = 1, ic;
    if (M[0].length !== undefined)
        ycols = M[0].length;

    var T = [];
    while (T.push(new Array(xcols + ycols)) < ir)
        ;
    for (var i = 0; i < ir; ++i) {
        for (var j = 0; j < xcols; ++j)
            T[i][j] = x[i][j];
    }

    if (ir !== M.length) {
        return null;
    }
    while (ir--) {
        ic = ycols;
        while (ic--) {
            T[ir][xcols + ic] = M[ir][ic];
        }
    }
    return T;
}

function matrixIdentity(n) {
    var I = [], i = n, j;
    while (i--) {
        j = n;
        I[i] = [];
        while (j--) {
            I[i][j] = (i === j) ? 1 : 0;
        }
    }
    return I;
}

function matrixMultiplicationToVector(matrix, Coordinate) {
    var crdMatrix = new Array();
    if (!Coordinate.z)
        Coordinate.z = 0;
    crdMatrix.push([Coordinate.x], [Coordinate.y], [Coordinate.z], [1]);
    var vectorResult = matrixMultiplication(matrix, crdMatrix);

    var newCoordinate = new Object({
        x: vectorResult[0][0],
        y: vectorResult[1][0],
        z: vectorResult[2][0]
    });
    return newCoordinate;
}

function matrixCreateEmpty(rows, columns) {
    var matrix = new Array();
    for (var r = 0; r < rows; r++) {
        var ro = new Array();
        for (var c = 0; c < columns; c++) {
            ro.push(0);
        }
        matrix.push(ro);
    }
    return matrix;
}

function matrixMultiplication(matrix1, matrix2) {
    var matrixFinal = new Array();
    matrixFinal = matrixCreateEmpty(matrix1.length, matrix2[0].length);
    for (var r = 0; r < matrix1.length; r++) {
        for (var c = 0; c < matrix2[0].length; c++) {
            var addition = 0;
            for (var i = 0; i < matrix1[0].length; i++) {
                var multiplication;
                multiplication = matrix1[r][i] * matrix2[i][c];
                addition = addition + multiplication;
            }
            matrixFinal[r][c] = addition;
        }
    }
    return matrixFinal;
}

function innerProduct(vec1, vec2) {
    var ret = 0;
    var dim = vec1.length;
    for (var i = 0; i < dim; i++)
        ret += vec1[i] * vec2[i];
    return ret;
}

function matrixRQDecompose(matrix) {
    var dim = matrix.length;
    var R = matrixCreateEmpty(dim, dim);
    var Q = matrixCreateEmpty(dim, dim);

    for (var i = dim - 1; i >= 0; i--)
        for (var k = dim - 1; k >= 0; k--)
            Q[i][k] = matrix[i][k];

    for (var i = dim - 1; i >= 0; i--) {
        var len = 0;
        for (var k = dim - 1; k >= 0; k--)
            len += Q[i][k] * Q[i][k];
        len = Math.sqrt(len);

        for (var k = dim - 1; k >= 0; k--)
            Q[i][k] = len > 0 ? Q[i][k] / len : 0;

        for (var j = i - 1; j >= 0; j--) {
            var innerQiQj = innerProduct(Q[i], Q[j]);
            for (var k = dim - 1; k >= 0; k--)
                Q[j][k] -= innerQiQj * Q[i][k];
        }
    }

    for (var i = 0; i < dim; i++) {
        for (var j = i; j < dim; j++) {
            R[i][j] = innerProduct(matrix[i], Q[j]);
        }
    }

    return { R: R, Q: Q };
}

function matrixAffineTSRDecompose(matrix) {
    var RQ = matrixRQDecompose(matrix);
    var T = matrixIdentity(4);
    var S = matrixIdentity(4);
    var R = matrixIdentity(4);

    for (var i = 0; i < 3; i++) {
        T[i][3] = RQ.R[i][3];
        S[i][i] = RQ.R[i][i];
        for (var j = 0; j < 3; j++)
            R[i][j] = RQ.Q[i][j];
    }

    return { T: T, S: S, R: R };
}

function matrixPrint(matrix) {
    var s = "[ ";
    for (var i = 0; i < matrix.length; i++) {
        if (i)
            s += "\n";

        s += " [";

        for (var j = 0; j < matrix[i].length; j++) {
            if (j)
                s += "  ";
            s += matrix[i][j];
        }
        s += "]";
    }
    s += " ]";
    return s;
}


function str2ranges(str, delemiter) {
    if (!delemiter)
        delemiter = ",";
    var arr = str.split(delemiter).sort(function (a, b) {
        return parseInt(a) - parseInt(b);
    });
    var dest = arr[0], dif = 0;
    for (var i = 1; i < arr.length; ++i) {
        dif = parseInt(arr[i]) - parseInt(arr[i - 1]);
        if (dif > 1) {
            if (dest.substring(dest.length - 1) == "-")
                dest += arr[i - 1];
            if ((dest.substring(dest.length - 1) != "-")
                && (dest.substring(dest.length - 1) != delemiter))
                dest += delemiter;
            dest += arr[i];
        } else if (dif == 1) {
            if (dest.substring(dest.length - 1) != "-")
                dest += "-";
        }
    }
    if (dest.substring(dest.length - 1) == "-")
        dest += arr[arr.length - 1];
    if (dest.substring(dest.length - 1) == ";")
        dest = dest.substring(0, dest.length - 2);
    return dest;
}

function arraybinarySearch(array, find, callFunc, closest, negative) {
    var low = 0, high = array.length - 1, i;
    if (!callFunc) {
        while (low <= high) {
            i = Math.floor((low + high) / 2);

            if (array[i] < find) {
                low = i + 1;
                continue;
            }

            if (array[i] > find) {
                high = i - 1;
                continue;
            }
            return i;
        }
    } else {
        while (low <= high) {
            i = Math.floor((low + high) / 2);
            var dif = callFunc(array[i], find);
            if (dif < 0) {
                low = i + 1;
                continue;
            }

            if (dif > 0) {
                high = i - 1;
                continue;
            }
            return i;
        }
    }
    var res = null;
    if (closest) {
        if (high < 0) high = 0;
        if (low >= array.length) low = array.length - 1;
        if (negative === true)
            res = high;
        else if (negative === false)
            res = low;
        else {
            var difH, difL;
            if (callFunc) {
                difH = callFunc(array[high], find);
                difL = -callFunc(array[low], find);
            } else {
                difH = array[high] - find;
                difL = find - array[low];
            }

            if (difH < difL)
                res = low;
            else
                res = high;
        }
    }
    return res;
}


function img_create(src, alt, title) {
    var img = (navigator.appName == 'Microsoft Internet Explorer') ? new Image() : document.createElement('img');
    img.src = src;
    if (alt != null) img.alt = alt;
    if (title != null) img.title = title;
    return img;
}

function eventOrigins(eventHolder, event, skiplist) {
    var list = traverseChildren(eventHolder);
    if (skiplist) list = list.concat(skiplist);
    var e = event.relatedTarget || event.target;
    if (!!~list.indexOf(e))
        return 0;
    else
        return 1;

}

function traverseChildren(elem) {
    var children = [];
    var q = [];

    q.push(elem);

    while (q.length > 0) {
        var elem = q.pop();
        children.push(elem);
        var elemArray = isok(elem.children) ? elem.children : elem.childNodes;
        for (var i = 0; i < elemArray.length; i++)
            q.push(elemArray[i]);
    }
    return children;
}


function isNumber(n) {
    var t_n = parseFloat(n);
    return !isNaN(t_n) && isFinite(t_n);
}

function Int(n) {
    if (isNumber(n))
        return parseInt(n);
    else return 0;
}

function Float(n) {
    if (isNumber(n))
        return parseFloat(n);
    else return 0;
}



function stopEventPropagation(e) {
    var event = e || window.event;
    if (!event && typeof (evt) != "undefined") event = evt;
    if (!event) return false;
    if (event.stopPropagation) {
        event.stopPropagation();
    }
    else {
        event.cancelBubble = true;
    }
}

function mouseEnter(evt, elem, fnc) {
    var relTarget = evt.relatedTarget;
    if (elem === relTarget || gIsChildOf(elem, relTarget)) {
        return;
    }
    funcLink(fnc, elem);
}
function mouseLeave(evt, elem, fnc) {
    var relTarget = evt.relatedTarget;
    if ((elem === relTarget || gIsChildOf(elem, relTarget))) {
        return;
    }
    funcLink(fnc, elem);
}

function gIsChildOf(parent, child) {
    if (parent === child) {
        return false;
    }
    while (child && child !== parent) {
        child = child.parentNode;
    }
    return child === parent;
}

var Handler = (function () {
    var i = 1,
        listeners = {};

    return {
        addListener: function (element, event, handler, capture) {
            addEventSimple(element, event, handler, capture);
            listeners[i] = {
                element: element,
                event: event,
                handler: handler,
                capture: capture
            };
            return i++;
        },
        removeListener: function (id) {
            if (id in listeners) {
                var h = listeners[id];
                removeEventSimple(h.element, h.event, h.handler, h.capture);
            }
        }
    };
}());

function addEventSimple(obj, evt, fn) {
    if (obj.addEventListener)
        obj.addEventListener(evt, fn, false);
    else if (obj.attachEvent)
        obj.attachEvent('on' + evt, fn);
}

function removeEventSimple(obj, evt, fn) {
    if (obj.removeEventListener)
        obj.removeEventListener(evt, fn, false);
    else if (obj.detachEvent)
        obj.detachEvent('on' + evt, fn);
}


function gValidAncestorOnAttribute(curNode, attributes, value) {
    attributes = verarr(attributes);
    for (var ia = 0; ia < attributes.length; ++ia) {
        if (curNode[attributes[ia]] == value || curNode.getAttribute(attributes[ia]) == value) return curNode;
    }

    if (curNode.parentNode && curNode.tagName != "HTML")
        return gValidAncestorOnAttribute(curNode.parentNode, attributes, value);
    else
        return null;

}

function gValidAncestorOnProp(curNode, tag, value) {
    if (curNode[tag] && curNode[tag] == value) return curNode;
    else {
        if (curNode.parentNode && curNode.tagName != "HTML")
            return gValidAncestorOnProp(curNode.parentNode, tag, value);
        else return null;
    }
}

function gSwapElements(obj1, obj2) {

    var temp = document.createElement("div");
    obj1.parentNode.insertBefore(temp, obj1);
    obj2.parentNode.insertBefore(obj1, obj2);
    temp.parentNode.insertBefore(obj2, temp);
    temp.parentNode.removeChild(temp);
}

function __gScrollBarWidth() {
    var inner = document.createElement('p');
    inner.style.width = "100%";
    inner.style.height = "200px";

    var outer = document.createElement('div');
    outer.style.position = "absolute";
    outer.style.top = "0px";
    outer.style.left = "0px";
    outer.style.visibility = "hidden";
    outer.style.width = "200px";
    outer.style.height = "150px";
    outer.style.overflow = "hidden";
    outer.appendChild(inner);

    document.body.appendChild(outer);
    var w1 = inner.offsetWidth;
    outer.style.overflow = 'scroll';
    var w2 = inner.offsetWidth;
    if (w1 == w2) w2 = outer.clientWidth;

    document.body.removeChild(outer);

    return (w1 - w2);
};

function __getIEVersion() {
    var rv = -1;
    if (navigator.appName == 'Microsoft Internet Explorer') {
        var ua = navigator.userAgent;
        var re = new RegExp("MSIE ([0-9]{1,}[\.0-9]{0,})");
        if (re.exec(ua) != null)
            rv = parseFloat(RegExp.$1);
    }
    return rv;
}

function __isMobile() {
    return false;

    var agents = ['android', 'webos', 'iphone', 'ipad', 'blackberry', 'mobile'];
    for (var i = 0; i < agents.length; i++) {
        if (navigator.userAgent.match(new RegExp(agents[i], 'i'))) {
            return true;
        }
    }
    return false;
}


var gEnv_list = { "desktop": { dir: "" }, "mobile": { dir: "mobile/" } };

function __isEnvironmentMatch(env_ck) {
    return gEnv_list[env_ck].dir == __getHTMLDir();

}

function __setHTMLDir(value) {
    cookieSet("sCgi_HtmlDir", value);
}

function __getHTMLDir() {
    return cookieGet("sCgi_HtmlDir");
}

function __getEnvironment() {
    if (__isMobile()) {
        return "mobile";
    }
    return "desktop";
}

function __getEnvironmentCookie() {
    return cookieGet("browse_env");
}

function __setEnvironmentCookie(value) {
    if (!value) {
        cookieDelete("browse_env");
    } else {
        cookieSet("browse_env", value);
    }
}

function __setEnvironment(force) {
    var env = __getEnvironment();
    var env_ck = __getEnvironmentCookie();

    if (force) {
        if (!env_ck) {
            return false;
        }
        else if (!__isEnvironmentMatch(env_ck)) {
            __setHTMLDir(gEnv_list[env_ck].dir);
            return true;
        }
        return false;
    }
    else if (!env_ck) {
        if (env != "desktop") {
            __setEnvironmentCookie(env);
            __setHTMLDir(gEnv_list[env].dir);
            return true;
        }
        return false;
    }
    return false;
}

function __updateEnvironment(force) {
    if (__setEnvironment(force)) {
        location.reload(true);
    }
}

function __switchEnvironment(env_value) {
    __setEnvironmentCookie(env_value);
    __updateEnvironment(true);
}

function __getOperaVersion() {
    var rv = 0;
    if (window.opera) {
        var sver = window.opera.version();
        rv = parseFloat(sver);
    }
    return rv;
}
var __userAgent = navigator.userAgent;
var __isIE = navigator.appVersion.match(/MSIE|Trident/) != null;

var __IEVersion = __getIEVersion();
var __isIENew = __isIE && __IEVersion >= 8;
var __isIEOld = __isIE && !__isIENew;

var __isFireFox = __userAgent.match(/firefox/i) != null;
var __isFireFoxOld = __isFireFox && ((__userAgent.match(/firefox\/2./i) != null) ||
    (__userAgent.match(/firefox\/1./i) != null));
var __isFireFoxNew = __isFireFox && !__isFireFoxOld;

var __isWebKit = navigator.appVersion.match(/WebKit/) != null;
var __isChrome = navigator.appVersion.match(/Chrome/) != null;
var __isOpera = window.opera != null;
var __operaVersion = __getOperaVersion();
var __isOperaOld = __isOpera && (__operaVersion < 10);

var compatibility = new Object();
__cnt = 0;

function stopDefault(e) {
    if (!e) e = event;
    if (!e) e = window.event;
    if (e.preventDefault) {
        e.preventDefault();
    }
    else {
        e.returnValue = false;
    }
    return false;
}
function dragDrop() {
    this.initialMouseX = undefined;
    this.initialMouseY = undefined;
    this.startX = undefined;
    this.startY = undefined;
    this.draggedObject = undefined;
    this.onDropCallback = undefined;
    this.onStartCallback = undefined;
    this.onMoveCallback = undefined;
    this.onTargetCallback = undefined;
    this.onCancelCallback = undefined;
    this.debug = gObject("debug");
    this.dropableAttrs = new Array();
    this.dropOnAttrs = new Array();

    this.EventEnabled = false;
    this.startStyle = {
        top: undefined,
        left: undefined,
        zIndex: undefined,
        position: undefined
    };

    this.cloneDraggedObject = function () {
        if (typeof (this.setClonedObject) != "undefined") {
            var clElem = this.setClonedObject.func.call(this.setClonedObject.obj, this.draggedObject);
            if (clElem) {
                this.draggedObject = clElem;
            } else if (this.draggedObject) {
                this.draggedObject = this.draggedObject.cloneNode(true);
            }
        } else if (this.draggedObject) {
            this.draggedObject = this.draggedObject.cloneNode(true);
        }

        if (!this.draggedObject) {
            return false;
        }

        this.draggedObject.style.height = "24px";
        this.draggedObject.style.width = "24px";
        var childList = traverseChildren(this.draggedObject);
        for (a in childList) {
            if (childList[a].className !== undefined) {
                childList[a].className = '';
            }
        }
        this.draggedObject.className = 'DnD';

        this.offsetParent.appendChild(this.draggedObject);

        return true;
    };

    this.initDropElement = function (element, dropOn) {
        var dropOnAttrs = verarr(this.dropOnAttrs);
        if (dropOn) {
            dropOnAttrs = verarr(dropOn);
        }
        for (var id = 0; id < dropOnAttrs.length; ++id) {
            element.setAttribute(dropOnAttrs[id], 'true');
        }
    };

    this.initDragElement = function (element, onDrop, onStart, onMove, onTarget, onStop, onCancel) {
        if (typeof element == 'string')
            element = document.getElementById(element);
        var that = this;
        element.onmousedown = function (e) {
            that.startDragMouse(e, element);
        };

        this.onDropCallback = onDrop;
        this.onStartCallback = onStart;
        this.onMoveCallback = onMove;
        this.onTargetCallback = onTarget;
        this.onStopCallback = onStop;
        this.onCancelCallback = onCancel;
    };
    this.startDragMouse = function (e, init_element) {
        var evt = e || window.event;
        var that = this;

        this.onscroll = Handler.addListener(window, 'scroll', function (e) {
            that.scrollMouse(e);
        }, false);
        this.onmousemove = Handler.addListener(document, 'mousemove', function (e) {
            that.dragMouse(e);
        }, false);
        this.onmouseup = Handler.addListener(document, 'mouseup', function (e) {
            that.releaseElement(e);
        }, false);

        this.startDrag(evt, init_element);

        this.initialMouseX = evt.clientX;
        this.initialMouseY = evt.clientY;
        this.initialScrollX = document.body.scrollLeft;
        this.initialScrollY = document.body.scrollTop;


        return false;
    };
    this.startDrag = function (evt, init_element) {
        stopDefault(evt);

        if (this.draggedObject)
            this.releaseElement();

        var obj = evt.currentTarget || init_element;

        this.draggedObject = obj;
        this.sourceObj = verarr(obj);
        this.sourceTriggerObj = obj;
        this.offsetParent = document.body;

        var dx = evt.clientX + 5;
        var dy = evt.clientY + 5;
        this.startX = dx;
        this.startY = dy;

        if (typeof (this.onStartCallback) != "undefined") {
            if (!this.onStartCallback.func.call(this.onStartCallback.obj,
                this.sourceObj)) {
                this.cancelled = true;
                this.releaseElement();
                return false;
            }
        }

        if (this.debug) {
            this.debug.innerHTML = "<div><b>&nbsp&nbspInitial absolutePositions</b> &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp <b>Offsets</b></div>";
            this.debug.innerHTML += "<div>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbspx: "
                + pos.x
                + " &nbsp y:"
                + pos.y
                + "&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp x:"
                + dx + " &nbsp y" + dy + "</div>";
        }

        this.sourceObj.every(function (e) { e.setAttribute('dragged', true); });
        return false;
    };

    this.scrollMouse = function (e) {
        var evt = e || window.event;
        stopDefault(evt);
        var dx = document.body.scrollLeft - this.initialScrollX;
        var dy = document.body.scrollTop - this.initialScrollY;
        this.initialScrollX = document.body.scrollLeft;
        this.initialScrollY = document.body.scrollTop;
        if (!this.EventEnabled) {
            if (!this.cloneDraggedObject.call(this)) {
                return;
            }
        } else {
            dx += Int(parseInt(this.draggedObject.style.left)) - this.startX;
            dy += Int(parseInt(this.draggedObject.style.top)) - this.startY;
        }
        this.EventEnabled = true;

        this.setPosition(dx, dy);

    };

    this.dragMouse = function (e) {
        var evt = e || window.event;
        stopDefault(evt);
        var dX = evt.clientX - this.initialMouseX + (document.body.scrollLeft);
        var dY = evt.clientY - this.initialMouseY + (document.body.scrollTop);

        if (Math.abs(dX - (document.body.scrollLeft)) < 3 && Math.abs(dY - document.body.scrollTop) < 3) {
            return false;
        }

        if (!this.EventEnabled) {
            if (!this.cloneDraggedObject.call(this)) {
                return;
            }
        }
        this.EventEnabled = true;

        this.setPosition(dX, dY);

        this.detectDrop(e);

        if (typeof (this.onDragCallback) != "undefined")
            this.onDragCallback.func.call(this.onDragCallback.obj,
                this.sourceObj);

        if (this.debug) {
            this.debug.innerHTML = "<div><b>&nbsp&nbsp&nbsp Initial styles</b> &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp <b>mouse Diff</b> &nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp <b>After style</b></div>";
            this.debug.innerHTML += "<div>&nbsp&nbsp x: "
                + this.draggedObject.style.left + " &nbsp y:"
                + this.draggedObject.style.top
                + "&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp x:" + dX
                + " &nbsp y:" + dY + " &nbsp; ";
        }

        return false;
    };

    this.setPosition = function (dx, dy) {
        this.draggedObject.style.left = this.startX + dx + "px";
        this.draggedObject.style.top = this.startY + dy + "px";
    };

    this.detectDrop = function (evt) {
        var target = evt.srcElement || evt.target;
        var tt = isok(this.dropableAttrs) ? gValidAncestorOnAttribute(target, this.dropableAttrs, "true") : null;
        if (this.dropElement) {
            if (this.dropElement != target) {
                this.dropElement.style.cursor = this.oldCursor;
                this.oldCursor = target.style.cursor;
            }
        } else {
            this.oldCursor = this.draggedObject.style.cursor;
        }
        this.dropElement = tt;
        if (this.dropElement
            && this.dropElement.getAttribute('dragged') != 'true') {
            gKeyCtrl = evt.ctrlKey || evt.metaKey;
            if (gKeyCtrl)
                this.dropElement.style.cursor = 'copy';
            else
                this.dropElement.style.cursor = 'move';
            this.dropAllowed = true;
            if (typeof (this.onTargetCallback) != "undefined")
                this.onTargetCallback.func.call(this.onTargetCallback.obj,
                    this.sourceObj, this.dropElement);
        } else {
            this.dropAllowed = false;
            this.dropElement = target;
            this.dropElement.style.cursor = 'not-allowed';
        }
    };

    this.releaseElement = function (e) {
        if (this.sourceTriggerObj == this.dropElement)
            gDragEvent = true;
        else
            gDragEvent = false;
        Handler.removeListener(this.onmousemove);
        if (!this.cancelled) Handler.removeListener(this.onmouseup);
        Handler.removeListener(this.onscroll);

        if (typeof (this.dropElement) != "undefined")
            this.dropElement.style.cursor = this.oldCursor;

        if (this.dropAllowed) {
            this.draggedObject.style.zIndex = this.startStyle.zIndex;
            if (typeof (this.onDropCallback) != "undefined")
                this.onDropCallback.func.call(this.onDropCallback.obj,
                    this.sourceObj, this.dropElement, gKeyCtrl);
        }
        if (typeof (this.onStopCallback) != "undefined")
            this.onStopCallback.func.call(this.onStopCallback.obj,
                this.sourceObj);
        if (this.EventEnabled)
            this.offsetParent.removeChild(this.draggedObject);
        else if (!this.cancelled) {
            if (typeof (this.onCancelCallback) != "undefined")
                this.onCancelCallback.func.call(this.onCancelCallback.obj,
                    this.sourceTriggerObj);

        }

        this.sourceObj.every(function (e) { e.setAttribute('dragged', false); });
        this.draggedObject = undefined;
        this.dropElement = undefined;
        this.dropAllowed = false;
        this.EventEnabled = false;
        this.cancelled = undefined;
    };
};


function __parseBorderWidth(width) {
    var res = 0;
    if (typeof (width) == "string" && width != null && width != "") {
        var p = width.indexOf("px");
        if (p >= 0) {
            res = parseInt(width.substring(0, p));
        } else {
            res = 1;
        }
    }
    return res;
}
function __getCurrentComputedStyle(el, attr) {
    var attrValue = undefined;
    if (attr == 'height' && __isIE) {
        return el.offsetHeight;
    }
    if (attr == 'width' && __isIE) {
        return el.offsetWidth;
    }
    if (window.getComputedStyle && !__isIE) {

        if (el == document) el = document.body;
        var styledeclaration = document.defaultView.getComputedStyle(el, null);
        if (!styledeclaration) return;
        attrValue = styledeclaration.getPropertyValue(attr);
    } else if (el.currentStyle) {
        attrValue = el.currentStyle[attr];
    }
    return attrValue ? attrValue : 0;
}

function __getBorderWidth(element) {
    var res = new Object();
    res.left = 0;
    res.top = 0;
    res.right = 0;
    res.bottom = 0;
    if (window.getComputedStyle) {
        var elStyle = window.getComputedStyle(element, null);
        res.left = parseInt(elStyle.borderLeftWidth.slice(0, -2));
        res.top = parseInt(elStyle.borderTopWidth.slice(0, -2));
        res.right = parseInt(elStyle.borderRightWidth.slice(0, -2));
        res.bottom = parseInt(elStyle.borderBottomWidth.slice(0, -2));
    } else {
        res.left = __parseBorderWidth(element.style.borderLeftWidth);
        res.top = __parseBorderWidth(element.style.borderTopWidth);
        res.right = __parseBorderWidth(element.style.borderRightWidth);
        res.bottom = __parseBorderWidth(element.style.borderBottomWidth);
    }

    return res;
}

function __isVisible(el) {
    while (el) {
        if (__getCurrentComputedStyle(el, 'display') == 'none' || __getCurrentComputedStyle(el, 'visibility') == 'hidden')
            return false;
        el = el.parentNode;
    }
    return true;
}

function getElementAbsolutePos(element) {
    var res = new Object();
    res.x = 0;
    res.y = 0;
    if (element !== null) {
        if (element.getBoundingClientRect) {
            var viewportElement = document.documentElement;
            var box = element.getBoundingClientRect();
            var scrollLeft = viewportElement.scrollLeft;
            var scrollTop = viewportElement.scrollTop;

            res.x = box.left + scrollLeft;
            res.y = box.top + scrollTop;

        } else {
            res.x = element.offsetLeft;
            res.y = element.offsetTop;

            var parentNode = element.parentNode;
            var borderWidth = null;

            while (offsetParent != null) {
                res.x += offsetParent.offsetLeft;
                res.y += offsetParent.offsetTop;

                var parentTagName = offsetParent.tagName.toLowerCase();

                if ((__isIEOld && parentTagName != "table")
                    || ((__isFireFoxNew || __isChrome) && parentTagName == "td")) {
                    borderWidth = kGetBorderWidth(offsetParent);
                    res.x += borderWidth.left;
                    res.y += borderWidth.top;
                }

                if (offsetParent != document.body
                    && offsetParent != document.documentElement) {
                    res.x -= offsetParent.scrollLeft;
                    res.y -= offsetParent.scrollTop;
                }

                if (!__isIE && !__isOperaOld || __isIENew) {
                    while (offsetParent != parentNode && parentNode !== null) {
                        res.x -= parentNode.scrollLeft;
                        res.y -= parentNode.scrollTop;
                        if (__isFireFoxOld || __isWebKit) {
                            borderWidth = kGetBorderWidth(parentNode);
                            res.x += borderWidth.left;
                            res.y += borderWidth.top;
                        }
                        parentNode = parentNode.parentNode;
                    }
                }

                parentNode = offsetParent.parentNode;
                offsetParent = offsetParent.offsetParent;
            }
        }
    }
    return res;
}

function gConstructCSSMetrics(num) {
    var m = num;
    if (m === undefined) return undefined;
    if (!(typeof (m) == 'string' && m.indexOf("%") != -1))
        m = m + "px";
    return m;
}

function gAncestorByTag(el, tagName) {
    var res = null;
    while (el) {
        if (el.tagName && el.tagName.toLowerCase() == tagName.toLowerCase()) {
            res = el;
            break;
        }
        el = el.parentNode;
    }
    return res;
}


function clipboard() {
    this.isCopy = false;
    this.content = new Array();
    this.objectsDependOnMe = new Array();

    this.reset = function () {
        this.content = [];
    };

    this.add = function () {
        this.reset();

        var that = this;

        this.append.apply(that, arguments);
    };

    this.append = function () {
        for (var i = 0; i < arguments.length; ++i) {
            var argument = arguments[i];
            if (argument instanceof Array && (arguments.length == 1)) {
                for (var r = 0; r < argument.length; ++r) {
                    this.content.push(argument[r]);
                }
            }
            else
                this.content.push(arguments[i]);
        }
    };

    this.copy = function (src) {
        this.isCopy = true;

        this._src = src;

        var that = this;

        var args = [].splice.call(arguments, 0);
        this.add.apply(that, args.splice(1));

        this.updateDependents();
    };

    this.cut = function (src) {
        this.isCopy = false;

        this._src = src;

        var that = this;

        var args = [].splice.call(arguments, 0);
        this.add.apply(that, args.splice(1));


        this.updateDependents();
    };

    this.paste = function (dst, objCls) {

        var objfunc = vjObjFunc('paste', objCls);
        if (objfunc && objfunc.func) {
            objfunc.func.call(objfunc.obj, this._src, dst, this.content, this.isCopy);
        }

        else {
            var url = "?cmd=";
            if (this.isCopy) {
                url += "objCopy";
            }
            else {
                url += "objCut";
            }
            if (this._src) {
                url += "&src=" + this._src;
            }
            url += "&ids=" + this.content.join(",");
            url += "&dest=" + dst;
            linkURL(url, "ajax");
        }
        if (!this.isCopy) {
            this.reset();
        }
        this.updateDependents();
    };

    this._delete = function (src, objCls, ids) {

        var objfunc = vjObjFunc('_delete', objCls);

        var ids = ids.split(",");

        if (objfunc && objfunc.func) {
            objfunc.func.call(objfunc.obj, src, ids);
        }

        else {
            var url = "?cmd=objRemove&src=";
            if (src) {
                url += src;
            }
            else {
                url += "root";
            }
            url += "&ids=" + ids.join(",");
            linkURL(url, "ajax");
        }
        this.updateDependents();
    };

    this.updateDependents = function () {
        if (!this.objectsDependOnMe) return;

        for (var im = 0; im < this.objectsDependOnMe.length; ++im) {
            if (typeof (this.objectsDependOnMe[im]) == 'string')
                this.objectsDependOnMe[im] = this.dataViewEngine.locate(this.objectsDependOnMe[im]);
            var depObj = this.objectsDependOnMe[im];

            if (depObj.composerFunction)
                depObj.composerFunction(depObj, depObj.getData(0).data);

        }
    };
}

gClip = new clipboard();


function user_setvar(variable, value) {
}
function user_getvar(variable, defaultValue, which) {
    return defaultValue;
}

function makeImgSrc(icon, sizedir) {
    if (icon.indexOf("?cmd=") != -1)
        return icon;

    if (!icon.match(/\.(gif|jpg|jpeg|png)$/i))
        icon += ".gif";
    if (icon.indexOf("/") == -1)
        icon = "img/" + icon;
    if (sizedir) {
        icon = icon.replace(new RegExp("(/[^/]+\.(gif|jpg|jpeg|png))$"), "/" + sizedir + "$1");
    }
    return icon;
}


function loadCSS(url) {
    var headID = document.getElementsByTagName("head")[0];
    var cssNode = document.createElement('link');
    cssNode.type = 'text/css';
    cssNode.rel = 'stylesheet';
    cssNode.href = url;
    cssNode.media = 'screen';
    headID.appendChild(cssNode);
}

function loadJS(url, callback) {
    let scriptElement = document.querySelector(`script[src="${url}"]`);

    if (scriptElement) {
        if (callback) {
            callback();
        }
        return;
    }

    scriptElement = document.createElement('script');
    scriptElement.type = 'text/javascript';
    scriptElement.src = url;

    if (callback) {
        scriptElement.onload = callback;
        scriptElement.onerror = function () {
            console.error(`Failed to load script: ${url}`);
        };
    }

    document.head.appendChild(scriptElement);
}

function setCookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
    var expires = "expires=" + d.toUTCString();
    document.cookie = cname + "=" + cvalue + "; " + expires;
}

function getCookie(cname) {
    var name = cname + "=";
    var ca = document.cookie.split(';');
    for (var i = 0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}

function classListAdd(elt, cls) {
    if (elt.classList !== undefined) {
        return elt.classList.add(cls);
    } else {
        var className = elt.className.baseVal !== undefined ? elt.className.baseVal : elt.className;
        var classList = className.split(/\s+/);

        if (className === "") {
            className = cls;
        } else if (classList.indexOf(cls) < 0) {
            className += " " + cls;
        }

        if (elt.className.baseVal !== undefined) {
            elt.className.baseVal = className;
        } else {
            elt.className = className;
        }
        return;
    }
}

function paginate(
    totalItems,
    currentPage = 1,
    pageSize = 10,
    maxPages = 10
) {
    let totalPages = Math.ceil(totalItems / pageSize);

    if (currentPage < 1) {
        currentPage = 1;
    } else if (currentPage > totalPages) {
        currentPage = totalPages;
    }

    let startPage, endPage;
    if (totalPages <= maxPages) {
        startPage = 1;
        endPage = totalPages;
    } else {
        let maxPagesBeforeCurrentPage = Math.floor(maxPages / 2);
        let maxPagesAfterCurrentPage = Math.ceil(maxPages / 2) - 1;
        if (currentPage <= maxPagesBeforeCurrentPage) {
            startPage = 1;
            endPage = maxPages;
        } else if (currentPage + maxPagesAfterCurrentPage >= totalPages) {
            startPage = totalPages - maxPages + 1;
            endPage = totalPages;
        } else {
            startPage = currentPage - maxPagesBeforeCurrentPage;
            endPage = currentPage + maxPagesAfterCurrentPage;
        }
    }

    let n = currentPage === 0 ? 1 : currentPage
    let startIndex = (n - 1) * pageSize;
    let endIndex = Math.min(startIndex + pageSize - 1, totalItems - 1);

    let pages = Array.from(Array((endPage + 1) - startPage).keys()).map(i => startPage + i);

    return {
        totalItems: totalItems,
        currentPage: currentPage,
        pageSize: pageSize,
        totalPages: totalPages,
        startPage: startPage,
        endPage: endPage,
        startIndex: startIndex,
        endIndex: endIndex,
        pages: pages
    };
}
