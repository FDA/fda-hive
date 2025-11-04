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
import React from "react";
import { Tree } from "antd";

import Papa from "papaparse";

const { TreeNode } = Tree;

const propgeturl = "cmdr=propget&mode=csv&perm=1&ids=";
//const propgeturl = "cmdr=propget&mode=csv&perm=1&actions=1&ids=";
const grplisturl = "cmdr=grpList&raw=1&grp=1";
//const usrlisurl = "cmdr=usrList&grp=0";

export default class ShareBriefView extends React.Component {
    componentDidMount() {
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;

        let url = this.urlExchangeParameter(propgeturl, "ids", this.props.ids);
        this.props.handleLoading(true);
        this.fetchData(url, "propGetData");
        this.fetchData(grplisturl, "grpListData");

    }

    componentDidUpdate(prevProps, prevState) {
        if (prevProps.ids !== this.props.ids) {
            this.handleLoadData()

        }
        if (this.props.reload !== prevProps.reload) {
            this.handleLoadData()

        }
    };

    handleLoadData = () => {
        var url = this.urlExchangeParameter(propgeturl, "ids", this.props.ids);
        this.props.handleLoading(true);
        this.fetchData(url, "propGetData").finally(() => this.props.handleLoading(false));
    }

    getPermInfo() {
        let propGetData = !this.props.propGetData ? '' : this.props.propGetData;
        let dataArr = Papa.parse(propGetData, { header: true });
        let toReturn = {}; //map user to permission

        for (var i = 0; i < dataArr.data.length; i++) {
            if (dataArr.data[i].name === "_perm") { //looking at permissions
                let rowVal = dataArr.data[i].value;
                let parsedRowVal = Papa.parse(rowVal, { delimiter: "," });

                let usr = parsedRowVal.data[0][0]; //this is the user id
                let permParse = Papa.parse(parsedRowVal.data[0][2]);
                let perms = permParse.data[0];
                let direction = parsedRowVal.data[0][3];

                toReturn[usr] = { perms: perms, dir: direction };
            }
        }
        return toReturn;
    }

    getGroupInfo() {
        let grpListData = !this.props.grpListData ? '' : this.props.grpListData;
        let dataArr = Papa.parse(grpListData, { header: true });
        let toReturn = {}; //will map user to their name and path(not needed for brief view)

        for (var i = 0; i < dataArr.data.length; i++) {
            let usrid = dataArr.data[i].id;
            toReturn[usrid] = { name: dataArr.data[i].name, path: dataArr.data[i].path };
        }
        return toReturn;
    }

    generateTree(permInfo, groupInfo) {
        if (Object.keys(permInfo).length === 0 || Object.keys(groupInfo).length === 0) return;

        let finalTree = { name: "root", children: [] };

        for (var usr in permInfo) {
            let usrInfo = groupInfo[usr];
            if (!usrInfo) continue;
            let path = usrInfo.path;
            let leafVal = { name: usrInfo.name, usrId: usr, perms: permInfo[usr] };
            let parsedPath = Papa.parse(path, { delimiter: "/" });
            if (parsedPath.data === 0) return;

            parsedPath.data[0].splice(0, 1)
            this.fixTree(finalTree.children, parsedPath.data[0], leafVal);
        }
        return finalTree;
    }

    fixTree(treeChildArr, path, leaf) {
        if (path.length === 0) return;

        const curNode = path[0];
        path.splice(0, 1);

        let i;
        for (i = 0; i < treeChildArr.length; i++) {
            if (treeChildArr[i].name === curNode) break;
        }

        if (i === treeChildArr.length) treeChildArr.push({ name: curNode, children: [] });

        if (path.length === 0) {
            treeChildArr[i].info = leaf;
            return;
        }

        this.fixTree(treeChildArr[i].children, path, leaf);
    }

    renderTreeNodes(node) {
        if (node === undefined) return;

        if (!node.children || node.children.length === 0) { //leaf node
            let nodeText = node.info.name + ": can ";
            for (let i = 0; node.info.perms && node.info.perms.perms && i < node.info.perms.perms.length; i++) {
                if (i === node.info.perms.perms.length - 1) nodeText += node.info.perms.perms[i];
                else nodeText += node.info.perms.perms[i] + ", ";
            }

            return (<TreeNode title={nodeText} key={node.name}>
            </TreeNode>);
        }
        return (<TreeNode title={node.name} key={node.name}>
            {node.children.map((key, i) => {
                return this.renderTreeNodes(key);
            })}
        </TreeNode>);
    }

    render() {
        let permInfo = this.getPermInfo();
        let groupInfo = this.getGroupInfo();

        let treeToDraw = this.generateTree(permInfo, groupInfo);
        if (!treeToDraw) return (<div> Share View </div>);

        return (
            <Tree defaultExpandAll={true}>
                {this.renderTreeNodes(treeToDraw.children.length > 1 ? treeToDraw : treeToDraw.children[0])}
            </Tree>
        );
    }
}