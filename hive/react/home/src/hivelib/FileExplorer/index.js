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
import React, { Component } from "react";
import "antd/dist/antd.css";
import "@ag-grid-community/all-modules/dist/styles/ag-grid.css";
import "@ag-grid-community/all-modules/dist/styles/ag-theme-balham.css";
import SplitPane from "react-split-pane";

import "./css/main.css";

import TreeList from "./components/tree_list";
import {FooterContainer} from "../Footer/footer_container"
import TableContainer from "./components/table_container";
import DataPreview from "./data-preview/data-preview_container";
import { sortAlphAndCount } from "./../modal/sort-filter_modal";
import { filterTree } from "./modal/tree_modal";

import { CustomRequest as RequestConstructor } from "../modal/request_modal";

export class FileExplorer extends Component {
  constructor(props) {
    super(props);
    this.allObjects = {
      title: "All Objects",
      type: "sysfolder",
      key: '0',
      isLeaf: true,
      perm: { act: { write: true } }
    };
    this.state = {
      treeData: [this.allObjects],
      selectedItem: null,
      tableItems: [],
      findObject: null,
      pageLoading: true
    };
    this.splitHeight = this.props.height ? this.props.height : "100% - 40px";
    this.styleSplitPane = {
      // height:`calc(${this.splitHeight})`
      height: `100%`
    };
    this.pane2Style = {
      height: `calc(${this.splitHeight})`
    };
    this.localStorageSelectedMenuKey = 'selectedMenu';
  }

  /// Lifecycle hook fetch data
  componentDidMount = () => {

    let parameters = {
        cmdr: "objList" ,
        mode: "json" ,
        actions: "1" ,
        //perm: "1" ,
        type: "^sysfolder$" ,
        prop: "name,_effperm"
    }

    let request = new RequestConstructor({ parameters });

    request.handleFetch()
      .then((response) => response.json())
      .then((json) => json.objs.map(result => result))
      .then((newData) => {
        //Sort Alphabetically
        newData = sortAlphAndCount(newData, 'name' )
        let filteredTree = filterTree(newData);
        let selectedMenuItem = JSON.parse(localStorage.getItem(this.localStorageSelectedMenuKey));
        let selectedMenuTitle;

        if (!selectedMenuItem || !selectedMenuItem.title || selectedMenuItem.title === '') selectedMenuTitle = "Inbox";
        else if (!selectedMenuItem.parent) selectedMenuTitle = selectedMenuItem.title;
        else selectedMenuTitle = selectedMenuItem.key;

        let selectedMenu = filteredTree.filter((item) => item.title === selectedMenuTitle);

        if (selectedMenu.length === 0 && selectedMenuItem && selectedMenuItem.key) {
          if (selectedMenuItem.key.toString() === "0" )
            selectedMenu = [this.allObjects];
          else {
            selectedMenuTitle = "Inbox";
            selectedMenu = filteredTree.filter((item) => item.title === selectedMenuTitle);
          }
        }

        filteredTree = filteredTree.map(item => {
          if (item.title.search(/Shared with me/i) > -1) {
            item.title = "Shared with me ---";
          }
          return item;
        });

        let initialFolder;
        initialFolder = selectedMenu && selectedMenu.length>0? selectedMenu[0] : this.allObjects;

        this.setState((state, props) => {
          let toReturn = {};
          toReturn["treeData"] = [state.treeData[0], ...filteredTree];
          toReturn["reloadTable"] = false;
          ///////////////////////////////////
          /// Open Inbox as initial TABLE ///
          initialFolder = props.tabs ? state.treeData[0] : initialFolder;
          toReturn["initialFolder"] = initialFolder;
          toReturn["selectedItem"] = state.selectedItem
            ? state.selectedItem
            : initialFolder;
          if (
            typeof props.findObject === "object"
            && props.findObject !== null
          ) {
            toReturn["selectedTableItemKey"] = props.findObject._id;
          } else {
            toReturn["selectedTableItemKey"] = initialFolder.key
              ? initialFolder.key
              : props.findObject;
          }

          return toReturn;
        });
      })
      .catch((error) => console.log("IndexTreeDataLoad:" + error));

      parameters = {
          cmdr: "objList" ,
          mode: "json" ,
          actions: "1" ,
          //perm: "1" ,
          type: "^sysfolder$" ,
          prop: "name,type_count,_effperm"
      }

      request = new RequestConstructor({ parameters });
      request.handleFetch()
        .then((response) => response.json())
        .then((json) => json.objs.map(result => result))
        .then((newData) => {

          newData = sortAlphAndCount(newData, 'name' )
          let filteredTree = filterTree(newData);

          this.setState((state, props) => {
            let toReturn = {};
            toReturn["treeData"] = [state.treeData[0], ...filteredTree];
            toReturn["reloadTable"] = false;
            toReturn["pageLoading"] = false;

            return toReturn;
          });
        })
        .catch((error) => console.log("IndexTreeDataLoad:" + error));
  };

  componentDidUpdate = (prevProps, prevState) => {
    if (
      prevProps.findObject !== this.props.findObject
      && this.state.findObject === null
    ) {
      this.setState((state, props) => {
        return { selectedTableItemKey: props.findObject._id };
      });
    }
  };

  relaodTreeTable = () => {
      this.setState({ reloadTable: true });
      this.componentDidMount();
  };

