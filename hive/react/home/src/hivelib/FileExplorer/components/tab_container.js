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
import React, { PureComponent } from "react";
import { Tabs } from 'antd';

import { CustomRequest as RequestConstructor } from "../../modal/request_modal";
import { clone } from  "../../modal/data_modal";

const TabPane = Tabs.TabPane;

class TabContainer extends PureComponent {
  constructor(props) {
    super(props);
    const allTabs = [
      { title: 'View All', types: [''], prop: '_brief', count: 0 },
      { title: 'Folders', types: ['^folder$'], count: 0 },
      { title: 'Computations', types: ['^svc-computations-base$+'], count: 0 },
      { title: 'Genomes', types: ['^genome$'], count: 0 },
      { title: 'Reads', types: ['^nuc-read$'], count: 0 },
      { title: 'Annotations', types: ['^u-annot$','^u-ionAnnot$+'], count: 0 },
      { title: 'Files', types: ['^u-file$','^table$+','^u-idList$','^image$+'], count: 0 }
    ];
    //^base_user_type$+
    // Selected tab is first on in the group
    this.state = {
      selectedTabNumber: 0,
      selectedTabTypes: allTabs[0].types,
      allTabs,
    };
  }

  componentDidMount = () => {
      if(this.props.tabs){
          this.setState({allTabs: this.props.tabs})
      }else{
        this.loadTabInfo().then(allTabs => this.settingUpTabs(allTabs));
      }
  }

  settingUpTabs = (allTabs) => {
            const {selectedTabNumber} = this.state
            //if previous selected folder has
            //count of zero and is not displayed
            if(allTabs[selectedTabNumber].count === 0){
                const leavingTab = allTabs[selectedTabNumber];
                const enteringTab = allTabs[0];
                this.props.handleTabChange(leavingTab,enteringTab);
                this.setState({ allTabs,
                                selectedTabNumber: 0,
                                selectedTabTypes: allTabs[0].types
                              })
            }else{
                this.setState({ allTabs })
            }
  }

  ////////////////////////////
  /// Refreshes Tab Detail ///
  ////////////////////////////
  loadTabInfo = () => {
    return new Promise((resolve,reject) => {

      // remove counts
      let allTabs = clone(this.state.allTabs)
      allTabs.forEach((tab) => {
        delete tab.count
      })

      const parents  = {
        "parents": [ this.props.scrFolder ],
        "show_other": false,
        "rtypes": allTabs
      }

      let parameters = {
        cmdr: "objcount" ,
        parse: JSON.stringify(parents)
      }

      let request = new RequestConstructor({parameters});

      request.handleFetch()
        .then(response => response.json())
        .then(json => {
                    const allTabs = json[0].type_count;
                    resolve( allTabs );
        })
        .catch(error => console.error('LoadTabInfo: ' + error));
    })
   }

   componentDidUpdate = (prevProps,prevState) => {
      if(prevProps.scrFolder !== this.props.scrFolder && !this.props.tabs){
         this.loadTabInfo().then(allTabs => this.settingUpTabs(allTabs));
      }
   }

   onTabChange = (activeKey) => {

      const {allTabs,selectedTabNumber} = this.state
      const leavingTab = allTabs[selectedTabNumber];
      const enteringTab = allTabs[activeKey];
      this.props.handleTabChange(leavingTab,enteringTab);

      // Upload new tab info on tabChange
      this.loadTabInfo().then(allTabs => this.setState({
          allTabs,
          selectedTabTypes: enteringTab.types,
          selectedTabNumber: activeKey
      }));
    }
    renderTabs = (allTabs) => {
        const tabs = allTabs.map((tab, index) => {
                                                    if(tab.count === 0){
                                                        return null;
                                                    }else if(!tab.count){
                                                        return(
                                                               <TabPane
                                                                    tab={<span>{tab.title}</span>}
                                                                    disabled={false}
                                                                    key={index}
                                                                >
                                                                </TabPane>
                                                              )
                                                    }else if(tab.count > 0){
                                                         return(
                                                                 <TabPane
                                                                    tab={<span>{tab.title} <p className="tab-info"> {tab.count} </p></span>}
                                                                    disabled={false}
                                                                    key={index}
                                                                 >
                                                                 </TabPane>)
                                                    }
                                                    return null;
                                               })
        return tabs;
    }


  render(){
      return(
          <>
              <Tabs
                 defaultActiveKey={`${this.state.selectedTabNumber}`}
                 onChange={this.onTabChange}
                 animated={false}
                 type="card"
               >
                {this.renderTabs(this.state.allTabs)}

              </Tabs>
          </>
      );
  }



}

export default TabContainer;