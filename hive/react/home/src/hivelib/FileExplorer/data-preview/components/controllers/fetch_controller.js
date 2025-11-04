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
import axios from "axios";
import Papa from "papaparse";
import { Spin } from 'antd';
import { getPrefix } from './../../../../modal/url_modal'

const getDataTblUrl = "cmdr=-qpData&req=&grp=1&dname=_.csv&default=error:%20134730%20_.csv%20not%20found";

const withFetch = (WrappedComponent) => {
    class HOC extends React.Component {
        constructor(props) {
            super(props);
            this.state = { allData: '', loading: true };
        };

        componentWillUnmount() {
            clearTimeout(this.rawRequest);
        }

        componentDidUpdate(prevProps, prevState) {
            if (prevProps.ids !== this.props.ids) {
                clearTimeout(this.rawRequest);
            }
            if (prevProps.reload !== this.props.reload) {
                clearTimeout(this.rawRequest);
            }
        }

        urlExchangeParameter = (url, parname, newvalue, doNotForce) => {
            url = "" + url; // in case if this is a document.location
            if (url.indexOf("static://") !== -1)
                return url;

            var sepRe = "(&|\\?|//)";
            var parRe = sepRe + parname + "=[^&]*";

            if (doNotForce && url.search(new RegExp(parRe)) === -1)
                return url;

            if (newvalue === "-") {
                return url.replace(new RegExp(parRe + "(&?)"), function (match, sep, endsep) {
                    return (sep === "&") ? endsep : sep;
                });
            }

            var replacement = parname + "=" + newvalue;
            var parFound = false;

            url = url.replace(new RegExp(parRe), function (match, sep) {
                parFound = true;
                return sep + replacement;
            });

            if (parFound)
                return url;

            if (url === "")
                return "?" + replacement;

            url = url.replace(new RegExp(sepRe + "?$"), function (match, sep) {
                if (!sep)
                    sep = "&";
                return sep + replacement;
            });
            return url;
        };

        trim = (s) => {
            s = s.replace(/(^\s*)|(\s*$)/gi, "");
            s = s.replace(/[ ]{2,}/gi, " ");
            s = s.replace(/\n /, "\n");
            return s;
        }

        fetchData = async (dataUrl, stateKey) => {
            if (!stateKey) stateKey = "data";
            const projectData = {};
            if (sessionStorage.getItem("projectID")) {
                projectData["params"] = { projectID: sessionStorage.getItem("projectID") };
            }

            await axios
                .get(getPrefix() + '?' + dataUrl, projectData)
                .then(response => {
                    if (response && response.status < 400 && response.data !== 'unknown') {
                        this.setState((state, props) => {
                            let tmp = {};
                            tmp[stateKey] = response.data;
                            let allData;

                            if (!state.allData) {
                                allData = tmp
                            } else {
                                let stateAllData = Object.assign({}, state.allData)
                                stateAllData[stateKey] = response.data
                                allData = stateAllData
                            }
                            return {
                                allData: allData,
                                loading: false
                            }
                        });
                    } else {
                        return response
                    }
                })
                .catch((e) => {
                    console.log(e)
                    this.setState((state, props) => {
                        return {
                            loading: false
                        }
                    })
                    return e.response
                })
        }

        loadRawCheck = async (loadUrl, stateKey) => { //once receive response, will just set the req in the state
            const projectData = {};
            if (sessionStorage.getItem("projectID")) {
                projectData["params"] = { projectID: sessionStorage.getItem("projectID") };
            }
            if (!stateKey) stateKey = "data";
            axios
                .get(getPrefix() + '?' + loadUrl, projectData)
                .then(response => {
                    let parsedResponse = Papa.parse(response.data, { header: true });

                    if (parsedResponse.data) {
                        let curStat = parseInt(parsedResponse.data[0].stat);
                        if (curStat === 5) {
                            //this.setState({tblReq:parsedResponse.data[0].reqID});
                            this.fetchData(this.urlExchangeParameter(getDataTblUrl, "req", parsedResponse.data[0].reqID), stateKey);
                        }
                        else if (curStat > 5) {
                            this.setState((state, props) => {
                                let tmp = {};
                                tmp[stateKey] = "error";
                                let allData = !state.allData ? tmp : { ...state.allData, ...tmp };
                                return {
                                    allData: allData,
                                    loading: false
                                }

                            });
                            console.log("server error returned");
                        }
                        else {
                            this.rawRequest = setTimeout(() => { this.loadRawCheck("cmdr=-qpRawCheck&req=" + parsedResponse.data[0].reqID, stateKey) }, 3000);
                        }
                    }
                })
                .catch(error => alert(error));
        }

        handleLoading = (loading) => {
            if (loading !== this.state.loading) {
                this.setState({ loading: loading })
            }
        }

        render() {
            return (
                <Spin spinning={!this.props.url ? false : this.state.loading}
                    wrapperClassName={this.props.componentInline ? 'hive-inline-block' : ''}
                >
                    <WrappedComponent
                        urlExchangeParameter={this.urlExchangeParameter}
                        trim={this.trim}
                        fetchData={this.fetchData}
                        loadRawCheck={this.loadRawCheck}
                        loading={this.state.loading}
                        handleLoading={this.handleLoading}
                        {...this.state.allData}
                        {...this.props}
                    />
                </Spin>
            )
        }

    }

    return HOC;

}

export default withFetch;