  renderPreview = (items) => {
    let id, type;
    let show = true;
    let tabs = [];
    if (items.length && items.length > 0 && Array.isArray(items)) {
      id = items[items.length - 1]._id;
      type = items[items.length - 1]._type;
      tabs = items[items.length - 1]._js_component || [];
    } else if (typeof items === "object") {
      id = items._id;
      type = items._type;
      tabs = items._js_component || []
    } else {
      show = false;
    }

    if (!id) { show = false; }

    return show
            ? (
                <DataPreview
                  className="dashboard-table-container"
                  tabs={tabs}
                  ids={id}
                  type={type}
                  tabDataPreview={
                    this.props.tabDataPreview ? this.props.tabDataPreview : null
                  }
                />
              )
            : "";
  };

  handleTableEvents = (e) => {
    if (e.target.id === "view-file-in-tree") {
      this.componentDidMount();
    }
  };

  handleOnTreeItemSelect = (selectedItem) => {
    this.setState((state, props) => {
      let toReturn = {};
      toReturn["selectedItem"] = selectedItem;
      if (props.recordTreeSelect && selectedItem.key.toString() !== "0") {
        toReturn["tableItems"] = [selectedItem];
      }
      if (props.findObject !== null) {
        toReturn["findObject"] = false;
      }
      return toReturn;
    });

    this.setItemPath(selectedItem);
  };

  setItemPath = (selectedItem) => {
    //keep for next task
    //findFolderPathKey(selectedItem.key).then((path) => {
    //  path = path.join(' / ');
    //})

    localStorage.setItem(this.localStorageSelectedMenuKey, selectedItem? JSON.stringify(selectedItem): null);
  };

  handleOnTableItemSelect = (item, isFolder) => {
    this.setState((state, props) => {
      let toReturn = {};
      toReturn["selectedTableItemKey"] = item._id;
      if (props.findObject !== null) {
        toReturn["findObject"] = false;
      }
      toReturn["selectedItem"] = isFolder ? item : state.selectedItem;

      return toReturn;
    });
  };

  handleOnActionData = (actionsData) => {
    this.setState({ actionsData });
  };

  handleSelectedTableItems = (tableItems) => {
    this.setState({ tableItems }); // list of tableTtems is needed to RENDER  this.renderPreview
  };

  onRepopulateInitialFolder = initialFolder => {
    this.setState({ selectedItem: initialFolder });
  };

  render() {
    return (
      <div className="hive home dashboard-container">
        <SplitPane
          style={this.styleSplitPane}
          split="vertical"
          minSize={175}
          maxSize={500}
          defaultSize={275}
          pane1Style={{ height: "inherit" }}
          pane2Style={this.pane2Style}
        >
          <aside className="dashboard-sidenav" style={{ height: "inherit" }}>
            <TreeList
              pageLoading={this.state.pageLoading}
              onTreeItemSelect={this.handleOnTreeItemSelect}
              tableItemSelect={this.state.selectedTableItemKey}
              treeData={this.state.treeData}
              actionsData={this.state.actionsData}
              reload={(reload) => ( reload ? this.relaodTreeTable() : null )}
              initialFolder={this.state.initialFolder}
              onRepopulateInitialFolder={this.onRepopulateInitialFolder.bind( this )}
              isDraggable={
                typeof this.props.isDraggable === "boolean"
                  ? this.props.isDraggable
                  : true
              }
            />
          </aside>
          <div className="dashboard-main" onClick={this.handleTableEvents}>
            <div className={
              this.props.hasFooter === true
                ? "dashboard__main--flex-footer"
                : "dashboard__main--flex"
              }
            >
              <TableContainer
                pageLoading={this.state.pageLoading}
                singleSelect={
                  typeof this.props.singleSelect === "boolean"
                    ? this.props.singleSelect
                    : false
                }
                recordTreeSelect={
                  typeof this.props.recordTreeSelect === "boolean"
                    ? this.props.recordTreeSelect
                    : false
                }
                findObject={
                  this.state.findObject === false
                    ? ""
                    : this.props.findObject
                }
                tabsToShow={this.props.tabs}
                onTableItemSelect={this.handleOnTableItemSelect.bind(this)}
                reloadTable={this.state.reloadTable}
                treeItem={this.state.selectedItem}
                onActionData={this.handleOnActionData}
                selectedTableItems={this.handleSelectedTableItems.bind(this)}
                hasUploader={
                  typeof this.props.hasUploader === "boolean"
                    ? this.props.hasUploader
                    : true
                }
                hasRunComp={
                  typeof this.props.hasRunComp === "boolean"
                    ? this.props.hasRunComp
                    : true
                }
                isDraggable={
                  typeof this.props.isDraggable === "boolean"
                    ? this.props.isDraggable
                    : true
                }
              />
              {
                this.props.customPreview !== null
                  ? this.props.customPreview(this.state.tableItems)
                  : null
              }
              {
                this.props.hasDataPreview
                && this.props.findObject !== null
                && this.state.findObject !== false
                  ? this.renderPreview(this.props.findObject)
                  : this.props.hasDataPreview
                  ? this.renderPreview(this.state.tableItems)
                  : null
              }
            </div>
            {
              this.props.hasFooter === true
              ? <FooterContainer footerStyle='dashboard__main__footer'/>
              : ''
            }
          </div>
        </SplitPane>
      </div>
    );
  }
}