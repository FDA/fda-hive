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
import SetCache from './SetCache.js'
import some from '../some.js'
import cacheHas from './cacheHas.js'

const COMPARE_PARTIAL_FLAG = 1
const COMPARE_UNORDERED_FLAG = 2

function equalArrays(array, other, bitmask, customizer, equalFunc, stack) {
  const isPartial = bitmask & COMPARE_PARTIAL_FLAG
  const arrLength = array.length
  const othLength = other.length

  if (arrLength != othLength && !(isPartial && othLength > arrLength)) {
    return false
  }
  const stacked = stack.get(array)
  if (stacked && stack.get(other)) {
    return stacked == other
  }
  let index = -1
  let result = true
  const seen = (bitmask & COMPARE_UNORDERED_FLAG) ? new SetCache : undefined

  stack.set(array, other)
  stack.set(other, array)

  while (++index < arrLength) {
    let compared
    const arrValue = array[index]
    const othValue = other[index]

    if (customizer) {
      compared = isPartial
        ? customizer(othValue, arrValue, index, other, array, stack)
        : customizer(arrValue, othValue, index, array, other, stack)
    }
    if (compared !== undefined) {
      if (compared) {
        continue
      }
      result = false
      break
    }
    if (seen) {
      if (!some(other, (othValue, othIndex) => {
        if (!cacheHas(seen, othIndex) &&
          (arrValue === othValue || equalFunc(arrValue, othValue, bitmask, customizer, stack))) {
          return seen.push(othIndex)
        }
      })) {
        result = false
        break
      }
    } else if (!(
          arrValue === othValue ||
            equalFunc(arrValue, othValue, bitmask, customizer, stack)
        )) {
      result = false
      break
    }
  }
  stack['delete'](array)
  stack['delete'](other)
  return result
}

export default equalArrays
