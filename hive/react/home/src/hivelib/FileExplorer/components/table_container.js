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
import { Spin } from 'antd';

import { CustomRequest as RequestConstructor } from "../../modal/request_modal";

import TableDetail from "./table_detail";
import TabContainer from "./tab_container";
import { PaginationContainer } from "./pagination_container";
import { columnsConstruct } from "./../view/table_view";
import { columnsReorder } from "./../modal/table_modal";

import ActionsData from "./../actions";

class TableContainer extends Component {
    constructor(props) {
        super(props);
        this.state = {
            tableDetail: [],
            selectedTab: { types: ['^base_user_type$+'], prop: '_brief' }, // Initially Tab All should be selected
            selectedTableItems: [],
            columns: [],
            pagerInfo: { total: 0, start: 0, cnt: 10 },
            columnOrders: {},
            columnNames: {},
            tableSearch: "",
            tableSpinner: false
        };
    }

    ////////////////////////////////
    /// Action Details FULL LIST ///
    ////////////////////////////////
    componentDidMount = () => {
        let selectedTab = {
            title: 'View All',
            types: ['^base_user_type$+'], prop: '_brief'
        }

        if (this.props.tabsToShow !== null) {
            selectedTab = this.props.tabsToShow
        }

        // !!! When we will have an api request for actions
        let parameters = {
            cmdr: "objList",
            mode: "json",
            prop: '_id,name,title,description,url,path,target',
            type: '^action$',
            flatten: '1'
        }
        let request_table = new RequestConstructor({ parameters });

        request_table.handleFetch()
            .then((response) => response.json())
            .then((json) => json.objs.map(result => result))
            .then((actions) => {
                actions = this.mergeAction(actions)
                this.setState({ selectedTab, actionsData: actions });
                this.props.onActionData(actions);
            })
            .catch((error) => console.error(error));
    }

    // Temporary Merge data
    mergeAction = (old_data) => {
        let merged_data = ActionsData.objs;
        merged_data.forEach((action, i) => {
            const name = action.name;
            let found = false;
            let j = 0;
            while (found === false && j < old_data.length) {
                if (old_data[j].name !== name) {
                    j++;
                } else {
                    merged_data[i] = Object.assign(old_data[j], merged_data[i])
                    found = true;
                }
            }
        })
        return merged_data;
    }

    //////////////////////////////
    /// Refreshes Table Detail ///
    //////////////////////////////
    loadTableItems = (column_setup = true) => {
        this.setState({ tableSpinner: true });
        return new Promise((resolve, reject) => {
            const id = this.state.treeItem ? this.state.treeItem.key : 0;
            const title = this.state.treeItem ? this.state.treeItem.title : 'All Objects';
            const types = this.state.treeItem ? this.state.selectedTab.types : "sysfolder";
            const start = this.state.pagerInfo.start;
            const cnt = this.state.pagerInfo.cnt;
            const dataInfo = this.state.selectedTab.prop === '_brief' ? '_brief' : '_summary';

            let parameters = {
                cmdr: "objList",
                mode: "json",
                actions: "1",
                js_components: '1',
                prop: `created,${dataInfo},submitter,name,single_obj_only`, // Need to define name for renamer action
                flatten: '1',
                info: '1',
                start: start,
                cnt: cnt,
                type: types
            }

            if (title === "Trash") { parameters['showTrashed'] = '1' }

            if (id.toString() !== '' && id.toString() !== '0') {
                parameters['parP'] = 'child'
                parameters['parIds'] = id
            }
            if (this.state.tableSearch.toString().length > 0) {
                parameters['search'] = this.state.tableSearch
                parameters['cnt'] = 'all'
            }

            let request_table = new RequestConstructor({parameters});

            request_table.handleFetch()
                .then((response) => response.json())
                .then((json) => {
                    /////////////////////////////
                    /// Setting up PAGER info ///
                    /////////////////////////////
                    const pagerInfo = json.info;
                    delete pagerInfo.page;
                    //fixes issue after selecting an empty page
                    if (pagerInfo.cnt === 0) {
                        pagerInfo.cnt = cnt;
                    }
                    /////////////////////////////

                    let newData = json.objs.map( (result) => {
                                    result.key = result._id;
                                    if(sessionStorage.getItem("projectID") && result.submitter){
                                        result.submitter += `&projectID=${sessionStorage.getItem("projectID")}`
                                    }
                                    return result;
                                });

                    if (column_setup) {
                        this.setState({
                            pagerInfo: { ...this.state.pagerInfo, ...pagerInfo },
                            tableSpinner: false,
                            tableDetail: newData
                        });

                        this.columnSetUp(newData);
                    } else {
                        this.setState({
                            pagerInfo: { ...this.state.pagerInfo, ...pagerInfo },
                            tableSpinner: false,
                        });
                        resolve(newData);
                    }
                })
                .catch((error) => console.error('LoadTableItems: ' + error));
        })
    }

