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
#ifndef sLib_cgisql_h
#define sLib_cgisql_h

#include <slib/std/cgi.hpp>
#include <ulib/usr.hpp>
#include <ulib/uquery.hpp>

namespace slib {

    class sUsrCGI: public sCGI
    {
            typedef sCGI TParent;
        public:

            sUsrCGI(idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false,
                    bool immediate = true, const char * forcedMethod = 0)
                    : TParent(argc, argv, envp, readfrom, isCookie, immediate, forcedMethod), m_apiVersion(sUdxMax)
            {
            }
            virtual ~sUsrCGI()
            {
            }

        public:

            virtual idx Cmd(const char * cmd);
            virtual bool OnCGIInit(void);

            virtual const char* selfURL(sStr& url);

            void logout(void);
            void login(void);
            void SSO(void);
            void loginInfo(void);
            void batch(void);
            void passwordReset(void);
            void passwordEdit(void);
            void passwordChange(void);
            void userSet(bool show);
            void userReg(void);
            void userList(void);
            void userInfo(void);
            void groupList(void);
            void groupAdd(void);
            void file2(void);
            void file(void);
            void dropboxList(void);
            void dropboxLoad(void);
            void userResendEmailVerify();
            void userVerifyEmail();
            void userResendAccountActivate();
            void userAccountActivate();
            void userGroupActivate();
            void userForgot();
            void inactList();
            void userMgr();
            void sendmail();

            void typeTree(void);
            void objs();
            void objQry(sVariant * dst = 0, const char * qry_text = 0);
            void objDel(void);
            void objRemove(void);
            void objCount(void);
            void objMove(bool isCopy = false);
            void folderCreate(void);
            void propSpec(const char* type_name = 0, const char* view_name = 0, const char* props = 0);
            void propSpec2(const char* type_name = 0, const char* view_name = 0, const char* props = 0);
            void propSpec3(const char* types = 0);
            void propGet(sUsrObjRes * res = 0, const char* view_name = 0, const char* props = 0, sJSONPrinter * json_printer = 0);
            void propSet(sVar* form = 0);
            void propSet2(sVar* form = 0);
            void propBulkSet(void);
            void propDel(void);
            void simpleCast(void);
            void action(const char* action_name = 0, const char* type_name = 0, const char* obj_ids = 0);
            void allStat(void);
            void permSet(void);
            void permCopy(void);

            void timelogAddEntry(void);

            void ionObjList(void);
            void ionPropGet(void);

            void genToken(void);
            void loginToken(void);

            void availableProjects(void);
            void projectMembers(void);
            void addToProject(void);
            void removeFromProject(void);
            
            void moveCheck(void);
            void moveToProject(void);
            void moveToHome(void);
            void CBERConnect(void);
            void createProject(void);

            virtual sUsrQueryEngine* queryEngineFactory(idx flags);

        public:
            sUsr m_User;
            udx m_apiVersion;
        protected:
            virtual bool resolveFile(const char * filename);

        private:

            void loginCommon(const sUsr::ELoginResult eres, const char * email, const idx logCount);
            bool loadAppPage(const char * app);

            sStr m_SID;
    };

}
;

#endif 