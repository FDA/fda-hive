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

function vjPDFView ( viewer )
{
    vjDataViewViewer.call(this,viewer);
    
    this.pdf = null;
    this.isRendering = false;
    this.pageNumberRendering = null;
    this.pageNumberPending = null;
    this.canvas = null;

    this.queueRenderPage = function(pageNumber) {
        if (this.isRendering) {
            this.pageNumberPending = pageNumber;
        } else {
            this.renderPage(pageNumber);
        }
    }
    
    this.renderPage = function(pageNumber) {
        var oThat = this;
        this.isRendering = true;
        this.pdf.getPage(pageNumber).then(function(page) {
            var scale = 1.5;
            var viewport = page.getViewport(scale);
            var context = oThat.canvas.getContext("2d");

            oThat.canvas.height = viewport.height;
            oThat.canvas.width = viewport.width;

            var renderContext = {
                canvasContext: context,
                viewport: viewport
            };
            var renderTask = page.render(renderContext);
            renderTask.promise.then(function() {
                oThat.isRendering = false;
                if (oThat.pageNumberPending !== null) {
                    oThat.renderPage(oThat.pageNumberPending);
                    oThat.pageNumberPending = null;
                }
            });
        });
        this.pageNumberRendering = pageNumber;
        $("#pdf-page-num").text(this.pageNumberRendering);
    }
    
    this.composerFunction=function(viewer, content)
    {
        var oThat = this;
        this.div.innerHTML = '<div><button id="pdf-page-prev" type="button">Prev</button><button id="pdf-page-next" type="button">Next</button><span>Page <span id="pdf-page-num">1</span>/<span id="pdf-page-cnt">?</span></span></div><canvas id="homepdfpreview"></canvas>';
        this.canvas = document.getElementById("homepdfpreview");
        
        $("#pdf-page-prev").click(function() {
            if (oThat.pdf && oThat.pageNumberRendering > 1) {
                oThat.queueRenderPage(oThat.pageNumberRendering - 1);
            }
        });
        
        $("#pdf-page-next").click(function() {
            if (oThat.pdf && oThat.pageNumberRendering < oThat.pdf.numPages) {
                oThat.queueRenderPage(oThat.pageNumberRendering + 1);
            }
        });
        
        var pdfUrl = vjDS[viewer.data[0]].url.replace("static:
        
        var loadingTask = PDFJS.getDocument(pdfUrl);
        loadingTask.promise.then(function(pdf_) {
            oThat.pdf = pdf_;
            $("#pdf-page-cnt").text(oThat.pdf.numPages);
            oThat.pageNumberRendering = 1;
            oThat.renderPage(oThat.pageNumberRendering);
        }, function (reason) {
            console.error("Loading PDF failed: " + reason);
        });
    };

    this.refresh = function ()
    {
        
    };
}

vjHO.register('u-file').Constructor=function ()
{

    this.fullview=function(node,dv)
    {
        this.mode = 'fullview';
        this.preview(node,dv);
    };

    this.preview = function(node,dv)
    {
        this.mode = 'preview';
        this.loadedID = node.id;
        linkCmd('objQry&raw=1&qry=o%3D(obj)"' + node.id + '";return[o.name,o.ext];', this, function(this_, txt) {
            var exts = [];
            try {
                exts = verarr(JSON.parse(txt));
            } catch (e) { }
            for (var i=0; i<exts.length; i++) {
                if (i == 0) {
                    var exts0 = exts[0].split('.');
                    if (exts0.length > 1) {
                        exts[0] = exts0[1];
                    } else {
                        exts[0] = "";
                    }
                }
            }
            this_.previewExt(node, dv, exts);
        });
    }

    this.previewExt = function(node, dv, exts)
    {
        var formObject = gAncestorByTag(gObject(dv.obj.name), "form");
        var previewViewers = [];

        var ext = "";

        for (var i=0; i<exts.length; i++) {
            if (exts[i].match(/^(.+\.)?(csv|tsv|tab|vcf)$/i)) {
                ext = "csv";
                break;
            } else if (exts[i].match(/^(.+\.)?(txt|xml)$/i)) {
                ext = "txt";
                break;
            }  else if (exts[i].match(/^(.+\.)?(json)$/i)) {
                ext = "json";
                break;
            }  else if (exts[i].match(/^(.+\.)?(pdf)$/i)) {
                ext = "pdf";
                break;
            }
        }

        if (ext == "csv") {
            this.urlSet["table_preview"].ext = exts[1];
            this.addviewer("table_preview_panel", new vjBasicPanelView({
                data: ["dsVoid", "table_preview"],
                formObject: formObject,
                urlExchangeParameter: function(url, arg, val) {
                    if (arg == "start" || arg == "cnt") {
                        url = urlExchangeParameter(url, arg, val);
                        var start = parseInt(this.computeSubmitableValue("start", "0", url));
                        if (!start)
                            start = 0;
                        var cnt = parseInt(this.computeSubmitableValue("cnt", "1000", url));
                        if (!cnt)
                            cnt = 1000;
                        if (start + cnt < 100 && this.computeSubmitableValue("src", "-", url) != "-") {
                            url = urlExchangeParameter(url, "parseCnt", start + cnt + 1)
                        } else {
                            url = urlExchangeParameter(url, "parseCnt", "-");
                        }
                        return url;
                    } else {
                        return urlExchangeParameter(url, arg, val);
                    }
                }
            }));
            this.addviewer("table_preview", new vjTableView({
                data: "table_preview",
                formObject: formObject,
                bgColors: ['#f2f2f2','#ffffff'],
                geometry: { width: "100%"},
                isok: true
            }));
            dv.obj.addTab(this.tabs.names[1], this.tabs.icons[1], [this.viewers["table_preview_panel"], this.viewers["table_preview"]]);
        } else if (ext == "txt" ) {
            
            this.addviewer("text_preview", new vjTextView ({
                data: "text_preview",
                isok: true,
                isJSON: ext == "json",
                argWidth: "95%"
            })); 
            
            dv.obj.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["text_preview"]]);
        } else if (ext == "json") {
            
            this.addviewer("text_preview", new vjHTMLView ({
                data: "text_preview",
                isok: true,
                composerFunctionCallback: function(data){
                    data = parseJSON(data);
            
                    if (typeof data === 'object'){
                        let jsonViewer = new JSONViewer();

                        jsonViewer.showJSON(data, null, 1);
                        data = jsonViewer._dom_container;
                    }
                    
                    return data;
                } ,
                isJSON: ext == "json",
                argWidth: "95%"
            })); 
            
            dv.obj.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["text_preview"]]);
        }else if (ext == "pdf") {
            this.addviewer("pdf_preview", new vjPDFView ({
                data: "pdf_preview"
            }));
            dv.obj.addTab(this.tabs.names[0], this.tabs.icons[0], [this.viewers["pdf_preview"]]);
        } else {
            return;
        }
        dv.obj.render(true);
        dv.obj.load("rerender");
    };

    this.addUrlSet({
        "text_preview": {
            title: "Retrieving file preview",
            active_url: "http:
            objs: "ids"
        },
        "table_preview": {
            title: "Retrieving table preview",
            make_active_url: function(url, loadedID) {
                var ext = this.urlSet["table_preview"].ext;
                if (!ext)
                    ext = "csv";
                return "qpbg_tblqryx4:
            }
        },
        "pdf_preview": {
            title: "Retrieving pdf preview",
            make_active_url: function(url, loadedID) {
                return "static:
            }
        },
    });

    this.tabs = {
        icons: ["rec", "table"],
        names: ["file preview", "table preview"]
    };

    if(this.onObjectContructionCallback) {
        funcLink(this.onObjectContructionCallback, this);
    }
};