    /////////////////////////////////////////////////////
    /// Filter Out Columns & Get Charater Count+Width ///
    /////////////////////////////////////////////////////
    columnSetUp = (newData) => {
        /////////////////////////////////////////////////////
        /// Filter Out Columns & Get Charater Count+Width ///
        /////////////////////////////////////////////////////
        let type_columns = [];
        let type_columns_chr_cnt = {};
        let current_table_types = [];
        let add_table_types = [];
        let columnOrders = this.state.columnOrders;
        let columnNames = this.state.columnNames;

        newData.forEach((row) => {

            /// Attribute is column
            /// Record the info about
            for (var attribute in row) {
                let att_value = row[attribute];

                if (attribute === "_type") {
                    /// See whether info about _type exists in array of current_table_types
                    if (!current_table_types.includes(att_value)) {
                        current_table_types.push(att_value);
                        if (!Object.keys(columnOrders).includes(att_value)) {
                            add_table_types.push(att_value);
                        }
                    }
                }

                if (
                    this.props.hasRunComp
                    && type_columns.indexOf('submitter') < 0
                    && row.submitter
                ) {
                    //Look up if _type its part of actions
                    type_columns.push('submitter')
                }


                /// 1. Translate column content into string
                if (typeof att_value === 'number') {
                    att_value = (att_value).toString();
                }
                /// 2a. See if column already recorded + modify width
                if (type_columns.includes(attribute)) {
                    if (type_columns_chr_cnt[attribute] < att_value.length) {
                        type_columns_chr_cnt[attribute] = att_value.length;
                    }
                    continue;
                    /// 2b. Record column and it's width
                } else {
                    if (
                        attribute !== "key"
                        && attribute !== "_action"
                        && attribute !== "_js_component"
                        && !(row.hasOwnProperty('_brief') && attribute === 'name')
                    ) { // attribute name is needed alongside brief for rename action
                        type_columns.push(attribute)
                    }
                    type_columns_chr_cnt[attribute] = att_value.length; /// Based on the NAME of the column
                }
            }

            // Add RunAction
            // I know type here see if it matches with any one with actions
            // add composed url
        });

        // For table filter
        const setCurrentTypeName = () => {
            const currentTypeName = current_table_types.map((type) => {
                let info = {
                    text: columnOrders[type].title,
                    value: type
                }
                return info;
            })
            return currentTypeName;
        }

        const setOrderColumnState = (columnNames, columnOrders) => {
            let column_reorder = columnsReorder(current_table_types, columnOrders, type_columns);
            this.setState({
                columnOrders,
                columnNames,
                columns: this.state.selectedTab.types.length > 0 ? columnsConstruct(column_reorder, type_columns_chr_cnt, columnNames , columnOrders) : [],
                currentTableTypes: setCurrentTypeName(),
            });
        }


        if (add_table_types.length === 0) {
            setOrderColumnState(columnNames, columnOrders);
        } else {
            /// Set proper column Name & Order
            const columnInfo = this.loadPropInfo(add_table_types);
            columnInfo.then((values) => {
                const [names, orders] = values;
                for (var attribute in names) {
                    if (!columnNames[attribute]) {
                        columnNames[attribute] = names[attribute];
                    }
                }
                for (var type in orders) {
                    if (!columnOrders[type]) {
                        columnOrders[type] = orders[type];
                    }
                }

                setOrderColumnState(columnNames, columnOrders);
            })
        }

        /////////////////////////////////////////////////////
    }

