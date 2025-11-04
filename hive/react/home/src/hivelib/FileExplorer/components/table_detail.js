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
import React , { Component } from "react";
import { Button } from "antd";
import { AgGridReact } from "@ag-grid-community/react";
//import { AgGridReact } from "ag-grid-react";
import { AllCommunityModules } from "@ag-grid-community/all-modules";

import { setUpActionList } from "./../modal/action_modal";
import { ButtonRow } from "./action_container";
import { SearchContainer } from "./search_container";

//scrFolder | actionsData | tableDetail | selectedType | columns
class TableDetail extends Component {
  constructor(props) {
        super(props);
        this.state = {
            modules: AllCommunityModules, //AgTable
            defaultColDef: { //AgTable
                headerCheckboxSelection: isFirstColumn,
                checkboxSelection: isFirstColumn,
                resizable: true
            },
            rowSelection: "multiple",  //AgTable
            actionList: null,
            selectedRowKeys: [],
            selectedRows: [],
            selectedRowsTypes: [],
            columns: [],
            submitter: ''
        };
  }
  componentDidMount(){
      if(typeof this.props.findObject === 'object' && this.props.findObject !== null){
        this.setState((state,props)=>{
             return {selectedRowKeys: [props.findObject._id]}
         })
         this.handleSearchValue(this.props.findObject._id)
      }
      if(this.props.singleSelect){
          this.setState({ rowSelection:'single' })
      }
  }

  shouldComponentUpdate(nextProps, nextState){
    if (
        nextProps.deselectItems === 0
        && nextProps.deselectItems !== this.props.deselectItems
        && nextProps.deselectItems !== nextState.selectedRows.length
    ) {
        this.tableApi.deselectAll(); // deselet all table items
        this.setState({
            selectedRowKeys: [],
            selectedRows: [],
            submitter: ''
        })
    }
    return true;
  }

  componentDidUpdate(prevProps){
      if (
          prevProps.tableDetail !== this.props.tableDetail
          && (prevProps.tableDetail.length !== 0  || prevProps.tableDetail.length !== 0)
      ) {
          this.renderCheckboxes();
      }
      if (prevProps.findObject !== this.props.findObject) {
         if (typeof this.props.findObject !== 'object') {
             this.handleSearchValue(this.props.findObject)
         } else {
            this.handleSearchValue(this.props.findObject._id)
         }
         this.setState((state,props)=>{
             return {selectedRowKeys: [props.findObject._id]}
         })

      }
  }

    onDragStarted = (event) => {
        if (event.target.attributes["row-id"]) {
            let rowId = event.target.attributes["row-id"].value
            let record = this.tableApi.getRowNode(rowId).data;
            this.dragStartHandler(record, event);
        }
    }

    dragStartHandler = (record, event) => {
        const {selectedRowKeys} = this.state;

        // Need to show in UI that ther is no premssion to write and read
        const write = this.props.scrFolder.perm.act.write;
        const read = this.props.scrFolder.perm.act.read;
        const key = record.key;
        const actions = record._action;
        const partOfSelectRows = selectedRowKeys.length > 0 && selectedRowKeys.includes(key);

        if (!read && !write) { return; }

        if (actions.includes('cut')) {
            if (partOfSelectRows) {
                event.dataTransfer.setData('text/plain', selectedRowKeys );       //record items dragged
            } else {
                event.dataTransfer.setData('text/plain', selectedRowKeys + key ); //record items dragged
            }
            event.dataTransfer.effectAllowed = "copyMove";                        //proper arrow image
        } else {
            event.dataTransfer.effectAllowed = "none";
        }

        let numDragObjects;
        if (partOfSelectRows) {
            numDragObjects = selectedRowKeys.length;
        } else {
            numDragObjects = selectedRowKeys.length + 1;
            event.target.style.opacity = '0.4';  // selection loses opacity
        }

        selectedRowKeys.map((key) => {
            let row = document.querySelector(`div[row-id='${key}']`);
            row.style.opacity = '0.4';
            return row; // -----
        })

        let tooltip = document.createElement("div");
        tooltip.id = 'drag-tooltip';
        tooltip.classList.add('drag-tooltip');

        if (write) {
            // move
            tooltip.innerHTML = `Moving: ${numDragObjects} objects`;
        } else if (read) {
            //copy
            tooltip.innerHTML = `Coping: ${numDragObjects} objects`;
        }

        document.body.appendChild(tooltip);
        event.dataTransfer.setDragImage(tooltip, 0, 0);

        event.stopPropagation();
    }

