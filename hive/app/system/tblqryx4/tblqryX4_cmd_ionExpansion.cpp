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

#include <slib/utils.hpp>
#include "tblqryX4_cmd.hpp"
#include "utils.hpp"

#include <slib/utils/json/parser.hpp>
#include <qpsvc/qpsvc-cgi.hpp>

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class ionExpansion : public Command
        {
            private:
                idx primary_col;
                sStr additional_colname;

                sHiveId ionObj;

                idx seqID_col,pos_start_col,pos_end_col;
                idx flanking;
                const char * _par_name;
                const char * _seqid1_tmplt, * _seqid2_tmplt;
                const char * _start_tmplt, * _end_tmplt;

                sStr _buf, _bufCSV;


            public:
                struct Map_struct {
                        bool mapOnPos;
                        sStr map_key_hdr;
                        sStr seqID;
                        idx pos_start_val, pos_end_val;
                        Map_struct() {
                            mapOnPos = false;
                            map_key_hdr.cut(0);
                            seqID.cut(0);
                            pos_start_val = pos_end_val =-1;
                        }
                };

                ionExpansion(ExecContext & ctx) : Command(ctx)
                {
                    primary_col=-1;
                    flanking=0;
                    errb.cut(0); _buf.cut(0); _bufCSV.cut(0);
                    seqID_col = pos_start_col = pos_end_col = -1;
                    _par_name="$TOBEREPLACED";
                    _seqid1_tmplt = "$SEQID1"; _seqid2_tmplt = "$SEQID2";
                    _start_tmplt = "$START"; _end_tmplt = "$END";
                }
                Map_struct map_obj;
                sIO errb;
                const char * getName() { return "expansion"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
                void composeMyQuery(sStr & query, Map_struct & map_o, sStr & colnames00, const char * type);
        };
        Command * cmdIonExpansionFactory(ExecContext & ctx) { return new ionExpansion(ctx); }
    };
};

bool ionExpansion::init(const char * op_name, sVariant * arg)
{
    _ctx.qproc().logOut(sQPrideBase::eQPLogType_Debug, "################## Calling ION EXPANSION plugin ########################### \n");
    ::printf("################## Calling ION EXPANSION plugin  ########################### \n");

    if (sVariant * colVal = arg->getDicElt("primary_key"))
    {
        if (colVal->isList())
            primary_col = colVal->getListElt(0)->asInt();
        else
            primary_col = colVal->asInt();
    }
    if (arg->getDicElt("foreign_key") != 0){
        map_obj.map_key_hdr.printf (0,"%s", arg->getDicElt("foreign_key")->asString());
    }
    if (arg->getDicElt("ionOutput") != 0){
        additional_colname.printf (0,"%s", arg->getDicElt("ionOutput")->asString());
        additional_colname.add0(2);
    }
    if (arg->getDicElt("ionObj") != 0){
         arg->getDicElt("ionObj")->asHiveId(&ionObj);
    }

    if (arg->getDicElt("mapOnPos") != 0){
         map_obj.mapOnPos = arg->getDicElt("mapOnPos")->asBool();
    }
    else {
        map_obj.mapOnPos=false;
    }

    if (sVariant * colVal =arg->getDicElt("seqID")){
        if (colVal->isList())
            seqID_col = colVal->getListElt(0)->asInt();
        else
            seqID_col = colVal->asInt();
    }
    if (sVariant * colVal =arg->getDicElt("pos_start")){
        if (colVal->isList())
            pos_start_col = colVal->getListElt(0)->asInt();
        else
            pos_start_col = colVal->asInt();
    }
    if (sVariant * colVal =arg->getDicElt("pos_end")){
        if (colVal->isList())
            pos_end_col = colVal->getListElt(0)->asInt();
        else
            pos_end_col = colVal->asInt();
    }
    if (sVariant * colVal =arg->getDicElt("flanking")){
        if (colVal->isList())
            flanking = colVal->getListElt(0)->asInt();
        else
            flanking = colVal->asInt();
    }



    return true;
}