    ///////////////////////////////////
    /// Proper COLUMN names & order ///
    ///////////////////////////////////
    loadPropInfo = (types) => {
        return new Promise((resolve, reject) => {
            let columnNames = {};
            let columnOrders = {};

            let parameters = {
                cmdr: "propspec3",
                types: types
            }

            let request_table = new RequestConstructor({ parameters });

            request_table.handleFetch()
                .then((response) => response.json())
                .then((newData) => {
                    let LoopThrowColumn = (list , type) => {
                        for (var column in list) {
                            let column_att = list[column];
                            columnOrders[type.name].order[column] = column_att.order;
                            columnNames[column] = column_att.title;
                            if (column_att._children) {
                                LoopThrowColumn(column_att._children, type)
                            }
                        }
                    }
                    newData.forEach((type) => {
                        columnOrders[type.name] = {};
                        columnOrders[type.name].title = type.title;
                        columnOrders[type.name].order = {};
                        LoopThrowColumn(type._attributes, type)
                    })
                    resolve([columnNames, columnOrders]);
                })
                .catch((error) => console.error('loadPropInfo: ' + error));
        })
    }

    //////////////////////////////////////
    /// Table Items Loader Initializer ///
    //////////////////////////////////////
    componentDidUpdate = (prevProps, prevState) => {
        /// Check whether props has changed and fetch data if true
        /// Selected Tree Item ///
        if (this.props.treeItem !== prevProps.treeItem) {
            this.setState((state, props) => {
                return {
                    pagerInfo: {
                        total: 0,
                        start: 0,
                        cnt: state.pagerInfo.cnt,
                    },
                    treeItem: props.treeItem
                }
            },
                this.loadTableItems
            )
            this.deselectAction();
        }
        if (this.props.reloadTable === true && this.props.reloadTable !== prevProps.reloadTable) {
            const column_setup = false;
            const data = this.loadTableItems(column_setup);
            const promise1 = Promise.resolve(data);
            promise1.then((item) => {
                const isEquivalent = this.isEquivalent(this.state.tableDetail, item);
                if (!isEquivalent) {
                    this.setState({ tableDetail: item });
                }
            });
        }
        if (this.state.tableSearch !== prevState.tableSearch) {
            this.loadTableItems();
        }
        /// Pager Used ///
        if (
            (this.state.pagerInfo.start !== prevState.pagerInfo.start || this.state.pagerInfo.cnt !== prevState.pagerInfo.cnt)
            || this.state.selectedTab.types !== prevState.selectedTab.types
        ) {
            this.loadTableItems();
        }
    }

    /////////////////////
    /// isEquivalent  ///
    /////////////////////
    isEquivalent = (old_data, new_data) => {
        if (old_data.length !== new_data.length) {
            return false;
        }

        for (let i = 0; i < old_data.length; i++) {
            const a = old_data[i];
            const b = new_data[i];

            if (a._id !== b._id && (a._brief !== b._brief || a.name !== b.name)) {
                return false;
            }
        }

        return true;
    }

    //////////////////
    /// Tab Change ///
    //////////////////
    handleTabChange = (leavingTab, enteringTab) => {
        const { pagerInfo } = this.state

        const leavingTabTitle = leavingTab.title.replace(/\s+/g, "_");

        const enteringTabTitle = enteringTab.title.replace(/\s+/g, "_");
        const pagerHasEnteringTab = pagerInfo.hasOwnProperty(enteringTabTitle);

        if (pagerHasEnteringTab) {
            this.setState({
                pagerInfo: {
                    ...this.state.pagerInfo,
                    total: this.state.pagerInfo[enteringTabTitle].total,
                    start: this.state.pagerInfo[enteringTabTitle].start,
                    cnt: this.state.pagerInfo.cnt
                    ,
                    [leavingTabTitle]: {
                        total: this.state.pagerInfo.total,
                        start: this.state.pagerInfo.start,
                        cnt: this.state.pagerInfo.cnt
                    }
                },
                selectedTab: enteringTab
            })
        } else {
            this.setState({
                pagerInfo: {
                    ...this.state.pagerInfo,
                    total: this.state.pagerInfo.total,
                    start: 0,
                    cnt: this.state.pagerInfo.cnt,
                    [leavingTabTitle]: {
                        total: this.state.pagerInfo.total,
                        start: this.state.pagerInfo.start,
                        cnt: this.state.pagerInfo.cnt
                    }
                },
                selectedTab: enteringTab
            })
        }
    }

