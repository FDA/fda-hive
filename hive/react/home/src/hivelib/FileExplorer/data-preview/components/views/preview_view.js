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
import { CustomRequest as RequestConstructor } from "./../../../../modal/request_modal";

class PreviewView extends React.Component {
  constructor(props) {
    super(props);
    this.fileUrlParam = {
      cmd: 'objFile',
      ids: null
    }
    this.state = {
      url: null,
      type:null
    };
    this.fileBlobURL = null
  }

  componentDidMount() {
    this.urlExchangeParameter = this.props.urlExchangeParameter;
    this.fetchData = this.props.fetchData;

    if (this.props.ids || this.props.url) {
      this.setState((state, props) => {
        this.fileUrlParam.ids = `${props.ids}`
        let url = props.url && props.ids
                    ? this.urlExchangeParameter( props.url, "ids", props.ids)
                    : null;
        if (url) this.fetchData(url);
        return {
          baseUrl: props.url ? props.url : null,
          data: ""
        };
      });
    }
  }

  componentWillUnmount(){
    this.revokeBlob()
  }

  revokeBlob = () => {
    if(this.fileBlobURL){
      URL.revokeObjectURL(this.fileBlobURL)
    }
  }

  getFile = (parameters = {}) =>{
    parameters = Object.assign(parameters,this.fileUrlParam)
    return new RequestConstructor({parameters});
  }

  setUpPDF = async () =>{
    this.props.handleLoading(true);
    await this.getFile().handleFetch()
    .then(response => response.blob())
    .then(data  => {
      this.revokeBlob()
      this.fileBlobURL = URL.createObjectURL(data)
      this.setState(()=> {
        return{
          type: 'pdf',
          url: this.fileBlobURL
        }
      }, this.props.handleLoading(false));
    })
    .catch((err)=>{
      this.setState(() => {
        return{
          type: 'default',
          dataToPrint: this.dataToPrint}
      },this.props.handleLoading(false));
      console.log(err)
    })
  }

  setUpTXT = async () => {
    this.props.handleLoading(true);
    let params = {}
    if(parseInt(this.file.size) > 5000000 ){
      params = {maxSize: 5000000, ellipsize:  '[File truncated due to large size]'}
    }
    return await this.getFile(params).handleFetch()
    .then(response => response.text())
    .then(data  => {
       this.setState(()=> {
         return{
         type: 'txt',
         url: this.getFile().getFullURL(),
         dataToPrint: data
        }} ,this.props.handleLoading(false) ) ;
    })
    .catch((err)=>{
      this.setState(() => {
        return {
          type: 'default',
          dataToPrint: this.dataToPrint}
      },this.props.handleLoading(false));
      console.log(err)
    })
  }
  setUpIMG =() =>{
    this.setState({
      type: 'img',
      url: this.getFile().getFullURL(),
      dataToPrint: this.dataToPrint
    }) ;
  }
  setUpDEFAULT =() =>{
    this.setState({
      type: 'default',
      url: this.getFile().getFullURL(),
      dataToPrint: this.dataToPrint
    });
  }
  componentDidUpdate(prevProps, prevState){
    if (prevProps.ids !== this.props.ids) {
      this.fileUrlParam.ids = `${this.props.ids}`
      this.handleLoadData();
    }
    if(this.props.reload !== prevProps.reload && this.props.reload){
        this.handleLoadData();
    }
    if (prevProps.data !== this.props.data) {
      this.revokeBlob()
      this.dataToPrint = this.props.prepData(this.props.data);
      this.file = {
        ext: `${this.props.data.ext}`,
        id: `${this.props.ids}`,
        name: `${this.props.data.name}`,
        size: this.props.data.size
      }
      let ext = `${this.props.data.ext}`.toLowerCase()
      switch(true){
        case ext.endsWith('pdf'):
          this.setUpPDF()
          break;
        case ext.endsWith('txt'):
          this.setUpTXT()
          break;
        case ext.endsWith('png'):
        case ext.endsWith('jpg'):
          this.setUpIMG()
          break;
        default:
          this.setUpDEFAULT()
      }
    }
  }

  handleLoadData = () => {
    if (this.state.baseUrl) {
      let url = this.urlExchangeParameter( this.state.baseUrl, "ids", this.props.ids );
      this.props.handleLoading(true);
      this.fetchData(url);
    }
  }

  render() {
    switch(this.state.type){
      case 'pdf':
        return(
          <object alt={this.props.ids} key={this.state.url} type="application/pdf" data={this.state.url} width="1080px" height="1200px" />
        )
      case 'img':
        return(
          <object alt={this.props.ids} key={this.state.url} type="image/jpg" data={this.state.url}/>
        )
      default:
        return (<pre key={this.state.url} >{this.state.dataToPrint}</pre>)
    }
  }
}

export default PreviewView;
