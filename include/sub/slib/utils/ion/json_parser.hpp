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

#pragma once
#ifndef sION_JSON_PARSER_HPP_
#define sION_JSON_PARSER_HPP_


#include <slib/utils/json/parser.hpp>
#include <slib/std/string.hpp>


namespace slib {

    class sJSON2IONQueryParser : public sJSONParser {
            class ionCommandParser {
                public:
                    ionCommandParser() : _cmd_buf()
                    {
                    }
                    virtual bool parse(const char * command, sVariant & current_variant) {
                        _cmd_buf.cut0cut();
                        return !!_cmd_buf.addString(command);
                    }
                    virtual const char * print(sStr & buf) {
                        return buf.addString(_cmd_buf.ptr());
                    }
                    const char * printStatement(sStr & buf) {
                        print(buf);
                        return buf.addString(";");
                    }
                    virtual const char * translate(sStr & buf, const char * command, sVariant & current_variant) {
                        if ( !parse(command, current_variant) ) {
                            return 0;
                        }
                        return printStatement(buf);
                    }
                protected:
                    sStr _cmd_buf;
            };

            class ionFunctionParser : public ionCommandParser{
                public:
                    ionFunctionParser() : ionCommandParser(), _parameter_buf()
                    {
                    }
                    typedef ionCommandParser TCommand;
                    virtual bool parse(const char * command, sVariant & current_variant) {
                        _parameter_buf.cut0cut();

                        if ( !current_variant.isDic() ) {
                            return false;
                        }
                        sVariant * parameters_var = current_variant.getDicElt(sJSON2IONQueryParser::command(eWhere));
                        if (!parameters_var) {
                            return false;
                        }
                        if (!parameters_var->isList()) {
                            return false;
                        }
                        for (idx i = 0; i < parameters_var->dim() ; ++i ) {
                            _parameter_buf.addString(",");
                            sString::unescapeFromJSON(_parameter_buf,parameters_var->getListElt(i)->asString());

                        }
                        return TCommand::parse(command,current_variant);
                    }
                    virtual const char * print(sStr & buf) {
                        TCommand::print(buf);
                        return buf.printf("(%s)",_parameter_buf.ptr(1));
                    }
                protected:
                    sStr _parameter_buf;
            };

            class ionResultParser: public ionFunctionParser {
                public:
                    ionResultParser(): ionFunctionParser(), _page_buf(), _assign_buf()
                    {
                    }
                    typedef ionFunctionParser TFunction;
                    virtual bool parse(const char * command, sVariant & current_variant) {
                        _page_buf.cut0cut();
                        _assign_buf.cut0cut();
                        sVariant * page_var = current_variant.getDicElt(sJSON2IONQueryParser::command(eSubset));
                        if (page_var) {
                            _page_buf.addString(page_var->asString());
                        }
                        sVariant * assign_var = current_variant.getDicElt(sJSON2IONQueryParser::command(eAssign));
                        if (assign_var) {
                            _assign_buf.addString(assign_var->asString());
                        }
                        return TFunction::parse(command,current_variant);
                    }
                    virtual const char * print(sStr & buf) {
                        if(_assign_buf.length()) {
                            buf.printf("%s=",_assign_buf.ptr());
                        }
                        TFunction::print(buf);
                        if(_page_buf.length()) {
                            buf.printf("%s",_page_buf.ptr());
                        }
                        return buf.ptr();
                    }
                protected:
                    sStr _page_buf;
                    sStr _assign_buf;
            };

            class ionTraverseParser : public ionResultParser{
                public:
                    ionTraverseParser() : ionResultParser(), _relation_buf(), _scope_buf()
                    {
                    }
                    typedef ionResultParser TResult;
                    virtual bool parse(const char * command, sVariant & current_variant) {
                        _relation_buf.cut0cut();
                        _scope_buf.cut0cut();
                        sVariant * relation_var = current_variant.getDicElt(sJSON2IONQueryParser::command(eTable));
                        if (relation_var) {
                            _relation_buf.addString(relation_var->asString());
                        }
                        sVariant * scope_var = current_variant.getDicElt(sJSON2IONQueryParser::command(eScope));
                        if (scope_var) {
                            _scope_buf.addString(scope_var->asString());
                        }
                        return TResult::parse(command,current_variant);
                    }
                    virtual const char * print(sStr & buf) {
                        if(_assign_buf.length()) {
                            buf.printf("%s=",_assign_buf.ptr());
                        }
                        if(_scope_buf.length()) {
                            buf.printf("%s:",_scope_buf.ptr());
                        }
                        TCommand::print(buf);
                        if(_relation_buf.length() ) {
                            buf.printf(".%s",_relation_buf.ptr());
                        }
                        buf.printf("(%s)",_parameter_buf.ptr(1));
                        if(_page_buf.length()) {
                            buf.printf("%s",_page_buf.ptr());
                        }
                        return buf.ptr();
                    }
                protected:
                    sStr _relation_buf;
                    sStr _scope_buf;
            };

        public:
            sJSON2IONQueryParser()
                : sJSONParser(), db_ids(), qry(), _error_log(), _warning_log()
            {
            }

            bool parseIQS(const char * iqs);
            sStr db_ids;
            sStr qry;
            const char * getLog() {return _error_log.ptr(1);}
            void logError(const char * fmt, ...) __attribute((format(printf, 2, 3)))
            {
                sStr tmp;
                va_list args;
                tmp.vprintf(fmt, args);
                _error_log.printf("\nIQS JSON parser error: %s", tmp.ptr());
            }
            void logWarning(const char * fmt, ...) __attribute((format(printf, 2, 3)))
            {
                sStr tmp;
                va_list args;
                tmp.vprintf(fmt, args);
                _warning_log.printf("\nIQS JSON parser warning: %s", tmp.ptr());
            }
            ionCommandParser * geCommandParser(sVariant & iqs_db);

        private:
            sStr _error_log, _warning_log;
            static const char * _list_json_keys;
            static const char * _list_ion_command_keys;
            bool _parseDBs(sVariant & iqs_db);
            bool _parseQuery(sVariant & iqs_query);
            ionCommandParser _commandParser;
            ionResultParser _resultParser;
            ionFunctionParser _functionParser;
            ionTraverseParser _traverseParser;
            enum EJSONKeys_enum
            {
                eDB = 0, eQuery
            };
            typedef enum EJSON2IONCommand_enum {
                eCommand = 0,
                eFunction,
                eResult,
                eTraverse,
                eUnkownCommand
            } EJSON2IONCommand;
        public:
            typedef enum EJSON2IONCommandParameters_enum {
                eWhere = 0,
                eAssign,
                eSubset,
                eTable,
                eScope,
                eUnkownParameter
            } EJSON2IONCommandParameters;
            static const char * command(idx my_num) {
                return sString::next00(_list_ion_command_keys,my_num);
            }
    };
}





#endif 