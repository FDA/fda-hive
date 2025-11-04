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
import { Tree, Icon } from "antd";
import "antd/dist/antd.css";

import { CustomRequest as RequestConstructor } from "../../modal/request_modal";

import { sortAlphAndCount, filterDuplicatesArr } from "./../../modal/sort-filter_modal";
import { setUpActionList } from "./../modal/action_modal";
import { filterTree } from "../modal/tree_modal";
import { gClip } from "./../controller/buttonUrlController";
import { ButtonRow } from "./action_container";
import { SearchContainer } from "./search_container";
import { handleNotifications } from "./../controller/notification_controller";
import { OpenNotificationWithIcon } from "./../view/notification_view";

const { TreeNode } = Tree;


class TreeList extends PureComponent {

  constructor(props) {
    super(props);
    this.state = {
      treeDataChildren: this.props.treeData,
      initialFolder: "",
      expandedKeys: [],
      searchValue: null,
      hightlightFolders: null,
      brHidden: false
    };
  }

  componentDidUpdate(prevProps) {
    let reload = false;
    let treeDataChildren = this.state.treeDataChildren;
    let initialFolder = this.state.initialFolder;

    if (this.props.treeData !== prevProps.treeData) {
      reload = true;
      treeDataChildren = this.props.treeData;
    }
    if (this.props.initialFolder !== prevProps.initialFolder) {
      reload = true;
      initialFolder = this.props.initialFolder;
    }
    if (this.props.tableItemSelect !== prevProps.tableItemSelect) {
      if (prevProps.tableItemSelect !== null && prevProps.tableItemSelect !== undefined) {
        this.dltHighlightFolder(false);
      }
      if (this.props.tableItemSelect !== null && this.props.tableItemSelect !== undefined) {
        this.onLoadExpandedKeys(this.props.tableItemSelect);
      }
    }

    if (reload) {
      this.setState({
        treeDataChildren,
        initialFolder
      })
    }
  }

  onLoadSearched = (value) =>
    new Promise(resolve => {
      let parameters = {
        cmdr: "objQry",
        qry: `pars = dic();
              p1 = function (c, d) {
                  if (pars[c] as int) { pars[c].push(d); } else { pars[c] = [d]; }
              };
              p = function (d) {
                  (d as obj).child.foreach({ p1(this, d) });
              };
              f = function (c, r) {
                  pars[c].foreach({ f(this, r) });
                  r.push(c);
              };
              z = dic();
              alloftype("^directory$+").foreach({ p(this) });
              alloftype("^directory$+", { name: { value: "${value}", method: "substring" } }).foreach({ z[this] = []; f(this, z[this]); });
              return z;  `
      }

      let request = new RequestConstructor({ parameters });

      (async () => {
        request.handleFetch()
          .then( (response) => {
            if (response.status === 200) {
              return response.json();
            }
          }).then( (data) => {
            //items has to stay like this Object.keys(data)
            let keys = Object.values(data).flat()
            let expandedKeys = filterDuplicatesArr(keys);
            expandedKeys = expandedKeys.map(item => item.toString())
            this.setState({
              expandedKeys,
              searchValue: value
            })
            resolve();
          })
      })().catch( (error) => console.error);
    });


  onLoadExpandedKeys = (key) =>
    new Promise( (resolve) => {
      let parameters = {
        cmdr: "objQry",
        qry: `pars = dic();
              p1 = function (c, d) {
                  if (pars[c] as int) { pars[c].push(d); } else { pars[c] = [d]; }
              };
              p = function (d) {
                  (d as obj).child.foreach({ p1(this, d) });
              };
              f = function (c, r) {
                  pars[c].foreach({ f(this, r) });
                  r.push(c);
              };
              z = [];
              alloftype("^directory$+").foreach({ p(this) });
              f(${key}, z);
              return z;`
      }

      let request = new RequestConstructor({ parameters });

      (async () => {
        request.handleFetch()
          .then((response) => {
            if (response.status === 200) {
              return response.json();
            }
          }).then((arr) => {
            let hightlightFolders = [];
            let expandedKeys = []

            arr.forEach((item, i, ar) => {
              if (key !== item) {
                expandedKeys.push(item);
              } else {
                hightlightFolders.push(ar[i - 1])
              }
            });

            expandedKeys = filterDuplicatesArr(expandedKeys).map((item) => item.toString());

            this.setState((state, props) => {
              return {
                selectedRowKeys: props.tableItemSelect,
                expandedKeys,
                hightlightFolders
              }
            })
            resolve();
          })
      })().catch((error) => console.error);
    });

