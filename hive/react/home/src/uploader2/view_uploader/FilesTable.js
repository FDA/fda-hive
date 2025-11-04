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
import React from 'react';
import { Tooltip  , Button , Tree , Icon, Pagination, Spin } from 'antd';
import filesize from "filesize";
import { connect } from 'react-redux';
import {
    handleProgress,
    removeKeyFileList,
    removeFileListByPath,
    subFileList,
    loadingFiles
} from '../actions';

const { TreeNode } = Tree;
//props title
const pathListToTree = (myData) => {
    const treeConstruct = {
        name: '/',
        children: [],
        size: 0
    }
    // Structure Tree
    myData.forEach(el => {
        // 1.Split path
        let path = el.uploadPath.split('/');

        // 2b.Loop over each item of the path cheching whether it's a part of the tree
        var x = treeConstruct;
        for (let i = 0; i < path.length; i++) {
            let item = x.children.find(item => item.name === path[i]);
            if (!item) {
                if (i === path.length - 1) {
                    const item1 = {
                        name: path[i],
                        title: path[i],
                        key: el.key,
                        size: el.size || null,
                        isLeaf:true
                    }
                    x.children.unshift(item1);
                    if (el.size) { treeConstruct.size = treeConstruct.size + el.size}
                    x = item1;
                } else {
                    const item1 = {
                        name: path[i],
                        title: path[i],
                        key:`${path.slice(0,i+1).join('/')}`,
                        folder: true,
                        size: 0,
                        children: []
                    }
                    x.children.unshift(item1);
                    x = item1;
                }
            } else {
                x = item;
                if (i === path.length - 1) {
                    // This is an empty folder
                    // Edit path proporties
                    x.name = path[i];
                    x.title = path[i];
                    x.path = path.slice(0,path.length-1);
                    x.key = el.key;
                }
                // else if (x.children){
                //     x.size = x.children.reduce((totalSize,cur) => {
                //         let size = cur.size ? cur.size : 0;
                //         return totalSize + size;
                //     },0)
                // }

            }
        }
    });
    return treeConstruct;
}

class FilesTable extends React.PureComponent{
    constructor(props){
       super(props)
       this.props = props
       this.state={
           filesList: [],
           totalSize: 0
       }
       this.treeSelectedKeys = []
    }

    componentDidUpdate(prevProps) {
        if(prevProps.filesList.length !== this.props.filesList.length){
            this.filesList = [...this.props.filesList]
            let { size , children } = pathListToTree([...this.filesList])
            let tree = children
            this.setState({filesList: tree , totalSize: size })
            this.props.loadingFiles(false) // For Files Tree component
        }
    }

    render(){
        return(
            <>
                <Spin tip="Loading..." spinning={this.props.loadingFilesState}>
                <Button
                    size={'small'}
                    icon='delete'
                    style={{display: this.props.filesList.length ? 'inline-block' : 'none', marginRight:'25px'}}
                    onClick={async ()=>{
                        await this.props.subFileList([])
                    }}
                    disabled={this.props.uploadState === 'disabled' ||
                            this.props.uploadState === 'idle' ||
                            this.props.uploadState === 'error' ? false : true
                            }
                >
                    remove all
                </Button>
                <em style={{display: this.state.totalSize ? 'inline-block' : 'none', marginRight:'25px' }}> {filesize(this.state.totalSize)}</em>
                <em style={{display: this.props.filesList ? 'inline-block' : 'none'}}>{this.props.filesList.length} items</em>
                <br/>
                <FilesTreeAb
                    fileModal={this.props.fileModal}
                    dataSource={this.state.filesList}
                />
                </Spin>
            </>
            );
    }
}

class FilesTreeAb extends React.PureComponent{
    constructor(props){
       super(props)
       this.props = props
       this.state ={ checkedKeys:[] }
       this.pagers = new Map()
       this.pagers.set('uploader://',[0,10,1,10])
    }

    handlePager = (folder, args) => {
        const [page,pageSize] = args;
        let finish = page*pageSize > folder.children.length ? folder.children.length : page*pageSize
        let pager = [(page - 1) * pageSize, finish ,page,pageSize]
        this.pagers.set(folder.key,pager)
        this.forceUpdate() // we change something that is deeply nested in props we need to reload
    }