    ///////////////////////
    /// Deselect Action ///
    ///////////////////////
    deselectAction = () => {
        if (!this.props.recordTreeSelect) {
            this.props.selectedTableItems([]); // To clear bottom preview
        }
        this.setState({ selectedTableItems: [] });
    }

    ///////////////////////
    /// Deselect Button ///
    ///////////////////////
    deselectItems = () => {
        if (this.state.selectedTableItems.length > 0) {
            return <button className="btn--minor" onClick={this.deselectAction}> deselect all</button>;
        }
    }

    handleSelectedTableItems = (items) => {
        if (items.length === 0 && this.state.selectedTableItems.length === 0) {
            return; // Prevent double render
        } else if (
            items.length === 1
            && this.state.selectedTableItems.length === 1
            && items[0]._id === this.state.selectedTableItems[0]._id
        ) {
            return; // Prevent double render if id is same
        } else {
            this.props.selectedTableItems(items);
            this.setState({ selectedTableItems: [...items] });
        }
    }

    handleFindFileInTree = (id) => {
        this.props.onTableItemSelect(id, false)
    }

    handleTableSearch = (value) => {
        this.setState({ tableSearch: value })
    }

    handleReload = (reload) => {
        if (reload) {
            this.loadTableItems()
        }
    }

    handleOpenTableFolder = (treeItem) => {
        this.setState(
            { treeItem },
            this.loadTableItems,
            this.props.onTableItemSelect(treeItem, true)
        )
        this.deselectAction();
    }

    handleOnPagerSelect = (info) => {
        this.setState({ pagerInfo: { ...this.state.pagerInfo, ...info } })
    }

    render() {
        if (!this.state.treeItem) {
            return <div> Select Folder </div>;
        } else {
            return (<>
                <div className="dashboard-table-container">
                    <TabContainer
                        tabs={this.props.tabsToShow ? [this.props.tabsToShow] : null}
                        handleTabChange={this.handleTabChange}
                        scrFolder={this.state.treeItem.key}
                    />
                    <Spin spinning={this.state.tableSpinner || this.props.pageLoading} >
                        <TableDetail
                            findObject={this.props.findObject}
                            scrFolder={this.state.treeItem}
                            openTableFolder={this.handleOpenTableFolder.bind(this)}
                            findFileInTree={this.handleFindFileInTree}
                            actionsData={this.state.actionsData}
                            tableDetail={this.state.tableDetail}
                            currentTableTypes={this.state.currentTableTypes}
                            selectedType={this.state.selectedTab.types}
                            columns={this.state.columns}
                            selectedTableItems={this.handleSelectedTableItems.bind(this)}
                            singleSelect={this.props.singleSelect}
                            deselectItems={this.state.selectedTableItems.length}
                            handleTableSearch={this.handleTableSearch}
                            reload={this.handleReload}
                            hasUploader={this.props.hasUploader}
                            isDraggable={this.props.isDraggable}
                        />
                    </Spin>
                    <div className="hive-table__bottom">
                        <div className="hive-table__selection">
                            {!this.props.singleSelect ?
                                <>
                                    <p>{this.state.selectedTableItems.length} items selected</p>{this.deselectItems()}
                                </> : ""
                            }
                        </div>
                        <div>
                            <PaginationContainer
                                pagerInfo={this.state.pagerInfo}
                                onPagerSelect={this.handleOnPagerSelect}
                                tableSpinner={this.state.tableSpinner}
                            />
                        </div>
                    </div>
                </div>
                </>
            );
        }
    }
}

export default TableContainer;