  ////////////////////////////
  /// Load More Tree Items ///
  ////////////////////////////
  onLoadData = treeNode =>
    new Promise(resolve => {
      let currentComponent = this;
      if (!treeNode.props.isLeaf) {
        const treeNodeData = treeNode.props.dataRef;
        const id = treeNodeData.key;
        const title = treeNodeData.title;

        let parameters = {
          cmdr: "objList",
          mode: 'json',
          parP: 'child',
          actions: '1',
          perm: '1',
          prop: 'name,type_count',
          parIds: id,
          type: '^folder$'
        }

        if (title === "Trash") { parameters['showTrashed'] = '1' }

        let request = new RequestConstructor({ parameters });

        (async () => {
          request.handleFetch()
            .then(response => response.json())
            .then(json => json.objs)
            .then((newData) => {
              newData = sortAlphAndCount(newData, 'name')
              newData = filterTree(newData);

              if (!this.props.initialFolder.perm && (id).toString() === (this.props.initialFolder.key).toString()) {
                // initialFolder coming from cookies lacks a lot of properties
                this.setState({
                  selectedTitle: treeNodeData.title,
                })
                this.props.onRepopulateInitialFolder(treeNodeData)
              } else if (!this.props.initialFolder.perm) {
                newData.forEach((el) => {
                  if (!this.props.initialFolder.perm && ((el.key).toString() === (this.props.initialFolder.key).toString())) {
                    // initialFolder coming from cookies lacks a lot of properties
                    this.setState({
                      selectedTitle: el.title,
                    })
                    this.props.onRepopulateInitialFolder(el)
                  }
                })
              }


              // 1. see if treeNode key is eqaul to srcFolder id
              if (id === this.state.srcFolder) {
                const keys = newData.map((el) => el.key);
                const current_key = parseInt(currentComponent.state.selectedRowKeys);

                // 2. if equals check for if this node has the selected key
                if (keys.indexOf(current_key) === -1) {
                  const new_slc_node = currentComponent.state.srcFolder;
                  let tree = currentComponent.state.treeDataChildren;
                  tree = { key: '', children: tree }
                  //3. if treeNode is deleted set the selection to its parent
                  this.findTreeNode(tree, new_slc_node);
                }
              }

              return newData;
            })
            .then((res) => {
              treeNodeData.children = res;

              //recording parent of each node
              treeNodeData.children.map(el => el.parent = id);
              currentComponent.setState({
                treeDataChildren: [...currentComponent.state.treeDataChildren]
              });

              resolve();
            })

        })().catch((error) => console.error);
      }
    });

  findTreeNode = (tree, node) => {
    if (tree.children) {
      for (let i = 0; i < tree.children.length; i++) {
        if (tree.children[i].key === node) {
          this.setSelection(tree.children[i]);
          return;
        } else {
          this.findTreeNode(tree.children[i], node);
        }
      }
    }
  }
  //////////////////
  /// Open table ///
  //////////////////
  onSelect = (selectedKeys, title) => {
    if (this.props.pageLoading) return;

    const selected = title.node.props.dataRef;
    this.setSelection(selected);
  }

  onExpand = (expandedKeys) => {
    expandedKeys = filterDuplicatesArr(expandedKeys);
    this.setState({ expandedKeys })
  }

  setSelection = (selected) => {
    this.setState({
      actionList: setUpActionList(this.props.actionsData, [selected]),
      selectedRowKeys: selected.key,
      selectedType: selected.type,
      selectedTitle: selected.title,
      srcFolder: selected.parent ? selected.parent : 0,
      brHidden:false
    })
    this.props.onTreeItemSelect(selected);
  }

  dltHighlightFolder = (setToNull = true) => {
    let allHighlights = document.querySelectorAll(".sidenav-tree__note");
    if (setToNull) {
      this.setState({ hightlightFolders: null });
    }
    allHighlights.forEach((item) => {
      item.classList.remove('sidenav-tree__note')
    })
  }