    renderTreeNodes = (data, folder) =>{
        if(data.length === 0) return data;
        if(folder.children.length > 5 && !this.pagers.has(folder.key)) {
            this.pagers.set(folder.key, [0,5,1,5])
        }
        let pager = this.pagers.get(folder.key)
        let start = pager ? pager[0] : 0;
        let finish = pager && pager[1] <= data.length ? pager[1]  : data.length ;
        let tree = []

        // vvv Solution to the problem when treenode is not rendered due to paging and checked state is lost
        // for(let i = 0; i < data.length ; i++){
        //     let visibility = i >= start && i < finish ? {} : {display:'none'};

        for(let i = start; i < finish ; i++){
            let item  = data[i];
            if(item.children){
                tree.push(
                <TreeNode
                    data-li
                    selectable={true}
                    className='uploader--form--files--li'
                    title={<span className='uploader--form--files--title' >
                                <span>
                                    <Tooltip overlayClassName='uploader--form--files--title_tooltip' placement="topLeft" title={item.title}>
                                        <span className='uploader--form--files--title_text' >{item.title}</span>
                                    </Tooltip>
                                </span>
                                {item.size ? <sup style={{top:'0.5em', color:'#666', fontSize: '90%'}}><i>{` ${filesize(item.size )}`}</i></sup> : ''}
                                <Button
                                    className='uploader--form--files--btn_remove'
                                    type='link'
                                    data-action-remove
                                    disabled={this.props.uploadState === 'disabled' ||
                                           this.props.uploadState === 'idle' ||
                                           this.props.uploadState === 'error' ? false : true
                                          }
                                > remove </Button>
                          </span>}
                    icon={({ expanded }) => <Icon
                                                style={{color:'#48567b'}}
                                                type={ expanded ? 'folder-open' : 'folder'}
                                                theme={item.isEmpty ? 'outlined' : 'filled'}
                                            />
                            }
                    key={item.key}
                    dataRef={item}
                >
                    {this.renderTreeNodes(item.children , item)}
                </TreeNode>
                );
            }else{
                tree.push(<TreeNode
                            className='uploader--form--files--li'
                            data-li
                            selectable={true}
                            key={item.key}
                            {...item}
                            dataRef={item}
                            title={ <span className='uploader--form--files--title' >
                                        <span  className='uploader--form--files--title_text'>
                                            <Tooltip overlayClassName='uploader--form--files--title_tooltip' placement="topLeft" title={item.title}>
                                                <span>{item.title}</span>
                                            </Tooltip>
                                        </span>
                                        {item.size && <sup style={{top:'0.5em', color:'#666', fontSize: '90%'}}><i>{` ${filesize(item.size )}`}</i></sup>}
                                        <Button  className='uploader--form--files--btn_remove' type='link' data-action-remove disabled={this.props.uploadState === 'disabled' ||
                                           this.props.uploadState === 'idle' ||
                                           this.props.uploadState === 'error' ? false : true
                                          }> remove </Button>
                                    </span>}
                            icon={<Icon
                                    style={{color:'#48567b'}}
                                    type={'file'}
                                    theme={'outlined'}
                                  />}
                            />)
            }
        }
        if(pager && 5 < data.length){
            tree.push( <TreeNode
                                key={`${folder.key}-pager`}
                                checkable={false}
                                switcherIcon={<></>}
                                selectable={false}
                                icon={<></>}
                                className='uploader--form--files--pager'
                                title={<Pagination
                                            size="small"
                                            showSizeChanger
                                            pageSizeOptions={['5','10', '15','20','50','100','1000','10000']}
                                            total={data.length}
                                            current={pager[2]}
                                            onShowSizeChange={(...args) => this.handlePager(folder,args)}
                                            onChange={(...args) => this.handlePager(folder,args)}
                                            defaultPageSize={pager[3]}
                                        />}
                            ></TreeNode>)
        }
        return tree
    }

    render() {
        return (
            <>
                <Tree
                    showIcon
                    switcherIcon={<Icon type="down" />}
                    checkable={false}
                    onSelect={(selected,event) => {
                        try{
                            if(event.nativeEvent.target.dataset.hasOwnProperty('actionRemove')){
                                let li_element = event.nativeEvent.composedPath().find((el) => el.dataset.hasOwnProperty('li') )
                                if(li_element){
                                    li_element.style.display = 'none'
                                }
                                if(selected[0].indexOf('file://') === 0){
                                    let name = selected[0].replace('file://','')
                                    this.props.fileModal.removeFileFromSet(name)
                                    // delete just one
                                    this.props.removeKeyFileList(selected)
                                }else{
                                    this.props.fileModal.removeDirFromSet(selected[0])
                                    //remove from pager
                                    this.pagers.delete(selected[0])
                                    // delete anything that begins with
                                    this.props.removeFileListByPath(`file://${selected[0]}/`)
                                }
                            }
                        }catch(error){
                            console.log(error)
                        }
                    }}
                    blockNode={true}
                    selectable={true}
                    expandAction={false}
                >
                    {this.renderTreeNodes(this.props.dataSource, {key: 'uploader://' ,children:this.props.dataSource})}
                </Tree>
          </>
        );
      }
}

const mapStateToProps = (state) => {
        return {
            filesList: state.filesList,
            uploadState: state.uploadState,
            loadingFilesState: state.loadingFiles
        };
}
const mapStateToPropsFileTree = (state) => {
    return {
        uploadState: state.uploadState,
    };
}
const mapDispatchToProps = dispatch => ({
    removeKeyFileList: i => dispatch(removeKeyFileList(i)),
    removeFileListByPath: i => dispatch(removeFileListByPath(i))
})
FilesTreeAb = connect(mapStateToPropsFileTree,mapDispatchToProps)(FilesTreeAb);
export default connect(mapStateToProps,{subFileList,handleProgress,loadingFiles })(FilesTable);