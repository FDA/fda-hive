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
import eq from '../eq.js'
import equalArrays from './equalArrays.js'
import mapToArray from './mapToArray.js'
import setToArray from './setToArray.js'

const COMPARE_PARTIAL_FLAG = 1
const COMPARE_UNORDERED_FLAG = 2

const boolTag = '[object Boolean]'
const dateTag = '[object Date]'
const errorTag = '[object Error]'
const mapTag = '[object Map]'
const numberTag = '[object Number]'
const regexpTag = '[object RegExp]'
const setTag = '[object Set]'
const stringTag = '[object String]'
const symbolTag = '[object Symbol]'

const arrayBufferTag = '[object ArrayBuffer]'
const dataViewTag = '[object DataView]'

const symbolProto = Symbol ? Symbol.prototype : undefined
const symbolValueOf = symbolProto ? symbolProto.valueOf : undefined

function equalByTag(object, other, tag, bitmask, customizer, equalFunc, stack) {
  switch (tag) {
    case dataViewTag:
      if ((object.byteLength != other.byteLength) ||
          (object.byteOffset != other.byteOffset)) {
        return false
      }
      object = object.buffer
      other = other.buffer

    case arrayBufferTag:
      if ((object.byteLength != other.byteLength) ||
          !equalFunc(new Uint8Array(object), new Uint8Array(other))) {
        return false
      }
      return true

    case boolTag:
    case dateTag:
    case numberTag:
      return eq(+object, +other)

    case errorTag:
      return object.name == other.name && object.message == other.message

    case regexpTag:
    case stringTag:
      return object == `${other}`

    case mapTag:
      let convert = mapToArray

    case setTag:
      const isPartial = bitmask & COMPARE_PARTIAL_FLAG
      convert || (convert = setToArray)

      if (object.size != other.size && !isPartial) {
        return false
      }
      const stacked = stack.get(object)
      if (stacked) {
        return stacked == other
      }
      bitmask |= COMPARE_UNORDERED_FLAG

      stack.set(object, other)
      const result = equalArrays(convert(object), convert(other), bitmask, customizer, equalFunc, stack)
      stack['delete'](object)
      return result

    case symbolTag:
      if (symbolValueOf) {
        return symbolValueOf.call(object) == symbolValueOf.call(other)
      }
  }
  return false
}

export default equalByTag
