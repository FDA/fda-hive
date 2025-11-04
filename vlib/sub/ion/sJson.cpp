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

#include <sys/stat.h>
#include <ion/vax.hpp>
#include <ion/sJson.hpp>


using namespace slib;



void sJson::CSVFlattener::streamJsonIn(JSNode * parentNode, const char * startPath, const char * prefix)
{
    if(prefix) {
        ofsPrefix=Prefix.length();
        Prefix.printf("%s",prefix);Prefix.add0(1);
    }
    else ofsPrefix=-1;

    Path.printf(0,"$root.%" DEC , iBlock);

    VAL * el=precomputeGeometry(parentNode);
    maxCols=varDic.dim();
    maxRows+=el->rows;
    ++iBlock;

}

sJson::CSVFlattener::VAL * sJson::CSVFlattener::precomputeGeometry(JSNode * parentNode)
{
    idx pos=Path.length();
    idx aLen=0; const char * atr=parentNode->atr(&aLen);
    idx vLen=0; const char * val=parentNode->val(&vLen);
    if(atr){
        Path.printf(pos,".%.*s",(int)aLen,atr);
    }
    idx inum;
    VAL *el=valDic.set(Path.ptr(0),Path.length(),&inum);
    VAL * parentEl=valDic.get(Path.ptr(0),pos);
    if(parentEl) {
        el->startRow=parentEl->rows+parentEl->startRow;
    }else el->startRow=maxRows;

    const char * cln=sJson::cleanIndexesFromPath(Path.ptr());
    if( parentNode->vType==JSNode::eVal ){
        el->val=val;
        el->rows=1; el->columns=1;

        sStrT buf; buf.printf(0,"0xFF%s-%" DEC,cln,el->startRow);
        CLN * clnV=clnDic.set(buf.ptr(),buf.length());
        clnV->val=val;clnV->vLen=vLen;
        clnV->ofsPrefix=ofsPrefix;

        (*varDic.set(cln,sLen(cln)))++;
    }
    else {

        idx columns=el->columns, rows=el->rows;
        for (JSNode cur_node=parentNode->child();cur_node.ok(); cur_node=parentNode->next()) {

            VAL * child=precomputeGeometry(&cur_node);
            el=valDic.ptr(inum);

            if(parentNode->vType==JSNode::eArr){
                columns=sMax(columns,child->columns);
                rows+=child->rows;
                el->rows=rows;
            } else {
                rows=sMax(rows,child->rows);
                columns+=child->columns;
            }

        }
        el->columns=columns; el->rows=rows;
    }


    Path.cut(pos);


    return el;
}


const char * sJson::CSVFlattener::printCSV(sStr * out, const char * removePrefix )
{


    if(maxCols>varDic.dim())maxCols=varDic.dim();

    if(ofsPrefix>=0)
        out->printf(",");

    for ( idx icol=0; icol<maxCols; ++icol) {
        idx idLen;const char * id = (const char*)varDic.id(icol,&idLen);
        if(icol>0)out->printf(",");
        const char * p=id+6;idx plen=idLen-6;
        if(removePrefix){
            const char * s=sString::searchSubstring(p, plen, removePrefix, sNotIdx, 0, false, 0);
            if(s){plen-=(s-p)+sLen(removePrefix);p=s+sLen(removePrefix);}
        }
        out->printf("%.*s",(int)plen,p);
    }
    out->printf("\n");


    sStrT buf;
    for ( idx irow=0; irow<maxRows ; ++irow) {
        idx cntOut=0;
        for ( idx icol=0; icol<varDic.dim() ; ++icol) {
            idx idLen;const char * id = (const char*)varDic.id(icol,&idLen);


            buf.printf(0,"0xFF%.*s-%" DEC , (int)idLen, id, irow);

            CLN * clnV=clnDic.get(buf.ptr(),buf.length());


            if(icol==0){
                if(clnV && clnV->ofsPrefix>=0)
                    out->printf("%s,",Prefix.ptr(clnV->ofsPrefix+1));
            }

            if(icol)
                out->printf(",");

            if(!clnV || !clnV->val)continue;

            out->printf("\"");
            sString::searchAndReplaceStrings(out, clnV->val, clnV->vLen, "\"" __ , "\"\"" __ ,0 , 0 );
            out->shrink00(0);
            out->printf("\"");
            ++cntOut;
        }
        if(cntOut)
            out->printf("\n");
        if(!cntOut)
            break;
    }


    return  out->ptr(0);
}





JSNode * sJson::pathConstruct(const char * path, bool isreadonly, JSNode * prvnode)
{
    sStr Path;
    if(!prvnode){
        sString::searchAndReplaceSymbols(&Path,path,0,".",0,0,true,true,true,true,0);
        path=Path.ptr();
    }

    JSNode & node=prvnode ? *prvnode : this->node(path);
    for(const char * p=sString::next00(path); p; p=sString::next00(p)){
        idx type=0;
        if(p[0]=='{' && p[1]=='}') {p+=2;type=1;}
        else if(p[0]=='[' && p[1]==']') {p+=2;type=2;}

        const char * val = (p[0]=='=') ? p+1 : 0;

        if(val) {
            idx lv;const char * v=node.val(&lv);
            if(lv==sLen(val) && strncmp(val,v,lv)==0 ) return &node;
            return 0;
        }


        if(*p=='#' || *p=='@') {
            JSNode * found=0;
            for ( idx i=0; i<node.dim(); ++i) {
                found=pathConstruct(p, true, &(node[i]));
                if(found)return found;
            }
            if(type==1)node=node.linkobj(p);
            else if(type==2)node=node.linkarr(p);
            continue;
        }

        JSNode & nextnode=node[p];
        if(!nextnode.ok()) {
            if(isreadonly)return 0;
            if(type==1)nextnode=node.linkobj(p);
            else if(type==2)nextnode=node.linkarr(p);
            else {
                val = sString::next00(p);
                if(val){
                    if(val[0]=='=')++val;
                    node.link(p,val);
                }
                return &node;
            }
        }


        node=nextnode;

    }
    return &node;
}
