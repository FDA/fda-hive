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

#include <qlib/QPrideProc.hpp>
#include <slib/std/file.hpp>
#include <ion/sJson.hpp>
#include <violin/violin.hpp>


class DNA_otviProc: public sQPrideProc
{
    public:
        DNA_otviProc(const char * defline00, const char * srv)
            : sQPrideProc(defline00, srv)
        {

        }
        ~DNA_otviProc()
        {
        }
        virtual idx OnExecute(idx);
};

struct IMP {
    idx coverage; real freqInDel, freqSNV;
};

idx DNA_otviProc::OnExecute(idx req)
{

    sStrT ref_prefix;formValue("ref-prefix", &ref_prefix,"chr;spike;");
    sString::searchAndReplaceSymbols(ref_prefix.ptr(0), 0, ";", 0, 0, true, true, false, true, false);

    idx len, iSampleNum;

    sVec<sBioal::Stat> stat;
    sDic < sHiveseq > SubsList;
    sDic < idx >  nominatedHitList,gBlockHitList;
    sDic < idx > samples, references, gblocks;
    sDic < IMP > imperfects;
    sVec < idx > sampleHits(sMex::fSetZero);

    sVec<sHiveId> hept_ids;
    formHiveIdValues("hept-nominated", &hept_ids);
    formHiveIdValues("hept-gblock", &hept_ids);

    sStrT buf,hexid,subid, nam;
    enum eMode { eNominated=0,eGBlock};
    idx curMode;


    for (idx in=0; in<hept_ids.dim(); ++in) {
        nam.cut(0);
        buf.cut(0);

        sUsrObj hept(*user,hept_ids[in]);
        hept.propGet("name",&nam);

        char * p=strstr(nam,".fastq"); {if ( !p)continue;*p=0;++p;}
        if( strstr(p," vs gBlocks") ) curMode=eGBlock;
        else if( strstr(p," vs Nominated") ) curMode=eNominated;
        else continue;

        (*samples.set(nam.ptr(),0,&iSampleNum))++;
        sampleHits.resize(samples.dim());

        hexid.cut(0);
        sUsrObj hex(*user,sHiveId(hept.propGet("parent_proc_ids",&hexid)));
        sHiveal hiveal(user, hexid);
        subid.cut(0);hex.propGet00("subject",&subid,";");
        sHiveseq * Sub=SubsList.set(subid.ptr());hiveal.Sub=(sBioseq * )Sub;
        if(!Sub->dim()){
            Sub->parse(subid.ptr(), sBioseq::eBioModeShort, false, user);
            Sub->reindex();
        }

        stat.cut(0);
        hiveal.countAlignmentSummaryBySubject(stat);
        idx cntReferences=0;
        for ( idx iSub=0; iSub<hiveal.Sub->dim(); ++iSub) {

            if(curMode==eNominated) {
                sampleHits[iSampleNum]+=stat[iSub+1].foundRpt;
            }

            const char * sid=(const char * )hiveal.Sub->id(iSub);if(!sid) continue;
            if(ref_prefix.length()){for ( p=ref_prefix.ptr(); p; p=sString::next00(p)){
                if( memcmp(sid,p,sLen(p))==0)break;
            }if(!p) continue;}

            cntReferences++;

            buf.printf(0,"%s@%s",nam.ptr(),sid);
            if(curMode==eNominated) {
                (*references.set(sid))++;
                *nominatedHitList.set(buf.ptr())=stat[iSub+1].foundRpt;
            }else if (curMode==eGBlock) {
                *gBlockHitList.set(buf.ptr())=stat[iSub+1].foundRpt;
                (*gblocks.set(sid))++;
            }
        }
        if(curMode==eNominated && cntReferences)
            sampleHits[iSampleNum]/=cntReferences;

        if(curMode==eNominated) {
            sTbl tbl;
            buf.cut(0);hept.getFilePathname(buf, "imperfect.csv");
            sFil imp(buf,sMex::fReadonly);
            if(imp.length())tbl.parse(imp.ptr(),imp.length());
            for( idx ir=1; ir<tbl.rows(); ++ir) {
                const char * reference=tbl.cell(ir,(idx)0,&len);if (!reference)continue;
                buf.printf(0,"%s@%.*s",nam.ptr(),(int)len, reference);
                IMP * imp=imperfects.set(buf.ptr());
                imp->coverage=tbl.ivalue(ir, 10, 0);
                imp->freqInDel=imp->coverage ? tbl.rvalue(ir, 8, 0)/imp->coverage : 0;
                imp->freqSNV=imp->coverage ? tbl.rvalue(ir, 7, 0)/imp->coverage : 0;
            }
        }

    }


    sVec < idx> indSamples,indReferences,indGBlocks;
    indSamples.resize(samples.dim());indReferences.resize(references.dim()),indGBlocks.resize(gblocks.dim());
    sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sSort::sort_stringsDicID,&samples,samples.dim(), samples.ptr(),indSamples.ptr());
    sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sSort::sort_stringsDicID,&references,references.dim(), references.ptr(),indReferences.ptr());
    sSort::sortSimpleCallback((sSort::sCallbackSorterSimple)sSort::sort_stringsDicID,&gblocks,gblocks.dim(), gblocks.ptr(),indGBlocks.ptr());


    sStrT dstPath;

    enum ePrintWhat { eCoverage=0,eFreqInDel, eFreqSNV, eGBlockHits, eGBlockFreq, eLast};
    const char * fileNames[]={"imperfect-coverage","imperfect-InDel", "imperfect-SNV", "gBlock-hits", "gBlock-frequencies"};
    for( idx printWhat=eCoverage; printWhat<eLast; ++printWhat ) {

        sDic < idx > * curRefs;
        sVec < idx > * curSort;
        if( printWhat <=eFreqSNV ) { curMode=eNominated; curRefs=&references;curSort=&indReferences;}
        else { curMode=eGBlock; curRefs=&gblocks;curSort=&indGBlocks;}

        dstPath.cut(0);reqAddFile(dstPath,"%s.csv",fileNames[printWhat]);
        sFile::remove(dstPath);
        sFil Fl(dstPath.ptr()), *out=&Fl;

        out->printf("references/samples");
        for( idx is=0; is<samples.dim() ; ++ is) {
            out->printf(",%s",(const char * )samples.id(indSamples[is]));
        }out->addString("\n",1);


        for( idx ir=0; ir<curRefs->dim() ; ++ ir) {

            const char * reference=(const char * )curRefs->id((*curSort)[ir]);

            out->printf("%s",reference);
            for( idx is=0; is<samples.dim() ; ++ is) {
                const char * sample=(const char * )samples.id(indSamples[is]);
                buf.printf(0,"%s@%s",sample,reference);
                out->addString(",",1);

                if( curMode==eNominated ) {
                    IMP * imp=imperfects.get(buf.ptr()); if(!imp)continue;
                    if(printWhat==eCoverage)out->printf("%" DEC, imp->coverage);
                    else if(printWhat==eFreqInDel)out->printf("%lg",imp->freqInDel );
                    else if(printWhat==eFreqSNV)out->printf("%lg",imp->freqSNV );
                }
                else if( curMode==eGBlock ) {
                    idx * pHits=gBlockHitList.get(buf.ptr());
                    idx hits=pHits ? *pHits : 0;
                    if (printWhat==eGBlockFreq){
                        if(sampleHits[indSamples[is]])out->printf("%lg",((real)hits)/sampleHits[indSamples[is]]);
                        else out->addString("0",1);
                    } else if (printWhat==eGBlockHits){
                        out->printf("%" DEC ,hits);
                    }
                }
            }
            out->addString("\n",1);
        }
    }


    reqProgress(100,100, 100);

    reqSetStatus(req, eQPReqStatus_Done );

    return 0;
}

int main(int argc, const char * argv[])
{
    sStr tmp;
    sApp::args(argc, argv);
    DNA_otviProc backend("config=qapp.cfg" __, sQPrideProc::QPrideSrvName(&tmp, "dna-otvi", argv[0]));
    return (int) backend.run(argc, argv);
}
