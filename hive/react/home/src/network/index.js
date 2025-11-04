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
import React,{Component} from 'react';
import { HeaderContainer } from '../hivelib/Header/index';
import controllers from '../hivelib/controller/controller_collector.js';
import { ProjectToolbar } from '../hivelib/project/ProjectToolbar.js';

import  {NetworkTree}  from "./js/network-tree_container.js";
import { NetworkForm } from "./js/newtork-form_container.js";
import {RequestToAddNetwork} from "./js/button-request-network_container"
//import './css/layouts.css'
//import './css/custom.css'

let { PublishSubscribe } = controllers;

const withConrols = (WrappedComponent) => {
    class HOC extends Component {
        constructor (props){
            super(props);
            this.props = props
            this.keysToURL = [];
            this.record = {}

        }
        onCheck = (checkedKeys , e ) => {
            checkedKeys = [...checkedKeys]
            this.record = {}
            // URLs to add
            checkedKeys.forEach((element,i) => {
                let arr = element.split('/')
                if(arr[arr.length-1] === ""){
                arr.splice(arr.length-1,1)
                }
                if(!this.record[arr.length]){
                this.record[arr.length] = []
                }
                this.record[arr.length].push(i)
            });

            let keys = Object.keys(this.record)
            keys.forEach((key,i)=>{
                this.record[key].forEach((item_a,j) => {
                let a = checkedKeys[item_a]
                for(let y = keys.length - 1; y >= 0 ; y--){
                    let pathCluster = this.record[keys[y]]
                    for(let z = 0; z < pathCluster.length ;){
                        let item_b = pathCluster[z];
                        let b = checkedKeys[item_b]
                        if(b.indexOf(a) === 0 && b !== a){
                            pathCluster.splice(z,1);
                            z = 0
                        } else {
                            z++
                        }
                    }
                }
                })
            })

            let URLtoRecord = []
            keys.forEach((key)=> {
                this.record[key].forEach((index)=>{
                URLtoRecord.push(`hive://${checkedKeys[index]}`)
                })
            })
            this.props.pubsub.publish('urladd', URLtoRecord);
        }
        render(){
            return( <WrappedComponent onCheck={this.onCheck.bind(this)} {...this.props} /> )
        }
    };

    return HOC;
};
const NetworkTreeWithControlls = withConrols(NetworkTree);


export class Network extends Component {
constructor(){
    super();
    this.state = {
        applicationKey: "0", // incremented whenever project is changed, thereby refreshing
                             // "application" to initial state
    };
    this.pubSub = new PublishSubscribe();
}
render(){
    return(
        <div className="header-container-layout">
            <HeaderContainer headerclass={"header-container-layout_header"} />
            <div className="project">
                    <ProjectToolbar
                        onProjectSwitch={() => {
                            const newKey = parseInt(this.state.applicationKey) + 1;
                            this.setState({applicationKey: newKey.toString()});
                        }}
                    />
                </div>
            <div style={{margin: '0 auto'}} className="header-container-layout_container">
                <div style={{display:'flex', justifyContent:'flex-start', alignItems: 'stretch', height:'100%', position: 'absolute', left: '0', right: '0'}}>
                    <div style={{width: "600px", backgroundColor:'#3340610f', padding:'30px'}}>
                        <NetworkForm key={this.state.applicationKey} pubsub={this.pubSub}/>
                    </div>
                    <div style={{marginLeft: "30px", padding:'30px'}}>
                        <h1>HIVE Network</h1>
                        <h4>Choose items to upload to your home page</h4>
                        <br/>
                        <RequestToAddNetwork
                            buttonSize="small"
                            buttonIcon='folder-add'
                            buttonName='Add Storage Location'
                            key={"reqAdd".concat(this.state.applicationKey)}
                        />
                        <NetworkTreeWithControlls
                            pubsub={this.pubSub}
                            key={"tree".concat(this.state.applicationKey)}/>
                    </div>
                </div>
            </div>
        </div>
    )
}
}
