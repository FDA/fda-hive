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
import { Tree , Spin , Result , Icon, Button , Pagination} from 'antd';
import "antd/dist/antd.css";
import Bottleneck from "bottleneck";
import shortid from "shortid";
import modals from '../../hivelib/modal/modal_collector';

const { url_modal ,csv_modal, request_modal } = modals;
const {getPrefix} = url_modal;
const { ParseCSVtoArray } = csv_modal;
const { CustomRequest } = request_modal;
const { TreeNode , DirectoryTree  } = Tree;

function formatBytes(bytes, decimals = 2) {
    if (bytes === '0') return '0 Bytes';

    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
}

const Error = <Result
    status="warning"
    title="There are some problems with your operation."
/>;

let addToTree = ( parent , path , iteminfo ) => {
    if(!path.length) return;
    let item = {}
    let found ;

    if(parent.children && parent.children.length){
        parent.children.forEach((child)=>{
            if(child.name === path[0]){ found = child}
        })
    }
    if(found){
        addToTree( found , path.slice(1) , iteminfo)
    } else {
        if(!parent.children){ parent['children'] = [] }
        let folder = path.length > 1 ? true
                        : path.length === 1 && parseInt(iteminfo.size) === -1 ? true
                        : false ;
        if(path.length === 1){
            item['size'] = iteminfo.size;
        }
        item['parent'] = parent;
        item['dropboxID'] = parent.dropboxID;
        item['name'] = path[0];
        item['id'] = `${parent.id}${path[0]}${ folder ? '/' : ''}`;
        item['folder'] = folder;
        addToTree(item , path.slice(1) , iteminfo)
        parent.children.push(item)
    }
}

