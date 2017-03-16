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

function vjObjType_uhiveseq(viewer)
{
    var d_geometry={
            positionalCount:{X:300,Y:300},
            positionalQuality:{X:300,Y:300},
            ATGCCount:{X:300,Y:300},
            ATGCQuality:{X:300,Y:300},
            lengthwiseCount:{X:300,Y:300},
            lengthwiseQuality:{X:300,Y:300}
    };
    this.typeName="u-hiveseq";
    this.idNulldsName="default";
    this.dsTitle="constructing preview of selected object";
    this.tabIcon=["dna","graph","graph","graph"];
    this.viewers=new Object();

    this.viewersToAdd=new Array();
    this.tabName=['sequences','histograms','ACGT','PositionalQC'];
    this.colorsACGT=['#006AFF','#95BE4D','#556B2F', '#000080'];
    this.active_url=["http://?cmd=seqList&out=num|id|len&ids=0",
                     "http://?cmd=seqQC&split=query&query=0&qc=countsAtPositionTable.csv",
                     "http://?cmd=seqQC&split=query&query=0&qc=sumLetterTable.csv",
                     "http://?cmd=seqQC&split=query&query=0&qc=sumPositionTable.csv"];
    this.inactive_url=["static://preview:Select a sequence file to show details",
                       "static://",
                       "static://",
                       "static://"];

    if(this.geometry===undefined){this.geometry=d_geometry;}
    if(this.geometry.positionalCount==undefined){this.geometry.positionalCount=d_geometry.positionalCount;}
    if(this.geometry.positionalQuality==undefined){this.geometry.positionalQuality=d_geometry.positionalQuality;}
    if(this.geometry.ATGCCount==undefined){this.geometry.ATGCCount=d_geometry.ATGCCount;}
    if(this.geometry.ATGCQuality==undefined){this.geometry.ATGCQuality=d_geometry.ATGCQuality;}
    if(this.geometry.lengthwiseCount==undefined){this.geometry.lengthwiseCount=d_geometry.lengthwiseCount;}
    if(this.geometry.lengthwiseQuality==undefined){this.geometry.lengthwiseQuality=d_geometry.lengthwiseQuality;}

    this.clone=function (viewer)
    {
        if(!viewer)viewer=this;
        var i=0;
        for (i in viewer ) {
            this[i] = viewer[i];
        }

    };
    this.clone(viewer);


    this.create=function(dvORtab, id, geometry, formName, stickytabs){
        this.load(dvORtab,id, geometry, formName);
        this.construct(stickytabs);
    };

    this.load = function(dvORtab, id, geometry,formName){
        if(typeof(dvORtab)=="string")dvORtab=vjDV[dvORtab];
        if(formName)this.formName=formName;
        this.viewersToAdd=[];
        this.loaded=true;
        this.constructed=false;
        this.current_dvORtab=dvORtab;
        var dsSuffix=this.idNulldsName;
        var url=this.inactive_url;

        if(id>=0){
            dsSuffix = id;
            url = [urlExchangeParameter(this.active_url[0], "ids",id),
                   urlExchangeParameter(this.active_url[1], "query",id),
                   urlExchangeParameter(this.active_url[2], "query",id),
                   urlExchangeParameter(this.active_url[3], "query",id)];
        }

        var dsName=["ds"+"_"+this.current_dvORtab.name+"_"+this.typeName+"_"+dsSuffix+"_0",
                    "ds"+"_"+this.current_dvORtab.name+"_"+this.typeName+"_"+dsSuffix+"_1",
                    "ds"+"_"+this.current_dvORtab.name+"_"+this.typeName+"_"+dsSuffix+"_2",
                    "ds"+"_"+this.current_dvORtab.name+"_"+this.typeName+"_"+dsSuffix+"_3"];


        for(var iN=0 ; iN < dsName.length ; ++iN){
            var ds=vjDS[dsName[iN]];

            if (!ds) {
                ds = vjDS.add(this.dsTitle, dsName[iN], url[iN]);
            }
        }


        this.length=0;
        var viewersContent = new vjHiveseqControl({
            data:dsName[0],
            formName:this.formName
        });
        this.viewers['content_panel'] = viewerContent[0];

        this.viewers[0]=this.viewers['content_panel'];
        this.viewersToAdd.push(this.viewers[0]);
        ++this.length;

        this.viewers['content']=viewersContent[1];
//        new vjHiveseqView( {
//            data: dsName[0],
//            formObject: document.forms[this.formName]
//        });
        ++this.length;
        this.viewers[1]=this.viewers['content'];
        this.viewersToAdd.push(this.viewers[1]);

        this.viewers['positionalCount'] = new vjHiveseqPositionalCountView ({
            data: dsName[1]
        });
        this.viewers[2]=this.viewers['positionalCount'];
        this.viewersToAdd.push(this.viewers[2]);
        ++this.length;

        this.viewers['positionalQuality'] = new vjHiveseqPositionalQualityView ({
            data: dsName[1]
        });
        this.viewers[3]=this.viewers['positionalQuality'];
        this.viewersToAdd.push(this.viewers[3]);
        ++this.length;

        this.viewers['ATGCCount']= new vjHiveseqACGTCountView ({
            data: dsName[2]
        });
        this.viewers[4]=this.viewers['ATGCCount'];
        this.viewersToAdd.push(this.viewers[4]);
        ++this.length;

        this.viewers['ATGCQuality'] = new vjHiveseqACGTQualityView ({
            data: dsName[2]
        });
        this.viewers[5]=this.viewers['ATGCQuality'];
        this.viewersToAdd.push(this.viewers[5]);
        ++this.length;

        this.viewers['lengthwiseCount']= new vjHiveseqLengthwiseCountView ({
            data: dsName[3]
        });
        this.viewers[6]=this.viewers['lengthwiseCount'];
        this.viewersToAdd.push(this.viewers[6]);
        ++this.length;

        this.viewers['lengthwiseQuality'] =new vjHiveseqLengthwiseQualityView ({
            data: dsName[3]
        });
        this.viewers[7]=this.viewers['lengthwiseQuality'];
        this.viewersToAdd.push(this.viewers[7]);
        ++this.length;

    };

    this.construct=function(stickytabs){
        if(!this.loaded || !this.current_dvORtab)return;
        this.constructed=true;
        if(this.current_dvORtab.tabs){
            var rmtabs=new Array();
            if(stickytabs){
                stickytabs=verarr(stickytabs);
                for(var it=0 ; it<this.current_dvORtab.tabs.length ; ++it){
                    var cTab=this.current_dvORtab.tabs[it];
                    var found=false;
                    for(var is=0; is< stickytabs.length ; ++is){
                        var sTab=stickytabs[is];
                        if(cTab.name==sTab){
                            found=true;
                            break;
                        }
                    }
                    if(!found){
                        rmtabs.push(cTab.name);
                    }
                }
            }

            rmtabs.push(this.tabNam);
            var t_viewersArr=[];
            this.current_dvORtab.remove(rmtabs);
            for(var iN=0 , ii=0; iN < this.tabName.length ; iN+=1){

                var tab=this.current_dvORtab.addTab(this.tabName[iN],this.tabIcon[iN],[this.viewersToAdd[ii++],this.viewersToAdd[ii++]]);
                t_viewersArr=tab.viewers;
                if(iN)tab.columns=2;
            }
            this.current_dvORtab.selected=stickytabs?stickytabs.length:0;
            this.viewersToAdd=t_viewersArr;
        }
        else{
            this.current_dvORtab.remove();
            this.viewersToAdd=this.current_dvORtab.add(this.viewersToAdd).viewers;
        }
        this.current_dvORtab.render();
        this.current_dvORtab.load('rerender');
    };
}