  renderTreeNodes = (data) => {
    const treeNodes = data.map((item) => {
      // SEARCH Highlight
      const { searchValue, hightlightFolders } = this.state;
      let beforeStr, afterStr, insideStr;
      let index = -1;
      let prolongedLoading = false;

      if(item.title && item.title.indexOf(' ---')>-1) {
        prolongedLoading = true;
        item.title = item.title.replace(' ---', '');
      }

      if (searchValue) {
        let partOfExpanded = !(item.key === undefined || item.key === null ||  item.key === "") ? this.state.expandedKeys.indexOf(item.key.toString()) : -1;
        if (item.title !== undefined && partOfExpanded > -1) {
          index = item.title.toLowerCase().indexOf(searchValue.toLowerCase());
          beforeStr = item.title.substr(0, index);
          afterStr = item.title.substr(index + searchValue.length);
          insideStr = item.title.slice(index,index + searchValue.length)
        }
      }


      let title = searchValue && index > -1
        ? (
          <span>
            {beforeStr}
            <span style={{ color: "#eaba3a" }}>{insideStr}</span>
            {afterStr}
          </span>
        )
        : ( <span> {item.title} </span> );

      if (hightlightFolders instanceof Array && hightlightFolders.indexOf(item.key) > -1) {
        title = <span className="sidenav-tree__note">{title}</span>
      }

      if (item.children && item.children.length > 0) {
        return (
          <TreeNode
            title={item.title ? title : item.key}
            key={item.key}
            dataRef={item}
            icon={({ expanded, selected }) => <Icon
                                                type={expanded || selected ? 'folder-open' : 'folder'}
                                                theme={item.isEmpty ? 'outlined' : 'filled'}
                                              />
                  }
          >
            {this.renderTreeNodes(item.children)}
          </TreeNode>
        );
      }

      let icon = ({ selected }) => <Icon type={selected ? 'folder-open' : 'folder'} theme={item.isEmpty ? 'outlined' : 'filled'}/>;
      if(prolongedLoading) {
        icon = <Icon type='loading' theme='outlined' />;
      }

      return (
        <TreeNode
          {...item}
          title={item.title ? title : item.key}
          dataRef={item}
          icon={icon}
        />
      );
    });
    return treeNodes;
  }

  //////////////////
  /// Drag + Drop///
  onDragStart = (info) => {
    if(info.node.props.dataRef.type === "sysfolder") {
      info.event.dataTransfer.effectAllowed = "none"
    }
  }

  onDragLeave = (info) => {
    info.event.preventDefault();
    const dstFolder = info.node.selectHandle;
    const classes = dstFolder.classList;
    classes.remove('allow', 'read-only')
  }

  // NOTE onDragEnter fires once but doesn't work with tree
  // NOTE this fires mutiple times but works with tree
  onDragOver = (info) => {
    let write = info.node.props.dataRef.perm.act.write;
    write =  info.event.dataTransfer.effectAllowed === "none" ? false : write;
    const dataTransfer = info.event.dataTransfer;
    !write ? dataTransfer.dropEffect = "none" : dataTransfer.dropEffect = "move";

    // Prevent Defualt needed to stop fetching urls
    info.event.preventDefault();
    // return false;
    const classes = info.node.selectHandle.classList;

    if (classes.contains('allow') || classes.contains('read-only')) {
      return;
    }

    !write ? classes.add('read-only') : classes.add('allow');
  }