void ionExpansion::composeMyQuery(sStr & query, Map_struct & map_o, sStr & colnames00,const char * type="u-ionAnnot") {
    idx st_num=1; _buf.cut(0);
    if (strcmp(type,"u-ionTable")==0) {
        query.printf(0,"a=find.row(name=\"%s\",value=\"%s\");unique.1(a.#R);", map_o.map_key_hdr.ptr(), _par_name);

        for (const char * colname=colnames00.ptr(); colname ; colname=sString::next00(colname), ++st_num) {
            if (st_num>1) {
                _buf.printf(",");
            }
            query.printf("s_%" DEC "=find.row(tbl=a.tbl,#R=a.#R,name=\"%s\");unique.1(s_%" DEC ".value);", st_num,colname,st_num);
            _buf.printf("s_%" DEC ".value",st_num);
        }
    } else {
        if (map_o.mapOnPos) {
            query.printf(0,"a=find.annot(#range=possort-max,%s,%s,%s,%s);unique.1(a.record);",_seqid1_tmplt, _start_tmplt, _seqid2_tmplt, _end_tmplt);
        }
        else {
            query.printf(0,"a=find.annot(id=\"%s\",type=\"%s\");unique.1(a.record);",_par_name, map_o.map_key_hdr.ptr());
        }

        for (const char * colname=colnames00.ptr(); colname ; colname=sString::next00(colname), ++st_num) {
            if (st_num>1) {
                _buf.printf(",");
            }
            query.printf("s_%" DEC "=find.annot(seqID=a.seqID,record=a.record,type=\"%s\");", st_num,colname);
            _buf.printf("s_%" DEC ".id",st_num);
        }
    }
    query.printf("print(%s)",_buf.ptr());

}

const char * myObjType(sUsr * user,sHiveId & hiveObj) {
    sUsrObj obj(*user,hiveObj);
    return obj.getTypeName();
}