    onDragEnd = (event) => {
        const {selectedRowKeys} = this.state;

        event.target.style.opacity = '1';
        if (selectedRowKeys.length > 0) {
            selectedRowKeys.map((key) => {
                let row = document.querySelector(`div[row-id='${key}']`);
                row.style.opacity = '1';
                return row; // ---------------
            })
        }

        let ghost = document.getElementById("drag-tooltip");
        if (ghost.parentNode) {
            ghost.parentNode.removeChild(ghost);
        }
    }

    onDrag = (event) => {
        event.preventDefault();
    }

    setSelection = (keys,rows) => {
        let types = [];
        const { tableDetail } = this.props
        let selectedRows = this.state.selectedRows;
        let selectedRowKeys = this.state.selectedRowKeys;

        if (keys.length === 0 && rows.length === 0) {          //If all deselected remove from state
            tableDetail.forEach( row => {
                let index = selectedRowKeys.indexOf(row._id)
                if (index > -1){
                     selectedRowKeys.splice(index,1);
                     selectedRows.splice(index,1);
                }
            })
        } else {
            rows.forEach((row) => {
                if (selectedRowKeys.indexOf(row._id) > -1){

                } else {
                    selectedRows.push(row)
                    selectedRowKeys.push(row._id)
                }
            })
            this.tableApi.forEachNode((node) => {
              let index = selectedRowKeys.indexOf(node.data._id)
              if (index > -1 && node.selected === false) {
                   selectedRowKeys.splice(index,1);
                   selectedRows.splice(index,1);
              }
            });
        }


        let submitter ;
        if(selectedRows.length > 0){
           selectedRows.forEach((el) =>{
                                    if (el.submitter) {
                                        submitter = submitter && el.submitter !== submitter ? '' : el.submitter;
                                    }
                                    types.push(el._type)
                            })
        }

        //!Temp for change name prompt
        let selectedRowTitle
        if(selectedRows.length === 1){
            selectedRowTitle = selectedRows[0].name || selectedRows[0]._brief
        }

        this.setState({
            selectedRowKeys,
            selectedRows,
            selectedRowTitle,
            actionList: setUpActionList(this.props.actionsData , selectedRows),
            selectedRowsTypes: types,
            submitter: submitter ? submitter : ''
        })
        this.props.selectedTableItems(selectedRows);
    }

    onClearSelection = () => {
        this.props.selectedTableItems([]);
        this.props.reload(true);
    }

    //last clicked should be shown
    mapSelection = (keys,rows) => {
        const prevKeys = this.state.selectedRowKeys;
        const lengthDiff = keys.length - prevKeys.length;

        if (lengthDiff === 1) {
            rows.forEach((el,i) => {
                if (prevKeys.indexOf(el._id) < 0) {
                    rows.splice(i,1);
                    rows.push(el);
                }
            })
        }

        this.setSelection(keys,rows);
    }

    filterSetUp = (col) => {
        let colFilter = {};
        colFilter.filters = this.props.currentTableTypes;
        colFilter.onFilter = (value , record) =>  record._type.indexOf(value) > -1;

        return colFilter;
    }

    handleSearchValue = (value) => {
        this.props.handleTableSearch(value)
    }

    handleReload = (reload) => {
        this.props.reload(reload)
    }

    handleFindFileInTree = (id) => {
        id = {_id:  id}
        this.props.findFileInTree(id);
    }

