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

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class GeneOntol : public Command
        {
            private:
                idx id_col;

            public:
                GeneOntol(ExecContext & ctx) : Command(ctx)
                {
                    id_col=-1;
                    errb.cut(0);
                }
                sIO errb;
                const char * getName() { return "extract"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return true; }
                bool loadGeneOntologyIon(sVec<sHiveId> & result, sIonWander & myWander, sUsr * user);

                bool init(const char * op_name, sVariant * arg);
                bool compute(sTabular * tbl);
                bool composeMyQuery(sIonWander & wander,sStr & query);
        };
        Command * cmdGeneOntology(ExecContext & ctx) { return new GeneOntol(ctx); }
    };
};

bool GeneOntol::init(const char * op_name, sVariant * arg)
{

    if (sVariant * colVal = arg->getDicElt("selectColumn"))
    {
        if (colVal->isList())
            id_col = colVal->getListElt(0)->asInt();
        else
            id_col = colVal->asInt();

    }

    if (id_col<0) {
        return false;
    }
    return true;
}

bool GeneOntol::loadGeneOntologyIon (sVec<sHiveId> & result, sIonWander & myWander, sUsr * user) {
    result.add(1);
    sviolin::SpecialObj::find(result[0], *user, "GeneOntologyDatabase", "1");
    if (!result[0].objId()) {
        return false;
    }
    return (sHiveIon::loadIonFile(user,result,myWander, "go.ion")) ? true : false;

}


bool GeneOntol::composeMyQuery(sIonWander & wander,sStr & query) {
    query.printf(0,"a=find.rel(atr=gene_product, val=$GENENAME);b=find.rel(val=a.sub);c=find.rel(atr=association,val=b.sub);d=find.rel(sub=c.sub,atr=name);unique.1(d.sub);print(d.val);");
    errb.cut(0);
    wander.traverseCompile(query.ptr(),query.length(),&errb);
    if (errb.length()) {
        return false;
    }
    return true;
}


bool GeneOntol::compute(sTabular * tbl)
{



    sVec<sHiveId> goIonId;

    sIonWander myWander;
    myWander.setSepar(",","|");

    if (!loadGeneOntologyIon(goIonId, myWander, _ctx.qproc().user)){
        return false;
    }



    sTxtTbl * toReturn = new sTxtTbl();

    toReturn->initWritable(tbl->cols()+1,sTblIndex::fTopHeader,",");


    sStr gname;
    sStr query;
    sStr escapedResult;
    if (!composeMyQuery(myWander, query)){
        printf("could not compose query\n");
        return false;
    }
    for (idx rr = -1; rr < tbl->rows(); rr++)
    {
        for (idx ic=0; ic < tbl->cols(); ++ic) {
            sVariant tmp;
            tbl->val(tmp, rr, ic, true);
            toReturn->addCell(tmp);

            if ( ic == id_col) {
                gname.printf(0, "%s", tmp.asString());
            }

        }
        if (rr<0) {
            toReturn->addCell("go_name");
            toReturn->addEndRow();
            continue;
        }
        if (gname.length()) {
            myWander.setSearchTemplateVariable("$GENENAME",9,gname.ptr(),gname.length());
            myWander.resetResultBuf();
            myWander.traverse();
        }
        else {return false;}

        escapedResult.cut(0);
        sString::escapeForCSV(escapedResult,myWander.traverseBuf.ptr(),myWander.traverseBuf.length());
        toReturn->addCell(escapedResult.ptr(),escapedResult.length());
        toReturn->addEndRow();
    }
    setOutTable(toReturn);
    return true;

}
