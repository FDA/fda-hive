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

function ProgressMsgDialog (props) {
    let _this = this

    this.dialogId = `dialog-project`
    this.tableId = `dialog-table-project`
    let dialog_props = {
        dialogID: this.dialogId
    }
    this.dialog = new DialogComponent(dialog_props)

    this.onMsgDataLoaded = function(parsedData){ 
        _this.dialog.updateDialogTitle(`${parsedData.Head.name} : ${parsedData.Head.reqID}`) 
    }

    let table_props = {
        onMsgDataLoaded: this.onMsgDataLoaded,
        parentId: this.dialogId,
        tableId: this.tableId,
        ...props
    }

    this.table = new ProgressMsgsTable(table_props)

    this.dialog.updateDialogContent(this.table)
    
}

let MSGS_TABLE_COUNTER = 0

function ProgressMsgsTable (props) {
    let _this = this
    
    this.parts = props.parts || false
    this.componentName = props.componentName || 'progress-msgs'
    this.parentId = props.parentId
    this.tableId = props.tableId
    this.params = props.params
    this.onMsgDataLoaded = props.onMsgDataLoaded || null

    this.tableHeader = []
    this.allMessages = []    
    
    this.messages = this.allMessages

    this.dsMsgInfo = `dsMsgInfo_${MSGS_TABLE_COUNTER}`
    MSGS_TABLE_COUNTER ++

    this.currentPage = 1,
    this.pageSize = 10,
    this.maxPages = 7
    this.paginate = paginate(this.messages.length, this.currentPage, this.pageSize, this.maxPages)


    this.renderTable = function() {
        _this.paginate = paginate( _this.messages.length, _this.currentPage, _this.pageSize, _this.maxPages)
        let messages =  _this.messages.slice(_this.paginate.startIndex, _this.paginate.endIndex + 1)
        $(`[data-component="${_this.componentName}"]`)
            .find(`[data-pager="info"]`)
            .text(_this.messages.length ? `${_this.paginate.startIndex+1}-${_this.paginate.endIndex+1} of ${_this.messages.length}` : _this.messages.length)
        $(`#${_this.tableId}`)
            .html(createTable(_this.tableHeader,  messages))
        this.hidePagerButtons()
    }

    this.hidePagerButtons = function(){
        if(this.paginate.startIndex === 0 || this.messages.length === 0){
            $('[data-event="pager-prev"]').css('display' , 'none')
        }else{
            $('[data-event="pager-prev"]').css('display' , '')
        }
        if(this.paginate.endIndex + 1 === this.paginate.totalItems || this.messages.length === 0){
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
                    vjDS[_this.dsMsgInfo].reload(null,true)
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
                        _this.messages = e.target.value === 'All' 
                            ? _this.allMessages
                            : filterArrayOfObjects(_this.allMessages, 'level' , e.target.value)
                        
                        _this.renderTable()
                    }
                }
            }
        },
        
    ]

    this.pager_buttons = [
        {
            prepend:`<span data-pager="info">${this.messages.length ? `${this.paginate.startIndex+1}-${this.paginate.endIndex+1} of ${this.paginate.totalItems.length}` : this.messages.length}</span>

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
        },
    ]

    this.handleTableEvents = function(){
        if(_this.actions) {
            _this.actions.forEach(function(action){
                $(`[data-component="${_this.componentName}"]`).off(action.name )
                $(`[data-component="${_this.componentName}"]`).on(action.name , action.action)
            })
        }
        
    }

    this.onInfoLoaded = function (_, data, request){
        if(request.status < 400 && data && data !== "unknown" && data !== ""){
            var parsedData = JSON.parse(data);
            _this.allMessages = parsedData.Head.messages || []

            if(parsedData.Head.parts){
                parsedData.Head.parts.forEach( function(part){
                    if(part.reqID.toString() === _this.params.req.toString())  _this.allMessages = part.messages || []
                })
            }

            _this.messages = _this.allMessages

            if(_this.onMsgDataLoaded) _this.onMsgDataLoaded(parsedData)
        } else {
            _this.allMessages = parsedData.Head.messages || []
            _this.messages = _this.allMessages
        }

        _this.tableHeader = [
            {name:'level' , title:'Severity', render: function(level){ return `<i class="rv-${infoLevels[level]}" title="${level}"/>`}}, 
            {
                name: 'date', 
                title:'Time',
                style: {
                    'min-width': '100px'
                }, 
                render: function(time){ return formaDatetime(time, null , null ,null , true)} 
            },
            {name:'msg', title:'Message'}
        ]

        _this.renderTable()
        
        _this.handleTableEvents()
    }; 

    let dsMsgInfo_url = 'http:
    Object.keys(this.params).forEach(function(param){
        dsMsgInfo_url = urlExchangeParameter (dsMsgInfo_url, param, _this.params[param]);
    })
    vjDS.add("",  _this.dsMsgInfo, dsMsgInfo_url);
    vjDS[_this.dsMsgInfo].register_callback(this.onInfoLoaded);
    vjDS[_this.dsMsgInfo].reload(dsMsgInfo_url,true)

    return (
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
}