    renderFindFileInTree = (selectedRowKeys) => {
        if (selectedRowKeys.length === 1) {
            return <Button id="view-file-in-tree" onClick={() => this.handleFindFileInTree(selectedRowKeys[0])} > View file location</Button>;
        }
    }

///////////////////////////////////////////////////////////////////////////////////////////
    onSelectionChanged(event) {
        let keys = [];
        let rows = event.api.getSelectedNodes().map((row) =>{
            keys.push(row.data._id)
            return row.data;
        });

        this.mapSelection(keys, rows)
    }

  onRowDoubleClicked(event) {
    let record = event.data;

    if (record._type === "folder") {
      record.type = record._type;
      this.props.openTableFolder(record);
    }
  }

  onGridReady = (params) => {
    this.tableApi = params.api;
    this.tableColumnApi = params.columnApi;
  }

  getRowNodeId(data) {
     return data._id;
  }

  renderCheckboxes(keys = []) {
    let {selectedRowKeys} = this.state;
    this.tableApi.forEachNode((node) => {
      if(selectedRowKeys.indexOf(node.id) > -1) {
        node.setSelected(true);
      }
    });
  }

  onComponentStateChanged() {
    this.tableApi.sizeColumnsToFit();
    let rowgroup = document.querySelector("div[ref='eCenterContainer']");
    let rows = rowgroup ? rowgroup.querySelectorAll("div[role='row']") : [];
    //this.renderCheckboxes(); // for changing tabs or page

    rows.forEach((row) => {
          row.setAttribute("draggable", this.props.isDraggable)
    })
  }

    setTableHeight = () => {
        const data_length = this.props.tableDetail.length;
        let height;
        if (data_length < 20) {
            height = (data_length + 1) * 33;
        } else {
            height = 500;
        }

        return `${height}px`;
    }

  render(){
        const tableHeight = this.setTableHeight();
        return (
            <div className="table-contents">
                <div className="table-controls">
                    {this.renderFindFileInTree(this.state.selectedRowKeys)}
                    <ButtonRow actionList={this.state.actionList}
                               types={this.state.selectedRowsTypes}
                               selectedTitle={this.state.selectedRowTitle || ''}
                               selectedRowKeys={this.state.selectedRowKeys}
                               clearRowKeys={(clear) => (clear ?  this.onClearSelection() : null)}
                               srcFolder={this.props.scrFolder.key}
                               reload={this.handleReload}
                               hasUploader={this.props.hasUploader !== undefined ? this.props.hasUploader : true}
                               submitter={this.state.submitter}
                    />
                    <SearchContainer
                        className={"right"}
                        handleSearch={this.handleSearchValue}
                        placeHolder="Search Tab"
                    />
                </div>
                <div
                    className="dashboard-table"
                    style={{
                        width:'100%',
                        display:"block",
                        paddingBottom: "10px",
                        resize: "vertical",
                        overflow: "auto",
                        height: tableHeight,
                        boxSizing: "content-box"
                    }}
                >
                    <div
                        id="myGrid"
                        style={{
                          height: "100%",
                          width: "100%",
                        }}
                        className="ag-theme-balham"
                        onDragStart={this.onDragStarted.bind(this)}
                        onDragEnd={this.onDragEnd.bind(this)}
                        onDrag={this.onDrag.bind(this)}
                      >
                            <AgGridReact
                              modules={this.state.modules}
                              columnDefs={this.props.columns}
                              defaultColDef={this.state.defaultColDef}
                              rowSelection={this.state.rowSelection}
                              onGridReady={this.onGridReady}
                              getRowNodeId={this.getRowNodeId.bind(this)}
                              rowData={this.props.tableDetail}
                              onSelectionChanged={this.onSelectionChanged.bind(this)}
                              onRowDoubleClicked={this.onRowDoubleClicked.bind(this)}
                              onComponentStateChanged={this.onComponentStateChanged.bind(this)}
                            />
                    </div>
                </div>
            </div>
        );
    }
}

function isFirstColumn(params) {
  var displayedColumns = params.columnApi.getAllDisplayedColumns();
  var thisIsFirstColumn = displayedColumns[0] === params.column;
  return thisIsFirstColumn;
}

//                    style={{
//                       paddingBottom: "10px",
//                        resize: "vertical",
//                        overflow: "auto",
//                        height: "500px"
//                    }}

export default TableDetail;