  onDrop = info => {
    //event.preventDefault;
    const { selectedRowKeys, initialFolder } = this.state;
    const dstFolder = info.node.selectHandle;
    const classes = dstFolder.classList;
    classes.remove('allow', 'read-only')

    // Check if item can be dropped into the folder
    if (!info.node.props.dataRef.perm.act.write ||  info.event.dataTransfer.effectAllowed === "none") { return; }

    // Where
    const dropKey = info.node.props.eventKey;
    const dropPos = info.node.props.pos.split('-');

    let dragKey, dragKeyParent;
    // What

    if (info.dragNode) {
      dragKey = info.dragNode.props.eventKey;
      dragKeyParent = info.dragNode.props.parent;

      // Check if folder dropped into is itself
      if (dragKey === dropKey) { return; }

      let notitication_pros = {
        type: 'warning',
        message: 'Moving',
        key: dragKey,
        icon: (<Icon type="loading" />),
      }
      OpenNotificationWithIcon(notitication_pros);

      gClip.cut(dragKeyParent, dragKey);
      gClip.paste(dropKey, "dcls").then((result) => {

        handleNotifications(result, "Drop", dragKey);
        if (result.status === 200) {
          const dropPosition = info.dropPosition - Number(dropPos[dropPos.length - 1]);
          const loop = (data, key, callback) => {
            data.forEach((item, index, arr) => {
              if (item.key === key) {
                return callback(item, index, arr);
              }
              if (item.children) {
                return loop(item.children, key, callback);
              }
            });
          };
          const data = [...this.state.treeDataChildren];

          // Find dragObject
          let dragObj;
          loop(
            data,
            dragKey,
            (item, index, arr) => {
              arr.splice(index, 1);
              dragObj = item;
            }
          );

          if (!info.dropToGap) {
            // Drop on the content
            loop(
              data,
              dropKey,
              (item) => {
                item.children = item.children || [];
                item.children.push(dragObj); // where to insert
              }
            );
          } else if (
            (info.node.props.children || []).length > 0 // Has children
            &&  info.node.props.expanded  // Is expanded
            && dropPosition === 1 // On the bottom gap
          ) {
            loop(
              data,
              dropKey,
              (item) => {
                item.children = item.children || [];
                item.children.unshift(dragObj); // where to insert
              }
            );
          } else {
            let ar;
            let i;
            loop(
              data,
              dropKey,
              (item, index, arr) => {
                ar = arr;
                i = index;
              }
            );

            if (dropPosition === -1) {
              ar.splice(i, 0, dragObj);
            } else {
              ar.splice(i + 1, 0, dragObj);
            }
          }
          this.props.reload(true);
        } else {
           // SHOW MICRO INTERACTION THAT SOMETHING WENT WRONG
        }

      }).catch((error) => console.log(error));
    } else {
      dragKey = info.event.dataTransfer.getData("text");
      info.event.dataTransfer.clearData();
      // Check if folder dropped into is itself
      if (dragKey === dropKey) { return; }

      // Add notification props
      OpenNotificationWithIcon({
        type: 'warning',
        message: 'Moving',
        key: dragKey,
        icon: (<Icon type="loading" />),
      });

      dragKeyParent = selectedRowKeys || selectedRowKeys === 0
                        ? selectedRowKeys
                        : initialFolder.key;

      gClip.cut(dragKeyParent, dragKey);
      gClip.paste(dropKey, "dcls").then((result) => {
        handleNotifications(result, "Drop", dragKey);
        if (result.status === 200) {
          this.props.reload(true);
        } else {
          // SHOW MICRO INTERACTION THAT SOMETHING WENT WRONG
        }
      })

    }
  };

  onSearchChange = (value) => {
    //e.preventDefault()
    if (value.length > 0) {
      this.onLoadSearched(value);
    } else {
      this.setState({
        searchValue: null,
      })
    }
  }

  handleclearRowKeys = (clear) => {
    if (clear) {
      this.props.reload(clear)
    }
  }
  render() {
    const { selectedRowKeys, initialFolder } = this.state;
    const defSelKey = initialFolder ? initialFolder.key : null;
    const selKey = selectedRowKeys || selectedRowKeys === 0 ? selectedRowKeys : defSelKey;

    return (
      <div style={{ height: 'inherit', flex: '1 1', display: 'flex', flexDirection: 'column' }}>
        <div className="sidenav-actions">
          <SearchContainer
            className="sidenav-search sidenav-search-inrow"
            handleSearch={this.onSearchChange}
            placeHolder="Search Folder"
            onClick={()=> this.setState({brHidden:true})}
          />
          <ButtonRow
            style={{display: this.state.brHidden ? 'none' : 'inline-block'}}
            actionList={this.state.actionList}
            reload={(reload) => {
              this.dltHighlightFolder()
              this.props.reload(true)
            } }
            selectedTitle={this.state.selectedTitle}
            selectedRowKeys={[this.state.selectedRowKeys]}
            clearRowKeys={this.handleclearRowKeys}
            type={[this.state.selectedType]}
            srcFolder={this.state.srcFolder}
          />

        </div>
        <div className="sidenav-tree" style={{ overflow: 'auto' }}>
          {
            this.props.treeData.length
            ? (
                <Tree
                  showIcon
                  showLine={{ showLeafIcon: false }}

                  switcherIcon={<Icon type="down" />}
                  ///////////////////////////////////
                  /// Open Inbox as initial TABLE ///
                  selectedKeys={[`${selKey}`]}
                  loadedKeys={[]}
                  expandedKeys={this.state.expandedKeys}
                  onSelect={this.onSelect}
                  onExpand={this.onExpand}
                  loadData={this.onLoadData}
                  //////////////////
                  /// Drag + Drop///
                  className="draggable-tree"
                  draggable={this.props.isDraggable}
                  onDragStart={event => this.onDragStart(event)}
                  onDragOver={event => this.onDragOver(event)}
                  onDragLeave={event => this.onDragLeave(event)}
                  onDrop={event => this.onDrop(event)}
                >
                  {this.renderTreeNodes(this.state.treeDataChildren)}
                </Tree>
              )
            : ('loading tree')
          }
        </div>
      </div>
    );

  }
}
export default TreeList;