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
import { Radio } from "antd";
import URLS from "./../../../../data/urls.js";

export default class AnnotaionView extends React.Component {
    constructor(props) {
        super(props);
        //ionAnnotTypes_type
        //ionAnnotTypes_relation
        //ionAnnotTypes_seqID
        this.state = {
            selectedRtype: 'ionAnnotTypes_type'
        }
    };

    componentDidMount() {
        this.urlExchangeParameter = this.props.urlExchangeParameter;
        this.fetchData = this.props.fetchData;
        this.loadRawCheck = this.props.loadRawCheck;
        this.handleDataLoad()
    }

    componentDidUpdate(prevProps, prevState) {
        if (prevProps.ids !== this.props.ids) {
            this.handleDataLoad()
        }

        if (prevState.selectedRtype !== this.state.selectedRtype) {
            this.handleDataLoad()
        }
    };
    renderPreview = (data) => {
        if (!data) {
            return (<h3> No Data =( </h3>);
        }
        return (<pre>{data}</pre>);
    }

    handleDataLoad = () => {
        const type = this.state.selectedRtype

        let url = this.urlExchangeParameter(URLS[type].url, URLS[type].id, this.props.ids);
        this.props.handleLoading(true);
        this.fetchData(url);
    }

    handleSizeChange = (e) => {
        this.setState({
            selectedRtype: e.target.value
        })
    }

    render() {

        return (<>
            <h4>Record Type: </h4>

            <Radio.Group size={'default'} value={this.state.selectedRtype} onChange={this.handleSizeChange}>
                <Radio.Button value="ionAnnotTypes_relation">Relation</Radio.Button>
                <Radio.Button value="ionAnnotTypes_type">Type</Radio.Button>
                <Radio.Button value="ionAnnotTypes_seqID">Sequence Identifier</Radio.Button>
            </Radio.Group>
            <hr />
            {this.renderPreview(this.props.data)}
        </>
        );
    }
}