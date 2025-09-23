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
#ifndef sLib_qlang_engine_h
#define sLib_qlang_engine_h

#include <qlang/interpreter.hpp>
#include <qlang/parser.hpp>


namespace slib {
    namespace qlang {
        class Engine
        {
            protected:
                Context * _ctx;
                Parser _parser;

            public:
                Engine(bool noAlloc = false)
                {
                    _ctx = noAlloc ? 0 : new Context();
                }
                virtual ~Engine()
                {
                    delete _ctx;
                }
                virtual bool parse(const char * queryText, idx len = 0, sStr * errorMessage = NULL)
                {
                    if( !getParser().parse(queryText, len ? queryText + len : 0) ) {
                        if( errorMessage ) {
                            getParser().printError(*errorMessage);
                        }
                        return false;
                    }
                    return true;
                }
                virtual bool parseTemplate(const char * tmplText, idx len = 0, sStr * errorMessage = NULL)
                {
                    if( !getParser().parse(tmplText, len ? tmplText + len : 0, Parser::fTemplate) ) {
                        if( errorMessage ) {
                            getParser().printError(*errorMessage);
                        }
                        return false;
                    }
                    return true;
                }
                virtual bool eval(sVariant & result, sStr * errorMessage = NULL)
                {
                    qlang::ast::Node * ast = getAstRoot();
                    if( ast ) {
                        if( !ast->eval(result, getContext()) ) {
                            if( errorMessage ) {
                                getContext().printError(*errorMessage);
                            }
                            return false;
                        }
                    } else {
                        if( errorMessage ) {
                            errorMessage->printf("no valid query to eval");
                        }
                        return false;
                    }
                    return true;
                }
                virtual sVariant* run(sStr * errorMessage = NULL)
                {
                    ast::Node * ast = getAstRoot();
                    if( ast ) {
                        if( ast->run(getContext()) ) {
                            return &(getContext().getReturnValue());
                        }
                        if( errorMessage ) {
                            getContext().printError(*errorMessage);
                        }
                    } else {
                        errorMessage->printf("no valid query to run");
                    }
                    return NULL;
                }
                virtual void reset()
                {
                    getContext().reset();
                }

                virtual Context& getContext()
                {
                    return *_ctx;
                }
                virtual Parser& getParser()
                {
                    return _parser;
                }

                virtual ast::Node* getAstRoot()
                {
                    return getParser().getAstRoot();
                }

                virtual bool hasBuiltin(const char * name)
                {
                    return getContext().hasBuiltin(name);
                }
                virtual bool copyBuiltin(const char * newname, const char * oldname)
                {
                    return getContext().copyBuiltin(newname, oldname);
                }
                virtual void registerBuiltinIdxPtr(const char * name, idx * ptr, bool isConst = false)
                {
                    getContext().registerBuiltinIdxPtr(name, ptr, isConst);
                }
                virtual void registerBuiltinIdxConst(const char * name, idx val)
                {
                    getContext().registerBuiltinIdxConst(name, val);
                }
                virtual void registerBuiltinUdxPtr(const char * name, udx * ptr, bool isConst = false)
                {
                    getContext().registerBuiltinUdxPtr(name, ptr, isConst);
                }
                virtual void registerBuiltinUdxConst(const char * name, udx val)
                {
                    getContext().registerBuiltinUdxConst(name, val);
                }
                virtual void registerBuiltinRealPtr(const char * name, real * ptr, bool isConst = false)
                {
                    getContext().registerBuiltinRealPtr(name, ptr, isConst);
                }
                virtual void registerBuiltinRealConst(const char * name, real val)
                {
                    getContext().registerBuiltinRealConst(name, val);
                }
                virtual void registerBuiltinStringPtr(const char * name, sStr * ptr, bool isConst = false)
                {
                    getContext().registerBuiltinStringPtr(name, ptr, isConst);
                }
                virtual void registerBuiltinFunction(const char * name, const BuiltinFunction & funcObj)
                {
                    getContext().registerBuiltinFunction(name, funcObj);
                }
                virtual void registerBuiltinFunction(const char * name, Context::BuiltinRawCallback funcPtr, void * param)
                {
                    getContext().registerBuiltinFunction(name, funcPtr, param);
                }
                virtual bool registerBuiltinValue(const char * name, sVariant & value)
                {
                    return getContext().registerBuiltinValue(name, value);
                }
                virtual bool registerBuiltinThis(sVariant & value)
                {
                    return getContext().registerBuiltinThis(value);
                }
        };
    }
    ;
}
;

#endif
