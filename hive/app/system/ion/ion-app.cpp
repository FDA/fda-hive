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


#include "ion-tools.hpp"

#include <slib/lang.hpp>
#include <ulib/usr.hpp>
#include <qlib/QPrideClient.hpp>




sLang lang(0,0);






idx __on_vars(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm)
{
    if(sIs(cmd,"-quit")){
        return sNotIdx;
    }else if(sIs(cmd,"-help")){
        const char * which=pForm->value("command");
        for ( idx i=0; iap->cmdExes[i].param!=sNotPtr ; ++i ) {
            if( which && strcmp(which,iap->cmdExes[i].cmd+1)!=0)continue;
            iap->printf("\t%s%s\n", iap->cmdExes[i].cmd,iap->cmdExes[i].descr);
        }
    }
    else if(sIs(cmd,"-verbose")){
        iap->gVerbose=pForm->ivalue("level",0);
    }else if(sIs(cmd,"-ionComment")){
        iap->gComment=iap->buf.length();
        sString::searchAndReplaceStrings(&(iap->buf),pForm->value("comment"),0,"\\n" _ "\\r" _ "\\t" _ "\\s" __,"\n" _ "\r" _ "\t" _ " " __,0,false);
    }else if(sIs(cmd,"-separField")){
        iap->gSeparField=iap->buf.length();
        sString::searchAndReplaceStrings(&(iap->buf),pForm->value("separator",","),0,"\\n" _ "\\r" _ "\\t" _ "\\s" __,"\n" _ "\r" _ "\t" _ " " __,0,false);
    }else if(sIs(cmd,"-separRec")){
        iap->gSeparRec=iap->buf.length();
        sString::searchAndReplaceStrings(&(iap->buf),pForm->value("separator","\n"),0,"\\n" _ "\\r" _ "\\t" _ "\\s" __,"\n" _ "\r" _ "\t" _ " " __,0,false);
    }else if(sIs(cmd,"-separAttr")){
        iap->gSeparAttribs=iap->buf.length();
        sString::searchAndReplaceStrings(&(iap->buf),pForm->value("separator","\n"),0,"\\n" _"\\r" _ "\\t" _ "\\s" __,"\n" _ "\r" _ "\t" _ " " __,0,false);
    }else if(sIs(cmd,"-silent")){
        iap->gSilent=pForm->ivalue("level",0);
    } else if(sIs(cmd,"-version")){
        iap->printf("%s\n",__TIMESTAMP__);
    } else if(sIs(cmd,"-user")){
        udx userId=pForm->uvalue("userID",0);
        if ( userId== (udx)7)
            iap->gNoLimit=1;
        else{
            iap->user=new sUsr(userId);
        }

    } else if(sIs(cmd,"-calc")){
        const char * expression=pForm->value("expression");
        lang.parse(expression, sLen(expression) );
        iap->printf("%s\n",lang.reslt.ptr(0));
    }
    else if(sIs(cmd,"-mode")){
        iap->gModeGeneral=pForm->uvalue("mode",0);
    }
    else if(sIs(cmd,"-start")){
        iap->gStart=pForm->uvalue("start",0);
    }
    else if(sIs(cmd,"-cnt")){
        iap->gCnt=pForm->uvalue("cnt",0);
    } else if(sIs(cmd,"-lazy")){
        iap->gLazy=pForm->uvalue("lazy",0);
    } else if(sIs(cmd,"-useSynonym")){
        iap->gUseSynonym=pForm->uvalue("mode",0);
    }

    return 0;
}


