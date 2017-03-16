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

#include <slib/core/rc.hpp>
#include <slib/core/str.hpp>

using namespace slib;

const sRC sRC::zero;

struct SlibRcEntryStrings
{
        const char * enum_name;
        const char * description;
};

#undef SLIB_RC_ENTRY
#undef SLIB_RC_ENTRY_WITH_VAL
#define SLIB_RC_ENTRY(id, description) { #id, description }
#define SLIB_RC_ENTRY_WITH_VAL(id, val, description) { #id, description }

//static
const char * sRC::operation2string(sRC::EOperation op, bool asEnumName)
{
    static SlibRcEntryStrings strings[] = {
        SLIB_RC_OPERATION_ENTRIES
    };
    int index = (op == 255) ? sDim(strings) - 1 : (int)op;
    return asEnumName ? strings[index].enum_name : strings[index].description;
}

//static
const char * sRC::entity2string(sRC::EEntity entity, bool asEnumName)
{
    static SlibRcEntryStrings strings[] = {
        SLIB_RC_ENTITY_ENTRIES
    };
    int index = (entity == 255) ? sDim(strings) - 1 : (int)entity;
    return asEnumName ? strings[index].enum_name : strings[index].description;
}

//static
const char * sRC::state2string(sRC::EState state, bool asEnumName)
{
    static SlibRcEntryStrings strings[] = {
        SLIB_RC_STATE_ENTRIES
    };
    int index = (state == 255) ? sDim(strings) - 1 : (int)state;
    return asEnumName ? strings[index].enum_name : strings[index].description;
}

const char * sRC::print(sStr * out, bool asEnumNames) const
{
    static sStr buf;
    if( out == 0 ) {
        out = &buf;
        buf.cut(0);
    }
    idx len = out->length();

    const char * op_string = operation2string((EOperation)val.parts.op, asEnumNames);
    const char * op_target_string = entity2string((EEntity)val.parts.op_target, asEnumNames);
    const char * bad_entity_string = entity2string((EEntity)val.parts.bad_entity, asEnumNames);
    const char * state_string = state2string((EState)val.parts.state, asEnumNames);

    if( op_string ) {
        out->addString(op_string);
        out->addString(" ");
    }
    out->addString(op_target_string);
    if( (op_string || op_target_string) && (bad_entity_string || state_string) ) {
        out->addString(": ");
    }
    if( bad_entity_string ) {
        out->addString(bad_entity_string);
        out->addString(" ");
    }
    out->addString(state_string);

    return out->ptr(len);
}
