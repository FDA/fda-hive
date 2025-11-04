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
#include <slib/utils/ion/json_parser.hpp>

using namespace slib;

const char * sJSON2IONQueryParser::_list_json_keys = "db" _ "query" __;
const char * sJSON2IONQueryParser::_list_ion_command_keys = "where" _ "assign" _ "subset" _ "table" _ "scope" __;


sJSON2IONQueryParser::ionCommandParser * sJSON2IONQueryParser::geCommandParser(sVariant & iqs_db) {
    EJSON2IONCommand res = eCommand;
    if ( iqs_db.isNull() || iqs_db.isNullish()) {
        res = eCommand;
    }
    if( !iqs_db.isDic() ) {
        logError("Command values described as a JSON object");
        return 0;
    }
    idx command_parameter_num = -1;
    for (idx i = 0 ; i < iqs_db.dim(); ++i ) {
        const char * command_parameter = iqs_db.getDicKey(i);
        if ( command_parameter ){
            sString::compareChoice(command_parameter, _list_ion_command_keys, &command_parameter_num, false, 0, true);
            switch (command_parameter_num) {
                case eWhere:
                    if ( res < eFunction)
                        res = eFunction;
                    break;
                case eSubset:
                case eAssign:
                    if ( res < eResult)
                        res = eResult;
                    break;
                case eScope:
                case eTable:
                    if ( res < eTraverse)
                        res = eTraverse;
                    break;
                default:
                    logWarning("Unkown ION parameter : %s", command_parameter);
                    break;
            }
        }
    }
    switch (res) {
        case eFunction:
            return &_functionParser;
        case eResult:
            return &_resultParser;
        case eTraverse:
            return &_traverseParser;
        default:
        case eCommand:
            return &_commandParser;
            break;

    }
    return 0;
}
bool sJSON2IONQueryParser::_parseQuery(sVariant & iqs_query) {
    if( !iqs_query.isList() ) {
        logError("Query must be a JSON array : %s", iqs_query.asString());
        return false;
    }

    for (idx i = 0 ; i < iqs_query.dim(); ++i ) {
        sVariant & iqs_command = *iqs_query.getListElt(i);
        if( !iqs_command.isDic() ) {
            logError("Query must be a JSON object : %s", iqs_command.asString());
            return false;
        }
        if( !iqs_command.dim() ) {
            logWarning("Command is empty: %s", iqs_command.asString());
            continue;
        }
        ionCommandParser * my_ionCommandParser = geCommandParser(*iqs_command.getDicElt(0));
        if (!my_ionCommandParser) {
            return false;
        }
        my_ionCommandParser->translate(qry,iqs_command.getDicKey(0),*iqs_command.getDicElt(0));
    }
    return true;
}

bool sJSON2IONQueryParser::_parseDBs(sVariant & iqs_db) {
    sVec<sVariant *> var_dbs;
    db_ids.cut0cut();
    switch (iqs_db.getType() ) {
        case sVariant::value_DIC:
            var_dbs.vadd(1,&iqs_db);
            break;
        case sVariant::value_LIST:
            for(idx i = 0; i < iqs_db.dim(); ++i) {
                var_dbs.vadd(1,iqs_db.getListElt(i));
            }
            break;
        default:
            logError("List of dbs must be a JSON object or array: %s", iqs_db.asString());
            return false;
    }
    for(idx i = 0; i < var_dbs.dim(); ++i) {
        sVariant * cur_db = var_dbs[i];
        if ( !cur_db->isDic() ) {
            logError("Each db must be a JSON object: %s", cur_db->asString());
        }
        sVariant * cur_db_id = cur_db->getDicElt("objID");
        if( db_ids.length())
            db_ids.addString(",");
        db_ids.addString(cur_db_id->asString());
    }
    return db_ids.length();
}

bool sJSON2IONQueryParser::parseIQS(const char * iqs_string){
    if ( !iqs_string ) {
        logError(" Missing 'iqs' argument");
        return false;
    }
    sStr error;
    if( unlikely(!parse(iqs_string)) ) {
        printError(error);
        logError("JSON parser failed to parse TQS query: %s", error.ptr());
        return false;
    }
    sVariant & rs = result();
    if( rs.getType() != sVariant::value_DIC ) {
        logError("input must be a JSON object");
        return false;
    }

    idx cmdnum = -1;
    for(idx i = 0; i < rs.dim(); ++i) {
        sString::compareChoice(rs.getDicKey(i), _list_json_keys, &cmdnum, false, 0, true);
        switch ( cmdnum ) {
            case eDB:
            case eQuery:
                break;
            default:
                logWarning( "Unkown command : %s", rs.getDicKey(i));
                break;
        }
    }
    sVariant * iqs_db = rs.getDicElt("db");
    if( iqs_db ) {
        if( !_parseDBs(*iqs_db) ) {
            return false;
        }
    } else {
        logError( "Missing ION db info");
        return false;
    }

    sVariant * iqs_query = rs.getDicElt("query");
    if( iqs_query ) {
        if (!_parseQuery(*iqs_query) ){
            return false;
        }
    } else {
        logError( "Missing query statements");
        return false;
    }
    return true;
}