let constructNetworkTree = (tree, array) => {
    let keys = array[0]; //id,name,size
    for(let i=1; i < array.length ; i++){
        let item = array[i];
        if( keys.includes('id') ){
            let path = item[ keys.indexOf('name')];
            let iteminfo = {
                id: item[ keys.indexOf('id') ],
                path: path,
                size:  item[ keys.indexOf('size') ]
            }

            let dropboxID = item[ keys.indexOf('id') ];
            let folder = path[path.length - 1] === '/';

            path = path[path.length - 1] === '/' ? path.slice(0,-1) : path;

            let pathItems = path.indexOf('/') === 0 ? path.slice(1).split(/(?<![\\])\//) : path.split(/(?<![\\])\//);
            // 1. find id in tree
            let found;
            tree.forEach((resource) => {
                if(resource.dropboxID === dropboxID){
                    found = true;
                    addToTree( resource , pathItems.slice(1) , iteminfo )
                }
            })

            if(!found){
                let new_resource = {}
                new_resource['dropboxID'] = dropboxID;
                new_resource['id'] = `${dropboxID}/`;
                new_resource['folder'] = folder;
                new_resource['dropboxName'] = pathItems[0];
                new_resource['name'] = pathItems[0];

                addToTree( new_resource , pathItems.slice(1) , iteminfo )
                tree.push(new_resource)
            }

        }
    }
    return tree;
}

export class NetworkTree extends Component {
    constructor(props){
        super(props)
        this.props = props;
        this.networkTree = [];
        this.state = {
            networkTree: [],
            output: [],
            loading: true,
            error: false
        }
    }
    componentDidMount(){
        const parameters = {cmdr: 'dropboxlist'}
        let request = new CustomRequest({parameters})
        request.handleFetch()
            .then( response => response.text())
            .then( text => {
                this.setupLimiter()
                //Need to parse
                let output = ParseCSVtoArray(text, ',')
                this.networkTree = constructNetworkTree(this.networkTree , output);
                this.setState({networkTree: this.networkTree , loading: false , output})
            })
            .catch(error => {
                this.setState({error: Error, loading: false})
            })
    }
    componentDidUpdate = (prevProps, prevState) => {
        if(prevState.output.join('') !== this.state.output.join('')){
            this.searchForBaseCalls([...this.state.output])
        }
    }
    renderNodeTitle = (item) =>{
        let name = item.name.toString().replaceAll('\\/' , '/')
        if(!item.hasBaseCalls && item.size === '-1'){
            return `${name}`
        }else if( item.size !== '-1' && item.size){
            return (
                <>
                {name}
                <sup style={{color:'#666', fontSize: '95%'}}><i>{` ${formatBytes(item.size )}`}</i></sup>
                </>
            )
         } else if(item.hasBaseCalls){
            return(
                <>
                {name}
                <Button style={{zIndex:'100', position: 'relative'}}type="link" target="_blank" onClick={(e) => e.stopPropagation() } href={`${getPrefix()}?cmd=dmBcl2Fastq&bclpath=dropbox://${encodeURI(item.id)}`}>Convert to FastQ</Button>
                </>
            )
         }

        return `${name} ${item.hasBaseCalls ? '| convert to FastQ': ''}`
    }
    handlePager = (folder, args) => {
        const [page,pageSize] = args;
        let finish = page*pageSize > folder.children.length ? folder.children.length : page*pageSize
        let pager = [(page - 1) * pageSize, finish ,page]
        let path = folder.id.split(/(?<![\\])\//).slice(0,-1)
        let dropbox = this.findDropbox(path[0])
        if(path.length === 1){
            dropbox['pager'] = pager
        }else{
            this.findFolder(dropbox , path.slice(1) , {pager} )
        }
        this.setState({networkTree: [...this.networkTree]})
    }
    renderTreeNodes = (data,pager,folder) =>{
        if(data.length === 0) return data;
        let start = pager ? pager[0] : 0;
        let finish = pager ? pager[1]  : data.length ;
        let treenode = []
        for(let i = 0; i < data.length ; i++){
            let visibility = i >= start && i < finish ? {} : {display:'none'}; // Solution to the problem when treenode is not rendered due to paging and checked state is lost
            let item  = data[i];
            if (item.children) {
                treenode.push(
                    <TreeNode
                        style={visibility}
                        title={this.renderNodeTitle(item) }
                        key={item.id}
                        dataRef={item}
                        icon={({ expanded, selected }) => <Icon
                                                            style={{color:'#48567b'}}
                                                            type={expanded || selected ? 'folder-open' : 'folder'}
                                                            theme={item.isEmpty ? 'outlined' : 'filled'}
                                                        />
                            }
                    >
                        {this.renderTreeNodes(item.children, item.children.length > 5 && (item.pager || [0,5,1]), item)}
                    </TreeNode>
                    );
            } else {
                treenode.push(<TreeNode
                    style={visibility}
                    title={this.renderNodeTitle(item)}
                    isLeaf={!item.folder}
                    key={item.id}
                    {...item}
                    dataRef={item}
                    icon={({ selected }) => <Icon
                                                style={{color:'#48567b'}}
                                                type={selected && item.folder ? 'folder-open' :
                                                        !selected && item.folder ? 'folder' : 'file'}
                                                theme={item.isEmpty || !item.folder ? 'outlined' : 'filled'}
                                            />
                        }
                />);
            }
        }
        if(pager){
            treenode.push( <TreeNode
                                checkable={false}
                                switcherIcon={<></>}
                                selectable={false}
                                icon={<></>}
                                title={<Pagination
                                            size="small"
                                            total={data.length}
                                            current={pager[2]}
                                            onChange={(...args) => this.handlePager(folder,args)}
                                            defaultPageSize={5}
                                        />}
                            ></TreeNode>)
        }
        return treenode;
    }
    onLoadData = treeNode =>
        new Promise(resolve => {
            if (treeNode.props.children) {
                resolve();
                return;
            }

            let dropboxID = treeNode.props.dataRef.dropboxID
            let path = treeNode.props.dataRef.id.replace(dropboxID,'')
            const parameters = {
                cmdr: 'dropboxlist',
                dropbox: dropboxID,
                path
            }
            let request = new CustomRequest({parameters})
            request.handleFetch()
                .then( response => response.text())
                .then( text => {
                    let output = ParseCSVtoArray(text, ',')
                    if(output.length === 1){
                        let dropbox = this.findDropbox(dropboxID)
                        this.findFolder(dropbox,path.slice(1).split(/(?<![\\])\//).slice(0,-1),{isEmpty:true})
                    } else {
                        this.networkTree = constructNetworkTree(this.networkTree , output);
                    }

                    this.setState({networkTree: [...this.networkTree],output})
                    resolve()
                })
                .catch(error =>{
                       //Handle if error
                })
        });

    findDropbox = (id) =>{
        for( let i = 0; i < this.networkTree.length; i++){
            let network = this.networkTree[i]
            if(network.dropboxID === id){
                return this.networkTree[i];
            }
        }
        return null;
    }
    findFolder = (parent, folders , info) =>{
            if(!parent.children || !folders.length){ return parent; }

            for(let i = 0 ; i< parent.children.length; i++){
                let kid = parent.children[i]
                if(kid.name === folders[0]){
                    if(folders.length === 1){
                        kid = Object.assign(kid,info)
                    } else {
                        this.findFolder(kid,folders.slice(1),info)
                    }
                    break;
                }
            }
        return parent;
    }

    searchForBaseCalls = (list) => {
        list.forEach((item, i)=>{
            if(i !== 0 && item[ list[0].indexOf('size') ] === '-1'){
                let treePath = item[ list[0].indexOf('name')].slice(1).split(/(?<![\\])\//).slice(1,-1)
                let searchPath = `${item[ list[0].indexOf('name')].slice(1).split(/(?<![\\])\//).slice(1).join('/')}`;
                let id = item[ list[0].indexOf('id') ];
                this.scheduleSearch(id,searchPath,treePath)
            }
        })
    }
    requestBasecall = (id,searchPath,treePath) => {
        return new Promise((resolve,reject) => {
            const parameters = {
                cmdr: 'dropboxlist',
                dropbox: id,
                search: `${searchPath}Data/Intensities/BaseCalls/*`
                }
            let request = new CustomRequest({parameters})
            request.handleFetch()
            .then( response => response.text())
            .then( text => {
                resolve(text)
            })
            .catch(error => {
                reject(error)
            })
        })

    }
    scheduleSearch = (id,searchPath,treePath) => {
        this.limiter.schedule({id: shortid.generate()}, () => this.requestBasecall(id,searchPath,treePath))
        .then( (text) => {
            let output = ParseCSVtoArray(text, ',')
            if(output && output.length > 1){
                let dropbox = this.findDropbox(id)
                this.findFolder(dropbox,treePath,{hasBaseCalls:true})
                this.setState({networkTree: [ ...this.networkTree] })
            }
        })
        .catch(error => {
            console.log('Search BaseCalls: ' , error)
        })
    }
    setupLimiter = () =>{
        this.limiter = new Bottleneck({maxConcurrent: 4 });
        this.limiter.on("error", (err) => {
            console.log('xrh error',err)
        });

        this.limiter.on("failed", async (error, jobInfo) => {
            if (jobInfo.retryCount < 100) { // Here we only retry once
                return 25;
            }
        });
    }

    render(){
        return(
            <div style={{width:'500px'}}>
                <Spin spinning={this.state.loading}>
                    <DirectoryTree
                        switcherIcon={<Icon type="down" />}
                        checkable
                        selectable={false}
                        loadData={this.onLoadData}
                        onCheck={(checkedKeys,e) => {
                            this.props.onCheck(checkedKeys,e)
                        }}
                        onExpand={this.handleExpand}
                        expandAction={false}
                        checkAction={false}
                    >
                        {this.renderTreeNodes(this.state.networkTree)}
                    </DirectoryTree>
                </Spin>
                {this.state.error}
            </div>
        )
    }
}