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
#ifndef sLib_qusql_h
#define sLib_qusql_h

#include <ssql/mysql.hpp>
#include <slib/std/string.hpp>

namespace slib
{
    class sQuSql
    {
        const char * qReq, * qDat;
        public:

        sSql * db;

        sQuSql( sSql * lsql,const char * req="QueueReq", const char * dat="QueueData") 
        {
            db=lsql;
            qReq=req;
            qDat=dat;
        } 

        idx submit( const char * svc, const char * descr="") 
        {
            sStr sql;
            db->execute(sql.printf(0,"insert into %s (svc, descr ) values ('%s', '%s')",qReq,svc,descr));
            idx qid=0;
            db->sscanfTable( "select LAST_INSERT_ID()","%" DEC "", &qid, 1) ;
            return qid;
        } 

        idx resubmit( idx qid) 
        {
            sStr sql;
            db->execute(sql.printf(0, "update %s set status = %d, pid = 0 where qid = %" DEC "",qReq, eWaiting, qid));
            return qid;
        } 

        enum eStatus{eWaiting=0, eProcessing, eRunning, eDone, eError, eKilled};
        enum eAction{eNone=0, eRun, eKill};
        static const char * actionList00;
        static const char * statusList00;

        struct Request {
            idx qid, status, action;
            char svc[255],descr[1024],subdate[64], email[255];
        };

        idx getRequest(idx qid, Request * r)
        {
            sVarSet res;
            sStr sql;
            if(! db->getTable( sql.printf("select qid,status,action,svc,descr,subdate,email from %s where qid = %" DEC " ",qReq,qid), &res))
                return 0;

            memset(r,0,sizeof(Request));
            sscanf(res.val(0,0),"%" DEC,&r->qid );
            sscanf(res.val(0,1),"%" DEC,&r->status );
            sscanf(res.val(0,2),"%" DEC,&r->action );
            sscanf(res.val(0,3),"%s",r->svc );
            sscanf(res.val(0,4),"%s",r->descr );
            sscanf(res.val(0,5),"%s",r->subdate );
            sscanf(res.val(0,6),"%s",r->email );
            return 1;

        }
        void setStatus(idx qid, idx status)
        {
            sStr sql;
            db->execute(sql.printf("update %s set status = %" DEC " where qid = %" DEC "",qReq, status, qid));
        }
        void setAction(idx qid, idx action)
        {
            sStr sql;
            db->execute(sql.printf("update %s set action = %" DEC " where qid = %" DEC "",qReq, action, qid));
        }

        void setData(idx qid, const char * name, const void * data, idx datasize=0)
        {
            if(!data)return ;
            
            sStr sql;
            db->execute(sql.printf("delete from %s where qid = %" DEC " and name = '%s'",qDat, qid, name) );
            
            if(datasize==0)datasize=sLen(data)+1;
            sStr enc;sString::encodeBase64(&enc,(const char *)data,datasize,true);enc.add0();

            db->execute(sql.printf("insert into %s (qid, name, data ) values (%" DEC ", '%s', '%s') ",qDat, qid,name, enc.ptr()));
        }

        void * getData(idx qid, const char * name, sStr * buf)
        {
            sStr sql;                                       
            sStr enc;
            if( db->getBlob( &enc,sql.printf(0,"select data from %s where qid=%" DEC " and name = '%s'",qDat, qid, name) ) ) {
                sString::decodeBase64( buf, enc.ptr(), enc.length());
                return buf->ptr();
            }
            return buf->add0();
        }

        idx grab(const char * svc, idx pid=0)
        {
            sStr sql;                         
            sStr enc;
            if(!pid)pid=getpid();
            idx qid=0;db->sscanfTable(sql.printf("select qid from %s where status = %d and action = %d and svc = '%s' order by subdate asc limit 1 for update",qReq, eWaiting, eRun , svc ),"%" DEC "",&qid);
            if(!qid)return 0;
            db->execute(sql.printf(0, "update %s set pid = %" DEC " , status = %d where qid = %" DEC "",qReq,pid, eProcessing, qid));
            return qid;
        }
        
    };
    
};

#endif 
















