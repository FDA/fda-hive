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
loadJS('jsx/widgets/vanilla/table.template.js')
loadJS('jsx/widgets/vanilla/button.template.js')
loadJS('jsx/widgets/jquery/components/Dialog.js')
loadJS('jsx/widgets/jquery/components/ProgressMsgsDialog.js');
loadJS('jsx/widgets/vanilla/actionButton.js')

function ProgressPartsDialog (props) {
    let _this = this

    this.dialogId = `dialog-parts-project`
    this.tableId = `dialog-table-parts-project`
    let dialog_props = {
        dialogID: this.dialogId
    }
    this.dialog = new DialogComponent(dialog_props)

    this.onPartsDataLoaded = function(parsedData){ 
        _this.dialog.updateDialogTitle(`${parsedData.Head.name} : ${parsedData.Head.reqID}`) 
    }

    let table_props = {
        onPartsDataLoaded: this.onPartsDataLoaded,
        parentId: this.dialogId,
        tableId: this.tableId,
        ...props
    }

    this.table = new ProgressPartsTable(table_props)

    this.dialog.updateDialogContent(this.table)

}

let PARTS_TABLE_COUNTER = 0

function ProgressPartsTable (props) {
    let _this = this
    this.componentName = props.componentName || 'progress-parts'
    this.parentId = props.parentId
    this.tableId = props.tableId
    this.params = props.params
    this.onPartsDataLoaded = props.onPartsDataLoaded || null

    this.breadcrumbs = []

    this.allParts = []
    this.parts = this.allParts
    this.currentPage = 1,
    this.pageSize = 10,
    this.maxPages = 7
    this.paginate = paginate(this.parts.length, this.currentPage, this.pageSize, this.maxPages)

    this.dsPartsInfo = `dsPartsInfo_${PARTS_TABLE_COUNTER}`
    PARTS_TABLE_COUNTER ++

    this.renderTable = function( ) {
        _this.paginate = paginate( _this.parts.length, _this.currentPage, _this.pageSize, _this.maxPages)
        let parts =  _this.parts.slice(_this.paginate.startIndex, _this.paginate.endIndex + 1)
        $(`[data-component="${_this.componentName}"]`)
            .find(`[data-pager="info"]`)
            .text(_this.parts.length ? `${_this.paginate.startIndex+1}-${_this.paginate.endIndex+1} of ${_this.parts.length}` : _this.parts.length)
        $(`#${_this.tableId}`)
            .html(createTable(_this.tableHeaderRow,  parts))
        this.hidePagerButtons()
    }

    this.renderBreadcrumbs = function(array) {
        if(array) _this.breadcrumbs = _this.breadcrumbs.concat(array)
        $(`[data-component="breadcrumbs"]`).empty()
        _this.breadcrumbs.forEach(function(crumb , i){
            $(`[data-component="breadcrumbs"]`).append(
                `${i !== 0 ? ' > ' : ''}<span class="hv-btn hv-btn-link ${crumb.selected ? 'hv-btn-link-selected' : ''}" data-crumb="${crumb.component}">${crumb.title}</span> `
            )
        })
    }

    this.hidePagerButtons = function(){
        if(this.paginate.startIndex === 0 || this.parts.length === 0){
            $('[data-event="pager-prev"]').css('display' , 'none')
        }else{
            $('[data-event="pager-prev"]').css('display' , '')
        }
        if(this.paginate.endIndex + 1 === this.paginate.totalItems || this.parts.length === 0){
            $('[data-event="pager-next"]').css('display' , 'none')
        }else{
            $('[data-event="pager-next"]').css('display' , '')
        }
    }

    this.buttons = [
        {
            name: 'refresh',
            event: 'refresh',
            actions: {
                click: function(e){
                    $(`#${_this.tableId}`).html('<img border="0" width="24" src="img/progress.gif"> Loading ...')
                    vjDS[_this.dsPartsInfo].reload(null,true)
                },
            }
        },
        {
            prepend: `<select data-event="filter">
                        <option>All</option>
                        <option><i class="rv-info-round-color" title="Info"></i> Info</option>
                        <option><i class="rv-bug-trace-color" title="Trace"></i> Trace</option>
                        <option><i class="rv-bug-round-color" title="Debug"></i> Debug</option>
                        <option><i class="rv-warning-color" title="Warning"></i> Warning</option>
                        <option><i class="rv-error-color" title="Error"></i> Error</option>
                    </select>`,
            event: 'filter',
            actions: {
                change: function(e){
                    if(e.target.tagName === 'SELECT') {
                        _this.parts = e.target.value === 'All' 
                            ? _this.allParts
                            : filterArrayOfObjects(_this.allParts, 'level' , e.target.value)
                        
                        _this.renderTable()
                    }
                }
            }
        },
    ]

    this.pager_buttons = [
        {
            prepend:`<span data-pager="info">${this.parts.length ? `${this.paginate.startIndex+1}-${this.paginate.endIndex+1} of ${this.parts.length}` : this.parts.length}</span>

                    <select data-event="pager">
                        <option>10</option>
                        <option>50</option>
                        <option>100</option>
                        <option>1000</option>
                    </select>`,
            event: 'pager',
            actions: {
                change: function(e){
                    if(e.target.tagName === 'SELECT') {
                        _this.pageSize  = parseInt(e.target.value)

                        _this.renderTable()
                    }
                    
                }
            },
            alt: 'Select page' 
        },
        {
            name: '<',
            event: 'pager-prev',
            actions: {
                click: function(e){
                    _this.currentPage--
                    _this.renderTable()
                }
            },
            alt: 'Previous page' 
        },
        {
            name: '>',
            event: 'pager-next',
            actions: {
                click: function(e){
                    _this.currentPage++
                    _this.renderTable()
                }
            },
            alt: 'Next page' 
        },
    ]

    this.handleAction = function(e , action) {
        let event = e.target.dataset.event
        if(!event) return
        
        _this.buttons.concat(_this.pager_buttons).forEach(function(button){
            let btn_actions = button.actions ? Object.keys(button.actions) : null; 
            if(event === button.event && btn_actions && btn_actions.includes(action)){
                button.actions[action](e)
            }
        })
    }

    this.actions = [
        {
            name: 'click',
            action: function(e){ _this.handleAction(e , 'click') }
        },
        {
            name:'change',
            action: function(e){ _this.handleAction(e , 'change') }
        }
    ]

    this.handleTableEvents = function(){
        $(`#${_this.tableId}`).off()

        if(_this.actions) {
            _this.actions.forEach(function(action){
                $(`[data-component="${_this.componentName}"]`).off(action.name )
                $(`[data-component="${_this.componentName}"]`).on(action.name , action.action)
            })
        }

        $(`#${_this.tableId}`).on('click', function(e){
            let target = e.target
            let data = target.dataset
            let event = data.event

            if(event === "view-msgs"){
                _this.breadcrumbs.forEach(function(crumb , i){
                    crumb.selected = false
                    $(`[data-component="${crumb.component}"`).hide()
                    if(crumb.component === 'progress-msgs')  _this.breadcrumbs.splice(i,1) 
                })
                _this.renderBreadcrumbs([{
                    title: `${data.reqid} messages`,
                    component: 'progress-msgs',
                    selected:true
                }])

                $('[data-component="progress-msgs"').remove()

                let table_props = {
                    parts: true,
                    componentName: 'progress-msgs',
                    parentId: _this.parentId,
                    tableId: `table-msgs-progress`,
                    params: {
                        objs: data.objid, 
                        req: data.reqid
                    }
                }
                let msgsTable = new ProgressMsgsTable(table_props)
                $(`#${_this.parentId}`)
                    .append(
                        msgsTable
                    )
            }
            if( event === 'progress-action' && data.action && data.action !== ''){
                linkCmd("?cmdr=-qpReqSetAction&req=" + data.reqid + "&act=" + data.action  + "&isGrp=" + (data.reqid == data.grpid));
                $(`#${_this.tableId}`).html('<img border="0" width="24" src="img/progress.gif"> Loading ...')
                vjDS[_this.dsPartsInfo].reload(null,true)
            }

        })
    }

    this.handleBreadcrumbsEvents = function(){
        $(`[data-component="breadcrumbs"]`).off()
        $(`[data-component="breadcrumbs"]`).on('click', function(e){
            let data = e.target.dataset

            let rerender = false
           _this.breadcrumbs.forEach(function(crumb){
                if(data.crumb && crumb.component === data.crumb && !crumb.selected){
                        crumb.selected = true
                        rerender = true
                        $(`[data-component="${crumb.component}"]`).show()
                }else if(data.crumb && crumb.component !== data.crumb){
                    crumb.selected = false
                    $(`[data-component="${crumb.component}"]`).hide()
                }
           })
           if(rerender) _this.renderBreadcrumbs()
        })
    }

    this.filterTableHeaderRow = function() {

        _this.allParts.forEach(function(){})
    }


    this.onInfoLoaded = function ( _ , data, request) {

        _this.tableHeaderRow = [
            {name:'severity' , title:'Severity', render: function(level){ 
                return level ? `<i class="rv-${infoLevels[level]}" title="${level}"/>` : ''
            }}, 
            {name: 'name', title:'Part' , render: function(name, row){ return `${row.reqID}`} },
            {
                name:'reqID', 
                title:'Messages' , 
                render: function(reqID, row){ 
                    let button = {name: `view`, classes:'hv-btn-minor' , event:'view-msgs', data:{reqid:reqID, objid: row.objID} }
                    return row.severity === "" || !row.severity 
                                ?  `<em style="color: #aaa"> none </em>`
                                : createFakeButton(button)
                }
            },
            {name: 'progress100', title:'Progress' , render: function(percent, row){ return `<progress value="${percent}" max="100"></progress> ${percent}% &nbsp; ${row.status}`} },
            {
                name: 'execTime', 
                title:'Elapsed' , 
                style: {
                    'min-width': '80px'
                },
                render: function(time){ return formatTimespan(time)} },
            {
                name: 'waitTime', 
                title:'Waited' , 
                style: {
                    'min-width': '80px'
                },
                render: function(time){ return formatTimespan(time)} },
            {
                name: 'status', 
                title:'Action' , 
                render: function(status, row)
                { 
                    if(status && status != "" && status != "0"){
                        return ProgressActionButton({
                            status: status,
                            data: {
                                reqid: row.reqID || '',
                                grpid: row.grpID || '',
                            }
                        });
                    }
                    return '';
                } 
            }
        ]

        if(request.status < 400 && data && data !== "unknown" && data !== ""){
            var parsedData = JSON.parse(data);
            _this.allParts = parsedData.Head.parts || []
            
            if(parsedData.Head && ( parsedData.Head.severity === "" || !parsedData.Head.severity )) {
                _this.tableHeaderRow.forEach(function(col, index) {
                    if (col.name === 'severity' || col.name === 'reqID') _this.tableHeaderRow.splice(index, 1);
                })
            }

            _this.parts = _this.allParts

            if(_this.onPartsDataLoaded) _this.onPartsDataLoaded(parsedData)

            _this.breadcrumbs=[{
                title: parsedData.Head.name,
                component: _this.componentName,
                selected: true
            }]
        } else {
            _this.allParts = []
            _this.parts = _this.allParts
        }
        
        

        _this.renderTable()

        _this.handleTableEvents()

        _this.renderBreadcrumbs()
        
        _this.handleBreadcrumbsEvents()

    }; 

    let dsPartsInfo_url = 'http:
    Object.keys(props.params).forEach(function(param){
        dsPartsInfo_url = urlExchangeParameter (dsPartsInfo_url, param, props.params[param]);
    })
    vjDS.add("", this.dsPartsInfo , dsPartsInfo_url);
    vjDS[this.dsPartsInfo].register_callback(this.onInfoLoaded);
    vjDS[this.dsPartsInfo].reload(dsPartsInfo_url,true)

    return (
        $(document.createElement("div"))
            .append(
                $(document.createElement("div"))
                    .attr("data-component", 'breadcrumbs'),
                $(document.createElement("div"))
                    .attr("data-component", this.componentName)
                    .append(
                        $(document.createElement("div"))
                            .css('display' , 'flex')
                            .css('justify-content' , 'space-between')
                            .append(
                                createButtonSet(this.buttons)
                            )
                            .append(
                                createButtonSet(this.pager_buttons )
                            ),
                        $(document.createElement("div"))
                        .attr("id",this.tableId)
                        .html('<img border="0" width="24" src="img/progress.gif"> Loading ...')
                    )
            )
    )
}