bool ionExpansion::compute(sTabular * tbl)
{


    ::printf("#################### COMPUTING ionExpansion Plugin########\n");


    bool isEdbcgi = false;
    if (isEdbcgi) {
        QPSvcCgi edb_cgi(_ctx.qproc(), "cmd=getIONHeader&ids=11330");

        edb_cgi.setSessionID(_ctx.qproc().pForm);
        edb_cgi.setVar("cgi_svc","edbCGI");
        idx waiting_for_req = edb_cgi.launch(*_ctx.qproc().user, _ctx.qproc().reqId);
        ::printf("#################### COMPUTING ionExpansion REQID : %" DEC "########\n",waiting_for_req);
        sVariant dataInfo; sStr buf;
        dataInfo.setDic();
        dataInfo.setElt("dataID", waiting_for_req);
        dataInfo.setElt("tbl", "cgi_output");
        buf.cut(0);
        dataInfo.print(buf, sVariant::eJSON);
        const char * dataInfoName = "data-info.json";
        _ctx.qproc().reqSetData(_ctx.qproc().reqId, dataInfoName, buf.mex());
    } else {
            sStr colnames00;
            sString::searchAndReplaceSymbols(&colnames00,additional_colname.ptr(),additional_colname.length(),",",0,0,true,true,true,true);
            colnames00.add0(2);

            sTxtTbl * toReturn = new sTxtTbl();
            toReturn->initWritable(tbl->cols()+sString::cnt00(colnames00),sTblIndex::fTopHeader,",");


            sStr toReplace;
            sStr query;

            sHiveIonBase * hi = sHiveIonBase::make_HiveIon_by_object(_ctx.qproc().user, ionObj);
            if (!hi->ionCnt) {
                return false;
            }

            composeMyQuery(query,map_obj,colnames00,myObjType(_ctx.qproc().user,ionObj));

            const char *a = 0; const char * fsep="@";
            sIonWander * iw =hi->addIonWander("query",query);
            iw->traverseRecordSeparator = (const char *) &a;
            iw->traverseFieldSeparator = fsep;

            iw->traverse();
            sStr seqID, pos_start_val, pos_end_val;
            sStr prev;
            for (idx rr = -1; rr < tbl->rows(); rr++)
            {
                for (idx ic=0; ic < tbl->cols(); ++ic) {
                    sVariant tmp;
                    tbl->val(tmp, rr, ic, true);
                    toReturn->addCell(tmp);
                    if (rr >=0) {
                        if (map_obj.mapOnPos) {
                            if (ic == seqID_col){
                                seqID.printf(0,"%s",tmp.asString());
                            }
                            if (ic == pos_start_col){
                                pos_start_val.printf(0,"%" DEC "",( tmp.asInt() - flanking ) < 0 ? 0 : ( tmp.asInt() - flanking ));
                            }
                            if (ic == pos_end_col){
                                pos_end_val.printf(0,"%" DEC "",tmp.asInt() + flanking);
                            }
                        } else{
                            if ( ic == primary_col) {
                                toReplace.printf(0,tmp.asString());
                                if (toReplace.length()) {
                                    _buf.printf(0,"%s",toReplace.ptr(0));
                                    if (strncmp(prev.ptr(), _buf.ptr(), _buf.length() )==0) {

                                    } else {
                                        iw->setSearchTemplateVariable(_par_name,sLen(_par_name),toReplace.ptr(),toReplace.length());
                                        iw->resetResultBuf();
                                        iw->traverse();
                                        if (iw->traverseBuf.length()) iw->traverseBuf.add0(2);
                                    }
                                    prev.cut(0);
                                    prev.addString(_buf.ptr(),_buf.length());
                                }
                            }
                        }
                    }
                }
                if (rr<0) {
                    for( const char * colname=colnames00.ptr(); colname; colname=sString::next00(colname) ){
                        toReturn->addCell(colname);
                    }
                    toReturn->addEndRow();
                    continue;
                }
                if (map_obj.mapOnPos){
                    _buf.printf(0,"%s_%s_%s",seqID.ptr(),pos_start_val.ptr(),pos_end_val.ptr());
                    if (strncmp(prev.ptr(), _buf.ptr(), _buf.length() )==0) {
                    } else {
                        iw->setSearchTemplateVariable(_seqid1_tmplt, 0,seqID.ptr(),seqID.length());

                        iw->setSearchTemplateVariable(_seqid2_tmplt, 0,seqID.ptr(),seqID.length());

                        iw->setSearchTemplateVariable(_start_tmplt,0,pos_start_val.ptr(),pos_start_val.length());
                        iw->setSearchTemplateVariable(_end_tmplt,0,pos_end_val.ptr(),pos_end_val.length());

                        iw->resetResultBuf();
                        iw->traverse();
                        if (iw->traverseBuf.length()) iw->traverseBuf.add0(2);

                    }
                    prev.cut(0);
                    prev.addString(_buf.ptr(),_buf.length());


                }
                if (iw->traverseBuf.length()){
                    idx iir =0;

                    for (const char * cur_el = iw->traverseBuf.ptr(); cur_el; cur_el=sString::next00(cur_el)) {
                        if (iir==0) {
                            _buf.cut(0);
                            sString::searchAndReplaceSymbols(&_buf,cur_el,sLen(cur_el),fsep,0,0,true,true,true,true,1);
                            for (const char * cur_cell = _buf.ptr(); cur_cell; cur_cell = sString::next00(cur_cell)) {
                                _bufCSV.cut(0);
                                sString::escapeForCSV(_bufCSV,cur_cell);
                                toReturn->addCell(_bufCSV.ptr(),_bufCSV.length());

                            }
                            toReturn->addEndRow();
                        } else {
                            for (idx ic=0; ic < tbl->cols(); ++ic) {
                                sVariant tmp;
                                tbl->val(tmp, rr, ic, true);
                                toReturn->addCell(tmp);
                            }
                            _buf.cut(0);
                            sString::searchAndReplaceSymbols(&_buf,cur_el,sLen(cur_el),fsep,0,0,true,true,true,true);
                            for (const char * cur_cell = _buf.ptr(); cur_cell; cur_cell = sString::next00(cur_cell)) {
                                _bufCSV.cut(0);
                                sString::escapeForCSV(_bufCSV,cur_cell);
                                toReturn->addCell(_bufCSV.ptr(),_bufCSV.length());

                            }
                            toReturn->addEndRow();
                        }
                        iir++;
                    }
                } else {
                    for (idx ie=0; ie < sString::cnt00(colnames00); ++ie) {
                        toReturn->addCell("");
                    }
                    toReturn->addEndRow();
                }


            }
            setOutTable(toReturn);
    }

    return true;

}
