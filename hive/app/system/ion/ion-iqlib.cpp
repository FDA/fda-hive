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



struct sIon_QLibrary::iqLibElement sIon_QLibrary::IQLIB[]={
    {"taxtree_name"
        ,"k=foreach(ACCESSION);a=find.accession_taxid(accession=k.1);print(a.accession,\",\");b=find.taxid_parent(taxid==a.taxid);c=find.taxid_name(taxid==b.taxid,tag=\"scientific name\");print(c.name,\"/\");d=find.taxid_parent(taxid==b.parent);jump!.c(b.parent,1);print(\"root\");"
        ,"null",0}
    ,{"taxtree_id"
        ,"k=foreach(ACCESSION);a=find.accession_taxid(accession=k.1);print(a.accession,\",\");b=find.taxid_parent(taxid==a.taxid);c=print(b.taxid,\"/\");d=find.taxid_parent(taxid==b.parent);jump!.c(b.parent,1);print(1);"
        ,"null",0},
    {"taxtree_ischild"
        ,"k=foreach(ACCESSION);a=find.accession_taxid(accession=k.1);print(a.accession,\",\");b=find.taxid_parent(taxid==a.taxid);c=find.taxid_parent(taxid==b.parent);jump!.e(b.parent,TAXON);f=find.taxid_name(taxid==b.parent,tag=\"scientific name\");print(f.name,\",\",f.taxid,\"\n\");e=jump!.c(b.parent,1);"
        ,"null",0},
    {"taxtree_lineageByTaxid"
        ,"o=find.taxid_name(taxid=='$taxid', tag='scientific name');b=find.taxid_parent(taxid==o.taxid);d=count.taxid_parent(parent==b.parent);e=find.taxid_name(taxid==b.parent, tag='scientific name');f=find.taxid_parent(taxid==e.taxid);print(e.name,e.taxid,f.rank,d.#,'//');jump!.b(e.taxid,1);"
        ,"null",0},
    {"taxtree_lineageByAccession"
        "y=foreach('$accession');x=find.accession_taxid(accession=y.1);o=find.taxid_name(taxid==x.taxid, tag='scientific name');k=find.taxid_parent(taxid=o.taxid);print('',o.name,o.taxid,k.rank,k.#,'|');b=find.taxid_parent(taxid==o.taxid);d=count.taxid_parent(parent==b.parent);e=find.taxid_name(taxid==b.parent, tag='scientific name');f=find.taxid_parent(taxid==e.taxid);print(e.name,e.taxid,f.rank,d.#,'|');jump!.b(e.taxid,1);"
        ,"null",0},
    {0,0}
};


sIon_QLibrary::iqLibElement * sIon_QLibrary::find(const char * name )
{
    for(idx i=0; IQLIB[i].name;++i){
        if(strcmp(IQLIB[i].name,name)==0)
            return IQLIB+i;
    }
return 0;
}