idx __on_vars(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_hc(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_hc_demo(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_ion(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_test(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_json(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);
idx __on_bio(sIonTools * iap, const char * cmd, const char * , const char * ,sVar * pForm);




sCmdLine::exeCommand sIonTools::cmdExes[]={
     {0,0,0,"","List of available commands"},

    {0,0,0,      "\t","\n\nGeneral commands\n"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,           "-quit"        ,"                       // quit "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argAllZeroList,    "-help"        ," command               // help"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-verbose"     ," level                 // define the level of debug outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-silent"      ," level                 // defines if there are outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-lazy"        ," mode                  // defines the current lazy mode for skipping commands"},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argNone,           "-version"     ,"                       // show the version of the program "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-user"        ," userID                // set user id "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-mode"        ," mode                  // set general operation mode "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-useSynonym"  ," mode                  // set general synonym using mode "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-start"       ," start                 // start index for outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-cnt"         ," cnt                   // count of outputs "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-calc"        ," expression            // 'expression to compute' "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-separField"  ," separator             // 'separator for Fields' "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-separRec"    ," separator             // 'separator for Records' "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-ionComment"  ," comment               // 'character to ignore' "},
    {0,(sCmdLine::exeFunType)&__on_vars,sCmdLine::argOneByOne,       "-separAttr"   ," separator             // 'separator for Attributes in complex records' "},


    {0,0,0,      "\t","\n\nIon commands\n"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionDebug"                     ," mode                     // debugging mode toggle "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionStream"                    ," stream                   // streaming mode toggle "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionProtect"                    ," protect                   // set protect field symbol "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-vaxNoQuote"                    ," ignoreQuote                   // protect or not protect records during parsing"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionExpect"                    ," record size              // set expectation on number of records and relations "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionWrite"                     ," ionname                  // create new ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionCreate"                    ," ionname                  // update new ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionCreateLazy"                ," ionname dependencyFile   // update new ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionRead"                      ," ionname1 [ionname2]        // open existing ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionParseVAX"                  ," control_file data_file   // parse VAX data into ionDB"},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -ionParseVAX vax_ion_control_file vax_file\n"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionParseTable"                ,"                          // generate simple table ionDB"},
        {0,0,0,      "\t\t","example: -separField \"\\t\" -separAttr \" ;\" -ionCreate ion-out -ionParseTable header file.csv"},
        {0,0,0,      "\t\t","example: -separField \",\" -ionCreate ion-out -ionParseTable . file.csv"},

    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionMode"                    ," hashmode           // streaming and hashing mode toggle "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,     "-nameInclude"            ," inclusionListForNames          // set filters for parsing annotations "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,     "-nameExclude"            ," exclusionListForNames          // set filters for parsing annotations "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,     "-valueExclude"           ," valueExclusionList             // set exclusion list "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,     "-isNameExact"            ," mode                           // 0|1 is the name search exact or partial "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,     "-isValueExact"           ," mode                           // 1|0 is the value search exact or partial "},

    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionImport"                    ,"                               // export ion"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionExport"                    ," record-type                   // import ion "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionMerge"                     ," source_ion                       // merge two ionDBs"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionOpenTransaction"           ," transaction_number_or_filepath   // set transaction"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionCommitTransaction"         ,"                                  // commit transaction"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionLoadTransactions"          ," transaction_number_or_folder monitoring_period    // load transaction"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionExecuteTransactions"       ," transaction_number_or_folder monitoring_period    // load transaction"},

    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionSort"                      ," relation useRecords sortName vtreeName // sort ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionSearch"                    ," relation hasher output       // search ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argOneByOne,       "-ionFile"                      ," file                         // use these ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionTraverse"                  ," traverse_arguments table     // traverse/search ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionTraverseFile"              ," traverse_filename table      // traverse/search ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionTraversePrecompile"        ," traverse_arguments            // traverse/search precompile ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionTraversePrecompileFile"    ," traverse_filename             // traverse/search precompile ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionAlias"                     ," alias value                   // associate aliases with constant values for parsed ions"},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllSpacedList,  "-ionLevel"                     ," max_level                    // limit ionDB traversal depth"},

    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionInfo"                     ,"                               // print information about ionDB "},
    {0,(sCmdLine::exeFunType)&__on_ion,sCmdLine::argAllZeroList,    "-ionInfoAll"                  ," record-type sortFile          // print all information about ionDB "},

    {0,0,0,      "\t","\n\nTest Commands\n"},
    {0,(sCmdLine::exeFunType)&__on_test,sCmdLine::argAllZeroList,   "-test"                         ,"                // test "},
    {0,(sCmdLine::exeFunType)&__on_test,sCmdLine::argAllZeroList,   "-test1"                         ,"                // test1 "},
    {0,(sCmdLine::exeFunType)&__on_test,sCmdLine::argAllZeroList,   "-test2"                         ,"                // test2 "},

    {0,0,0,      "\t","\n\nHoneyComb commands\n"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcRoot"                   ," rootnode              // set root node "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllSpacedList,   "-hcCreate"                 ," ionbirelname          // create new ionBirel"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcRead"                   ," ionbirelname          // read ion birel file "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcWrite"                  ," ionbirelname          // write ion birel file "},

    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcParse"                  ," filenames             // parse honeycomb table "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcLink"                   ," sub atr val           // create a link in honecombDB "},

    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcOut"                    ," [json|tbl|hdr|row|sub]// output mode "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcSearch"                 ," par1~val&&par2=^val2||^par3$~val3$// search atributes and values "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcFields"                 ," par1&&par2||par3  // output only fields"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcInto"                   ," par1||par2          // search atributes and values "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcStart"                  ," start          // starting element "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcCnt"                    ," cnt                  // number of elements scanned "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcCntList"                ," cnt                  // number of elements scanned in deep layer lists "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcCntArr"                 ," cnt                  // number of elements scanned in deep layer arrays "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argOneByOne,        "-hcDepth"                  ," depth                // depth scanned "},

    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllSpacedList,   "-hcQry"                    ," sub                  // traverse honeycomb starting from sub "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcType"                   ," types startDepth endDepth //  retrieve type attributes"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcType+"                  ," types                 //  show inherited types"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcObj"                    ," types ids             //  retrieve object fields"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcUser"                   ," ids                   //  retrieve users in the group "},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcGroup"                  ," ids                   //  retrieve groups of the user"},
    {0,(sCmdLine::exeFunType)&__on_hc,sCmdLine::argAllZeroList,     "-hcRule"                   ," act party obj infparty infobj //  set up a new rule"},

    {0,0,0,      "\t","\n\nJSON commands\n"},
    {0,(sCmdLine::exeFunType)&__on_json,sCmdLine::argAllSpacedList,  "-jsonFlatten"                   ," schema destination outfile       //  "},

    {0,0,0,      "\t","\n\nDemo commands\n"},
    {0,(sCmdLine::exeFunType)&__on_hc_demo,sCmdLine::argNone,       "-hcDemoRegisterType"           ,"                      // "},


    {0,0,0,      "\t","\n\nBiological data commands\n"},

    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-annotColumnMap"         ," columns                        // set column list "},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-annotSort"              ," sortFile vTreeName             // sort annotations "},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-annotParse"              ," annotFile seqID start end separ    // parse annotations "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -annotParse file.csv"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-gtfParse"                ," gtfFile                      // parse GTF "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -gtfParse file.gtf"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-vcfParseSpecial"         ," vcfFile                      // parse VCF "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -vcfParseSpecial file.vcf"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-vcfParse"                ," vcfFile                      // parse VCF "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -vcfParse file.vcf"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-gffParse"                ," gffFile                      // parse GFF "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -gffParse file.gff"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-snpParse"                ," snpFile                      // parse SNPprofile "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -snpParse SNPprofile.csv"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-gbParse"                ," genbankFile                   // parse Genbank "},
        {0,0,0,      "\t\t","example: -ionCreate ion-out -gbParse file.gb"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-oboParse"               ," obofile                       // pare gene ontology obo files "},

    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-parseConventionalExpression"                     ," ionFile expressionFile experiment // parse Convention Expression data (in vitro)"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-parseExpression"                     ," ionFile expressionFile experiment id sample passage cutSym // parse expression data (in vitro)"},
    {0,(sCmdLine::exeFunType)&__on_bio,sCmdLine::argAllZeroList,     "-parseExpressionOmnibus"              ," ionFile expressionFile experiment  // parse expression data from omnibus "},



    {sNotPtr,0}
};



sIonClient ionClient;
sCmdLine cmdL;
sIonTools vapp;
extern sIon * RION;

#include <slib/std/cgi.hpp>
#include <slib/std/app.hpp>
#include <slib/std/file.hpp>

class sIGI: public sCGI {
    public:

    sIGI(idx argc = 0, const char * * argv = 0, const char * * envp = 0, FILE * readfrom = stdin, bool isCookie = false, bool immediate = true, const char * forcedMethod = 0)
        :sCGI(argc ,argv , envp , readfrom , isCookie , immediate , forcedMethod ){
    }
    virtual idx Cmd(const char * cmd);

};

idx sIGI::Cmd(const char * cmd)
{
    sStr masterpath("./%s",pForm->value("ion_master","ion_master"));

    sDir dir;dir.list(sFlag(sDir::bitFiles)|sFlag(sDir::bitNoExtension)|sFlag(sDir::bitFollowLinks), masterpath.ptr(), "ion.ion", 0, 0);
    pForm->inptr("ion_master",dir.ptr(),dir.length());
    cmd = pForm->value("cmd");

    outHtml();
    idx res=cmd ? ionClient.Cmd(this,cmd,pForm) : 0 ;
    if(!res)res=sCGI::Cmd(cmd);

    return 1;

}


int main(int argc, const char * argv[], const char *envp[])
{

    srand ( time(0) );

    for ( idx i=0; vapp.cmdExes[i].param!=sNotPtr ; ++i )
        vapp.cmdExes[i].param=&vapp;
    sApp::args(argc, argv, envp);

        if(argc > 1) cmdL.init(argc,argv);
        else cmdL.init("-help");



    if(strstr(argv[0],".cgi")) {
        sIGI igi(argc ,argv , envp , stdin, true, true);
        igi.run();
    }
    else {
        sApp a;
        sQPrideClient qapp("config=qapp.cfg" __);
        vapp.qdb=qapp.sql();
        cmdL.exec(vapp.cmdExes,0,  0);
        if(vapp.user)
            delete vapp.user;
    }

    return 0;
}

