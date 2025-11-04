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

function TextPreview(props) {
    let data = props.data || null
    if (!data || typeof data !== 'string') return;

    let filename = props.filename && typeof props.filename === 'string' && props.filename !== '' ? props.filename : 'hive_download.html'

    let file = new File([data], filename, {
        type: "text/plain",
    });

    var fileURL = URL.createObjectURL(file)

    let  script = ` 
        <style type="text/css">
            .hv-alert-error{
                color: #000!important;
                background-color: #ffc107!important;
                font-family: Verdana,sans-serif;
                position:relative;
                padding: 10px;
            }
            .hv-alert-error__close{
                cursor:pointer;
                position: absolute;
                right: 10px;
                top: 10px;
                font-size: 18px
            }
        </style>
        <script type="text/javascript">
            let founderror = false
            window.addEventListener('error', function(event){
                if (founderror) return;
                founderror = true

                let errorBar = document.createElement("div");
                errorBar.classList.add('hv-alert-error')

                let closeIcon = document.createElement("span")
                closeIcon.classList.add('hv-alert-error__close')
                closeIcon.innerHTML = "&times;"
                closeIcon.onclick = function(e){
                    e.target.parentElement.style.display='none'
                }
                errorBar.appendChild(closeIcon)

                let msg = document.createElement("p") 

                let subject = escape( "HTML Preview Error" );
                let body = escape( "Error Message: " + event.message) + '%0D%0A'+ escape("File name: ${filename}")
                let subject_body = "?subject=" + subject + "&body=" + body

                msg.innerHTML = "Some <b>error</b> occurred. Not all content might be visible. If content is missing try <b>downloading</b> the <a download='${filename}' href='${fileURL}'>html file</a>. For any questions please email <a href='mailto:HIVE@fda.hhs.gov" + subject_body + "'>HIVE@fda.hhs.gov</a>."
                errorBar.appendChild(msg)

                document.body.prepend(errorBar)
            });

            window.onunload = function(event) { 
                URL.revokeObjectURL('${fileURL}')
            }
        
        </script>
    ` 
    this.init = function() {
        data =  script + '<pre>' + data + '</pre>';
           
        let winUrl = URL.createObjectURL(
            new Blob([data], { type: 'text/html' })
        );
        window.open(
            winUrl,
            filename,
            `
            toolbar=0,
            location=0,
            titlebar=0,
            width=1000,
            height=1000,
            left=2000,
            noreferrer=true,
            noopener=true
            `
        );
        URL.revokeObjectURL(winUrl)
    }
}
