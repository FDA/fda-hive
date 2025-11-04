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

class TextView extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      url: null
    };
  }

  componentDidMount() {
    this.urlExchangeParameter = this.props.urlExchangeParameter;
    this.fetchData = this.props.fetchData;
    this.loadRawCheck = this.props.loadRawCheck;

    if (this.props.ids || this.props.url) {
      let url;
      this.setState((state, props) => {
        url =
          props.url && props.ids
            ? this.urlExchangeParameter(props.url, "ids", props.ids)
            : null;
        if (url) this.fetchData(url);
        return {
          baseUrl: props.url ? props.url : null,
          url: url,
          data: ""
        };
      });
    }
  }

  componentDidUpdate(prevProps, prevState) {
    if (prevProps.ids !== this.props.ids) {
        this.handleLoadData();
    }
    if(this.props.reload !== prevProps.reload){
        this.handleLoadData();
    }
    if (prevProps.data !== this.props.data) {
      this.setState((state, props) => {
        let dataToPrint = props.prepData
          ? props.prepData(props.data)
          : JSON.stringify(props.data, null, 4);
        return { dataToPrint: dataToPrint };
      });
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
    return (<pre>{this.state.dataToPrint}</pre>);
  }
}

export default TextView;
