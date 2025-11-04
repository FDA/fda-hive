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
import "antd/dist/antd.css";
import { message, Button, Icon, Progress, Spin, Tooltip } from "antd";
import Bottleneck from "bottleneck";
import filesize from "filesize";
import { connect } from 'react-redux';
import ProgressButtons from '../../ProgressButtons';
import "./ChunkedUpload.css";
import {
    progressView ,
    handleProgress ,
    handleIsOnline ,
    getFileList ,
    subFileList ,
    clearUpload,
    addToFormArchiver,
    recordSVCArchiverId,
    loadingFiles,
    recordPipelineLink
} from '../../../actions';
import { windowLeaveNotificationON , windowLeaveNotificationOFF } from '../../../controller/NotificationController';
import modals from '../../../../hivelib/modal/modal_collector';
import controllers from '../../../../hivelib/controller/controller_collector';
import { CustomRequest  } from '../../../../hivelib/modal/request_modal';
import ChooseFolder from './ChooseFolder'

const { url_modal } = modals
const throttle = controllers.fireController.throttle;

const DEFAULT_CHUNK_SIZE = 10 * 1024 * 1024; //10MB
const DEFAULT_CONCURRENCY = 2;
const IDLE_TIMEOUT_SEC = 86400;  // 1 day
const ON_PAUSE_IDLE_TIMEOUT_SEC = 432000; // 5 days

function formatEta(secs) {
  const s = Math.round(secs);
  const days = Math.floor(s / (3600 * 24));
  let hours = Math.floor(s / 3600);
  if (days > 0) {
    let eta = `${days}d`;
    let deltaSec = s - ( days * 3600 * 24 )
    hours = Math.floor(deltaSec / 3600)
    if (hours > 0) {
      eta = `${eta} ${hours}h`;
    }
    return eta;
  }
  const minutes = Math.floor(s / 60) % 60;
  if (hours > 0) {
    let eta = `${hours}h`;
    if (minutes > 0) {
      eta = `${eta} ${minutes}m`;
    }
    return eta;
  }
  const eta = [];
  if (minutes > 0) {
    eta.push(`${minutes}m`);
  }
  const seconds = s % 60;
  if (seconds > 0) {
    eta.push(`${seconds}s`);
  }
  return eta.join(' ');
}

function quantity(n, s) {
  return n === 1 ? `${n} ${s}` : `${n} ${s}s`;
}

function filesTooltip(path, total) {
  return total === 1 ? path : `${path} and ${total - 1} more`;
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function until(fn , time =10) {
  while (!fn()) {
      await sleep(time);
  }
}

class ChunkedUpload extends React.Component {
  constructor(props) {
    super(props);

    this.group = [];
    this.hasExtensions = {}
    this.state = {
      hightlight: false,
      progress: 0,
      requestID: undefined,
      OnPauseTime: null
    };
    this.fileModal = this.props.fileModal;
    this.activeRequests = new Set();
    this.fileInputRef = React.createRef();
    this.folderInputRef = React.createRef();
    this.openFileDialog = this.openFileDialog.bind(this);
    this.openFolderDialog = this.openFolderDialog.bind(this)
    this.onFilesAdded = this.onFilesAdded.bind(this);
    this.onDragOver = this.onDragOver.bind(this);
    this.onDragLeave = this.onDragLeave.bind(this);
    this.onDrop = this.onDrop.bind(this);
    this.calcEta = this.calcEta.bind(this);
    this.pause = this.pause.bind(this);
    this.resume = this.resume.bind(this);
    this.requestID = undefined;
    this.startTime = 0;
    this.progress = 0;
        this.timeBeforePause = 0;
    this.pausedTime = null;
    this.IdleTimeoutSec = IDLE_TIMEOUT_SEC;
    this.OnPauseIdleTimeoutSec = ON_PAUSE_IDLE_TIMEOUT_SEC;
    this.failedChunks = 0;
    this.deltaFailedChunks = 0;
    this.idleTimeStart = null;
    this.IdleTimeout = null;
    this.PauseTimeout = null;
  }

  componentDidMount() {
    window.addEventListener('offline', this.OfflinePause);
    window.addEventListener('online', this.BackOnline);
    windowLeaveNotificationON();
  }
  componentWillUnmount() {
    window.removeEventListener('offline', this.OfflinePause);
    window.removeEventListener('online', this.BackOnline);
  }

  async componentDidUpdate(prevProps, prevState) {
    if (this.props.uploadState !== prevProps.uploadState) {
      if(this.props.uploadState !== 'uploading'){
        this.props.progressView({uploading:false})
      }
      switch (this.props.uploadState) {
        case 'preparing':
          this.removeAll(false)
          this.startUpload()
          break;
        case 'uploading':
          this.props.progressView({uploading:true})
          break;
        case 'error':
          this.props.progressView({uploading:false})
          break;
        case 'pausing':
          this.pause();
          break;
        case 'resuming':
          this.resume();
          break;
        case 'done':
          this.handleFinished();
          break;
        case 'disabled':
          break;
        default:
          return null;
      }
    }

    if (this.props.clear !== prevProps.clear && this.props.clear === true) { // If job is canceled we still want to have access to files
      if (this.props.uploadState === 'preparing') {
        this.stopInitUpload()
      } else if (this.props.uploadState === 'error'){
        this.removeAll(true)
        this.setState({requestID: undefined});
        this.props.handleProgress('disabled');
        this.paused = false;
      } else if(this.props.uploadState === 'disabled'){
        this.pauseAllProcess()
        this.removeAll(false)
        this.cancel()
      }else {
        if (this.props.uploadState !== 'done') {
          this.pauseAllProcess() // 1. Pause the process all together and abort existing requests
        }
        this.removeAll(true) // 2. Remove limiter , progress , files , state
        if (this.props.uploadState === 'done') {
          this.setState({requestID: undefined});
        }
      }

    }
    if(this.state.error !== prevState.error){
      this.setState((state,props)=>{
        let error = state.error
        props.progressView({error:error})
      })
    }
    if(this.state.progress !== prevState.progress){
      this.setState((state,props)=>{
        let progress = state.progress;
        props.progressView({progress:progress})
      } )
    }

    if(this.props.filesList !== prevProps.filesList && this.props.filesList.length === 0){
      await this.props.handleProgress('disabled')
    }

  }

  handleOnPauseTimeout = async() => {
      this.props.handleProgress('error');
      this.pauseAllProcess()
      this.removeAll(false);
      this.setState({
        OnPauseTime: null,
        error: 'Pause timeout, upload was canceled. Retry the upload or start new one.'
      })
  }

  getStep = (retryCount) => {
    let idleTimePassed = Date.now() - this.idleTimeStart;
    let tt = this.IdleTimeoutSec*1000
    if(idleTimePassed < tt){
      let i = retryCount;
      let step = Math.ceil(333*(1 + (i/100)))
      if(idleTimePassed + 500 + step > tt ){
        step = tt - idleTimePassed;
      }
      return step;
    }
    return false;
  }

  // # 1
  startUpload = async () => {
    this.setState({ error: null });
    let init = await this.initUpload();
    if (init === true) {
      this.uploadFiles(this.props.filesList);
    }
  }

  // # 2
  initUpload =  async () => {
    this.startTime = Date.now();
    this.initLimiter = new Bottleneck({
      maxConcurrent:1,
      minTime: 333
    })

    this.initLimiter.on("failed", async (error, jobInfo) => {
      if(this.IdleTimeout === null){
        this.startIdleTimeout()
      }
      if(jobInfo.retryCount < 100 && this.IdleTimeout !== true && this.props.uploadState === 'preparing') {  // limit on steps otherwise it never stops if a 404 error
        let step = this.getStep(jobInfo.retryCount)
        return step;
      }
    });

    await this.untilOnline();

    return await this.initLimiter.schedule({id: 'requestInfo'}, () => this.getRequestInfo() )
      .then( (json) => {
        if(this.props.uploadState === 'preparing'){
          this.cancelIdleTimeout();
          this.setupUpload(json);
          return true;
        }
        return false;
      })
      .catch((error) => {
        this.stopInitUpload()
        this.setState({ error: error ? error : 'Exceeded number of attempts'});
        return false;
      })
  }

  // # 2a
  getRequestInfo = async () => {
    await this.untilOnline(); // if offline
    //1. Get params
    return new Promise((resolve, reject) => {
      let parameters = {
        'cmd': 'init' ,
        'prop.svc-pipeline-upload.st2_datasource': 'file://' ,
        'prop.svc-pipeline-upload.isPostponed': '0',
      }
      let obj = JSON.parse(JSON.stringify(this.props.formArchiver));
      obj = this.handleFormArchiverSetUp( obj )
      parameters = Object.assign( parameters , obj )

      let cgi = 'upload.cgi';
      let fetchparametes = {  cache: 'no-cache',method: 'GET' };

      let request = new CustomRequest({parameters,cgi});
      request.handleFetch(fetchparametes)
        .then( async (response) => {
          let status = await response.status
          if ( status >= 400 ){
            let error = 'Failed to initialize the upload. Try again later.';
            reject( error )
          }
          let json = await response.json()
          if (typeof json !== "object" || json.error){
            let error = typeof json === "object" && json.error ? json.error : 'Failed to initialize the upload. Try again later.';
            reject( error )
          }else {
            resolve( json );
          }
        })
    })
  }

  // # 2b
  stopInitUpload = () => {
    if(this.initLimiter){
      this.initLimiter.updateSettings({reservoir:0 , expiration: 0});
      this.initLimiter.stop();
      this.initLimiter = null;
    }
    this.props.handleProgress('idle');
    this.props.clearUpload(false); // does nothing, switch back for future
  }

  // # 3
  setupUpload = (init_json) => {
    this.props.handleProgress('uploading')
    this.updateUploadingParameters(init_json)

    // Starts the queue of chunks
    this.limiter = new Bottleneck({maxConcurrent: this.concurrency});
    this.limiter.on("error", (err) => {
      console.log('xrh error',err)
    });

    this.limiter.on("failed", async (error, jobInfo) => {
      if(this.IdleTimeout === null) {
        this.startIdleTimeout();
      }

      if(!this.limiter || this.props.uploadState === 'error'){
        throw new Error(error);
      } else {
        this.failedChunks ++
        if(this.failedChunks > 1 &&  !this.slow && error){
          this.slow = true;
          this.limiter.updateSettings({maxConcurrent: 1}); // wont change when jobs are running
        }

        if (this.IdleTimeout !== true && error) {
          let step = this.getStep(jobInfo.retryCount)
          return step;
        } else {
          this.setState({ error: !error && this.IdleTimeout === true ?  'Exceeded number of attempts' : error });
          this.clearRecentSpeed();
          this.limiter.stop();
          this.limiter = null;
        }
      }
    });
    // this.limiter.on("debug", function (message, data) { console.log(message , data) });
  }

  // # 4
  uploadFiles = async(fileEntries) => {
    async function getFile(fileEntry) {
      try {
        return await new Promise((resolve, reject) => fileEntry.file(resolve, reject));
      } catch (err) {
        console.error(err);
        return false;
      }
    }

    const totalFiles = fileEntries.length;
    if (totalFiles === 0) {
      message.warning('No files selected',8);
      return;
    }

    this.paused = false;
    this.group = { chunks: [], uploadedChunks: 0, fileCount: 0, totalFiles, size: 0 };
    this.updateSummary();

    let files = [];
    let spaceLeft = this.chunkSize;

    await this.untilOnline(); // if offline
    for (let i = 0; i < totalFiles; i++) {
      const fileEntry = fileEntries[i];
      const file = fileEntry instanceof File ? fileEntry : await getFile(fileEntry);
      if (!file) { continue; }
      const size = file.size;
      this.group.size += size;
      let path = fileEntry.uploadPath;
      const id = fileEntry.id;
      if (path[0] === '/' || path[0] === '\\') {
        path = path.substring(1);
      }

      if (!this.group.tooltip) {
        this.group.tooltip = path;
      }
      // Prepare Chunks
      const lastModified = file.lastModified;
      if (size <= spaceLeft) {
        files.push({
          file,
          size,
          path,
          id,
          lastModified,
          totalChunks: 1
        });
        spaceLeft -= size;
      } else {
        const fileChunks = 1 + Math.ceil((size - spaceLeft) / this.chunkSize);
        const reverse = this.reverseFileRegex && this.reverseFileRegex.test(path);
        // Finish up loading previous chunk with another file
        files.push({
          file,
          size,
          path,
          id ,
          lastModified,
          start: reverse ? size - spaceLeft : 0,
          end: reverse ? size : spaceLeft,
          totalChunks: fileChunks
        });
        let start = spaceLeft;
        this.scheduleChunk({files});

        // Cut large file into different chunks fully filling them up
        while (start + this.chunkSize <= size) {
          this.scheduleChunk({files: [{
              file,
              size,
              path,
              id,
              lastModified,
              start: reverse ? size - start - this.chunkSize : start,
              end: reverse ? size - start : start + this.chunkSize,
              totalChunks: fileChunks
            }
          ]});
          start += this.chunkSize;
        }

        // Fit last part into new chunk
        files = [];
        spaceLeft = this.chunkSize;
        if (start < size) {
          files.push({file, size, path, id , lastModified, start: reverse ? 0 : start, end: reverse ? size - start : size, totalChunks: fileChunks});
          spaceLeft -= size - start;
        }
      }
      this.group.fileCount = i + 1;
    }
    if (files.length > 0) {
      this.scheduleChunk({files, relSize: 1.0 * (this.chunkSize - spaceLeft) /  this.chunkSize});
    }
    this.updateSummary();
    this.initRecentSpeed();
  }

  initRecentSpeed() {
      this.etaTimer = setInterval(this.calcEta, 1000); // increase time how often calculates
  }

  clearRecentSpeed() {
    this.etaTimer = clearInterval(this.etaTimer);
        this.startTime = null;
    this.setState({eta: null});
    clearTimeout(this.resumeRetry)
    clearTimeout(this.pauseRetry)
  }

  // # 3a
  updateUploadingParameters = (json) => {
    // IdleTimeoutSec: 86400
    // OnPauseIdleTimeoutSec: 432000
    // alignments: (3) ["sam", "bam", "blast_out"]
    // chunkSize: 10485760
    // compress: false
    // concurrency: 4
    // reference_genomes: (12) ["fa", "fa.gz", "fas", "fas.gz", "fasta", "fasta.gz", "fsa", "fsa.gz", "gb", "gb.gz", "gbk", "gbk.gz"]
    // reqid: 517349
    // reverseFileRegex: ".(tar|zip)"
    if(typeof json !== 'object' || Object.keys(json).length === 0){
      return ;
    }
    this.requestID = json.reqid || this.requestID;
    this.IdleTimeoutSec = json.IdleTimeoutSec || json.IdleTimeoutSec === 0 ? json.IdleTimeoutSec : this.IdleTimeoutSec;
    this.OnPauseIdleTimeoutSec = json.OnPauseIdleTimeoutSec || json.OnPauseIdleTimeoutSec === 0 ? json.OnPauseIdleTimeoutSec : this.OnPauseIdleTimeoutSec;
    this.concurrency = json.concurrency || DEFAULT_CONCURRENCY;
    this.chunkSize = json.chunkSize || DEFAULT_CHUNK_SIZE;
    if (json.reverseFileRegex) {
      this.reverseFileRegex = new RegExp(json.reverseFileRegex);
    }
    this.setState({ requestID: this.requestID })
  }

  calcEta() {
    let chunksSentSize = this.group.chunksSentSize
    let totalChunksSize = this.group.totalChunksSize
    let OnPauseTime;
    let eta = null;

    if ( this.paused && this.props.uploadState === 'paused') {
      eta = '∞';
      OnPauseTime =  Math.ceil((Date.now() - this.startTime) / 1000)  // calc OnPauseTime in sec
      this.setState({eta , OnPauseTime });
    }  else if (chunksSentSize < totalChunksSize) {
      let now = Date.now();
      let deltams = ( now - this.startTime ) + this.timeBeforePause;
      let speed = chunksSentSize / (deltams);
      let deltasize = totalChunksSize - chunksSentSize
      let secondsLeft = deltasize === 0 ? 0 : ( deltasize / speed ) / 1000;
      eta = formatEta( Math.ceil(secondsLeft) );
    } else if (chunksSentSize >= totalChunksSize){
      eta = 0;
    }

    if ( !this.paused ){
      this.setState({eta, progress: this.progress});
    }
  }

  scheduleChunk = async (chunk) => {
    const chunks = this.group.chunks;
    const index = chunks.length;
    chunks.push(chunk);

    this.limiter.schedule({id: index}, () => this.sendChunk(chunk, index))
      .then( async (json) => {
        this.updateUploadingParameters(json)
        if(this.failedChunks > 0){
          this.failedChunks --
        }
        await this.untilOnline(); // if offline
        await this.untilResumed(); // if paused
        this.cancelIdleTimeout();
        if(this.requestID !== this.state.requestID) { return ;}
        this.group.uploadedChunks++;
        if (this.group.uploadedChunks === chunks.length && this.group.fileCount === this.group.totalFiles) {
          this.finishUpload();
        } else {
          if ((this.failedChunks < 2 && this.slow) || (json.concurrency && json.concurrency !== this.concurrency)) {
            this.slow = false;
            this.concurrency = json.concurrency;
            this.limiter.updateSettings({maxConcurrent: this.concurrency});
          }
        }
        this.updateSummary();
      })
      .catch(error => {
        console.log(error)
        if(error){
          this.setState({ error});
        }
      });
  }

  removeAll(clearfiles = true) {
    if (this.limiter && !this.limiter.empty()){
      this.limiter.stop();
      this.limiter = null;
    }

    //Clear all Files
    if(clearfiles){
      this.props.subFileList([]);
      this.fileModal.clearAll();
    }
    this.progress = 0;

    let errorprop;
    this.setState((state,prop)=>{
      errorprop = state.error && prop.uploadState === 'error' ? state.error : null;
      return{progress: this.progress, error: errorprop, requestID: undefined, totalChunks: 0}
    });
    this.props.clearUpload(false); // just to changes state for future
  }

  sendChunk = async (chunk, index) => {
    const onProgress = async (event) => {
      if (event.lengthComputable) {
        // update loaded amount
        chunk.loaded = event.loaded;
        chunk.totalSize = event.total;

        let chunksSentSize = 0;
        let totalChunksSize = 0;
        let chunksInProgress = 0
        this.group.chunks.forEach((chunk)=>{
          chunksSentSize += chunk.loaded || 0;
          chunksInProgress += chunk.totalSize ? 1 : 0;
          totalChunksSize += chunk.totalSize || 0;
        })
        totalChunksSize = chunksInProgress !== this.group.chunks.length ? this.group.size : totalChunksSize;

        this.group.chunksSentSize = chunksSentSize;
        this.group.totalChunksSize = totalChunksSize;

        this.progress = (chunksSentSize/totalChunksSize * 100);

        await this.untilResumed(); // if paused
        if(this.progress === 100){
          this.setState({progress:this.progress})
        }
      }
    };

    await this.untilOnline(); // if offline
    await this.untilResumed(); // if paused

    return new Promise((resolve, reject) => {
      if(this.requestID !== this.state.requestID) {
        reject(this.props.uploadState === 'error' ? false : 'Upload failed, worng requestID');
      }
      const xhr = new XMLHttpRequest();
      let throttleRate  = 500;// size ^^ and maxConcurrent ^ =  throttle time ^
      switch(true){
        case  (5*10**5) < this.group.size && this.group.size < (5*10**6) :
          throttleRate = 750;
          break;
        case this.group.size < (5*10**7) : // < 50 MB
          throttleRate = 1000;
          break;
        case this.group.size <= (5*10**8) : // < 500 MB
          throttleRate = 2000;
          break;
        case this.group.size > (5*10**8) : // > 500 MB
          throttleRate = 4000;
          break;
        default:
          break;
      }

      const rate = this.concurrency >= 4 ? 4 : 2;
      throttleRate =  throttleRate * (this.concurrency / rate);
      xhr.upload.addEventListener("progress", throttle(onProgress,throttleRate)); // throttle onProgress takes to much memory for constant calculations anf rendering this.group.size

      // xhr.addEventListener("error", event => {
      //   this.activeRequests.delete(xhr);
      //   const json = xhr.responseText ? JSON.parse(xhr.responseText) : {};
      //   let error = typeof json === "object" && json.error ? json.error : 'Failed to the xhr chunk.';
      //   reject(error);
      // });

      xhr.upload.addEventListener("error", event => {
        this.activeRequests.delete(xhr);
        if(this.props.uploadState === 'error') {
          reject(false)
        }
        const json = xhr.responseText ? JSON.parse(xhr.responseText) : '';
        let error = typeof json === "object" && json.error ? json.error : 'Failed to the upload chunk.';
        reject(error);
      });

      xhr.addEventListener("load", (event) => {
        let json;
        if(xhr.status >= 500){
          reject('Failed to the upload chunk.')
        } else if(xhr.status >= 400){
          json = xhr.responseText && JSON.parse(xhr.responseText) ? JSON.parse(xhr.responseText) : {};
          let error = typeof json === "object" && json.error ? json.error : 'Failed to the upload chunk.';
          reject(error)
        }else if(xhr.status >= 200){
          json = xhr.responseText && JSON.parse(xhr.responseText) ? JSON.parse(xhr.responseText) : {};
          this.activeRequests.delete(xhr);
          resolve(json);
        }
      });
      let formData = new FormData();
      const infoFiles = chunk.files.map(f => {
        const {file, ...rest} = f;
        return rest;
      })
      const info = {index, total: this.group.fileCount === this.group.totalFiles ? this.group.chunks.length : -1, files: infoFiles};
      const infoBlob = new Blob([JSON.stringify(info)], {type: 'application/json'});
      formData.set('info', infoBlob);
      chunk.files.forEach((info, i) => {
        const blob = info.end ? info.file.slice(info.start, info.end) : info.file;
        formData.set(info.id, blob, info.file.name);
      });
      xhr.open("POST", `${url_modal.getPrefixPlain()}/upload.cgi?raw=1&req=${this.state.requestID}`);

      xhr.onabort = event => {
        this.activeRequests.delete(xhr);
        reject(false);
      };

      xhr.send(formData);
      this.activeRequests.add(xhr);
    });
  }

  handleFormArchiverSetUp = (form) => {
      let { filesList } = this.props
      let nm = '';
      for (let i = 0; i < filesList.length; i++) {
        nm = `${nm}, ${filesList[i].name}`;
        if( nm.length > 100 && i < filesList.length - 1) {
          nm = `${nm}...`;
          break;
        }
      }
      form['prop.svc-pipeline-upload.name'] = nm.substring(2);

      return form;
  }

  handleError = () => {
      this.setState({ error: true });
  }

  handleFinished = () => {
      this.setState({ error: null });
  }

  recordArchiverId = async (id) => {
      id = parseInt(id);
      this.props.recordSVCArchiverId(id)
  }

  async finishUpload() {

    this.group.uploaded = true;
    let parameters = {
      raw: 1 ,
      req: this.requestID ,
    }
    let cgi = 'upload.cgi';
    let fetchparametes = { method: 'PUT' };

    let request = new CustomRequest({parameters,cgi})

    let retry = 0;
    const getResponse = async() => {
      await this.untilOnline(); // if offline
      if(this.requestID !== this.state.requestID) { return; }

      request.handleFetch(fetchparametes)
      .then((response) => {
        if(response.status >= 400) {
          return [response.json() , true];
        } else if(response.status >= 200){
          return [response.json()];
        }
      })
      .then(async (args)=>{
        let [json , error] = args;
        json = await json;
        if (error && retry > 5) {
          error = json && json.error ? json.error : `Could not finish the upload`;
        } else if (error) {
          retry ++
          setTimeout( getResponse() , 1000) ;
        } else if(json.id) {
          this.recordArchiverId(json.id);
        }
        if(json.goto){
          this.props.recordPipelineLink(json.goto);
        }
        if(!error || retry > 5){
          this.props.handleProgress('done');
          windowLeaveNotificationOFF();
          this.clearRecentSpeed(); // all chunks are uploaded
        }
      })
    }
    getResponse()
  }

  async openFileDialog() {
    if (this.props.disabled) return;
    await this.props.loadingFiles(true)
    this.fileInputRef.current.oninput = (e) => {
      if(e.target.value === "" || !e.target.value){
        this.props.loadingFiles(false)
      }else{
         this.props.loadingFiles(true)
      }
    }

    this.fileInputRef.current.onclick = async (e)=>{
      if(e.target.title === 'init'){
        this.fileInputRef.current.title = null
        await this.props.loadingFiles(true)
      }else if(e.currentTarget.value === "" || !e.currentTarget.value){
        await this.props.loadingFiles(false)
      } else{
        this.fileInputRef.current.value = null
        await this.props.loadingFiles(true)
      }
    }

    this.fileInputRef.current.click();
  }

  async openFolderDialog() {
    if (this.props.disabled) return;
    await this.props.loadingFiles(true)

    this.folderInputRef.current.oninput = (e) => {
      if(e.target.value === "" || !e.target.value){
        this.props.loadingFiles(false)
      }else{
         this.props.loadingFiles(true)
      }
    }

    this.folderInputRef.current.onclick = async (e)=>{
      if(e.target.title === 'init'){
        this.folderInputRef.current.title = null
        await this.props.loadingFiles(true)
      }else if(e.currentTarget.value === "" || !e.currentTarget.value){
        await this.props.loadingFiles(false)
      } else{
        this.folderInputRef.current.value = null
        await this.props.loadingFiles(true)
      }
    }

    this.folderInputRef.current.click();
  }

  async onFilesAdded(evt) {
    if (this.props.disabled) return;
    //let files = [...evt.target.files];
    let files = evt.target.files
    // add key namepath for table structuring
    let loopedFiles = this.fileModal.loopFiles( files , this.props.filesList );
    message.warning(`${files.length} items added`,8)
    //add more items
    this.props.getFileList(loopedFiles.files);
    this.props.handleProgress('idle')
  }

  onDragOver(event) {
    event.preventDefault();
    if (this.props.disabled) return;
    this.setState({ hightlight: true });
  }

  onDragLeave(event) {
    this.setState({ hightlight: false });
  }

  async onDrop(event) {
    event.preventDefault();
    if (this.props.disabled || this.props.uploadState === 'done') return;
    this.props.loadingFiles(true) // For Files Tree component
    const items = event.dataTransfer.items;
    const array = await this.fileListToArray(items);
    if(array.length === 0){
      this.props.loadingFiles(false)
      message.warning('Folder is empty',8)
      this.setState({ hightlight: false });
      return;
    }

    message.warning(`${array.length} items added`,8)
    // add parameters and detec extentions
    let loopedFiles = this.fileModal.loopFiles( array , this.props.filesList );
    await this.props.getFileList(loopedFiles.files);
    this.props.handleProgress('idle')
    this.setState({ hightlight: false });
  }

  async fileListToArray(items) {
    async function getAllFileEntries(dataTransferItemList) {
      let fileEntries = [];
      // Use BFS to traverse entire directory/file structure
      let queue = [];
      // Unfortunately dataTransferItemList is not iterable i.e. no forEach
      for (let i = 0, n = dataTransferItemList.length; i < n; i++) {
        const item = dataTransferItemList[i];
        queue.push(item.webkitGetAsEntry());
      }
      while (queue.length > 0) {
        let entry = queue.shift();
        if (entry.isFile) {
          fileEntries.push(entry);
        } else if (entry.isDirectory) {
          let reader = entry.createReader();
          queue.push(...await readAllDirectoryEntries(reader));
        }
      }
      return fileEntries;
    }

    // Get all the entries (files or sub-directories) in a directory by calling readEntries until it returns empty array
    async function readAllDirectoryEntries(directoryReader) {
      let entries = [];
      let readEntries = await readEntriesPromise(directoryReader);
      while (readEntries.length > 0) {
        entries.push(...readEntries);
        readEntries = await readEntriesPromise(directoryReader);
      }
      return entries;
    }

    // Wrap readEntries in a promise to make working with readEntries easier
    async function readEntriesPromise(directoryReader) {
      try {
        return await new Promise((resolve, reject) => {
          directoryReader.readEntries(resolve, reject);
        });
      } catch (err) {
        console.error(err);
      }
    }

    return await getAllFileEntries(items);
  }

  untilResumed = async() => {
    await until(() => !this.paused);
  }

  untilOnline = async () => {
    await until(() => this.props.isOnline)
  }

  pauseAllProcess = () => {
    if(!this.paused){
      this.paused = true;
      if(this.limiter){
        this.limiter.submit({weight: 0}, this.untilResumed);
      }
    }
    this.stopRequests()
  }

  stopRequests = () => {
    setTimeout(() => {
      this.activeRequests.forEach(xhr => xhr.abort());
      this.clearRecentSpeed();
    }, 50);
  }

  async cancel(){
    let parameters = {
      raw: 1 ,
      req: this.requestID ,
    }
    let cgi = 'upload.cgi';
    let fetchparametes = { method: 'DELETE' };

    let request = new CustomRequest({parameters,cgi})

    let retry = 0;
    const getResponse = async () => {
      await this.untilOnline(); // if offline
      request.handleFetch(fetchparametes)
      .then((response) => {
        if(response.status >= 400) {
          return [response.json() , true];
        } else if(response.status >= 200){
          return [response.json()];
        }
      })
      .then((args)=>{
        let[json,error] = args;
        if (error && retry > 5) {
          error = json && json.error ? json.error : `Could not delete the upload`;
          this.setState({error})
        } else if (error) {
          retry ++ ;
          setTimeout( getResponse() , 1000 * retry );
        } else {
          this.props.handleProgress('idle')
        }
      })
      .catch((err) => {
        console.log(err)
      })
    }
    getResponse()
  }

  getPauseResume = async(action) => {
    let prevAction = action === 'pause' ? 'resume' : 'pause';
    let parameters = {
      cmd: action ,
      req: this.requestID ,
    }
    let cgi = 'upload.cgi';
    let fetchparametes = {  cache: 'no-cache',method: 'GET' };

    let request = new CustomRequest({parameters,cgi})

    if(this[`${prevAction}Retry`]){
      clearTimeout(this[`${prevAction}Retry`])  // clear timeout of opposite action
    };

    let retry = 0;
    const getResponse = async () => {
      await this.untilOnline(); // if offline

      request.handleFetch(fetchparametes)
      .then((response) => {
        let json = response.json();
        if(response.status >= 400) {
          return [json , true];
        } else if(response.status >= 200){
          return [json];
        }
      })
      .then((args)=>{
        let[json,error] = args;
        if (error && retry > 5) {
          this.startTime = Date.now();
          error = json && json.error ? json.error : `Could not ${ action } the upload`;
          this.setState((state,prop)=>{
            return {
              error,
              OnPauseTime: this.paused ? state.OnPauseTime : null,
            }
          })
          if(action === 'resume'){
            this.props.handleProgress('paused')
          } else {
            this.props.handleProgress('uploading')
          }
        } else if (error) {
          retry ++
          this[`${action}Retry`] = setTimeout( getResponse , 1000) ;
        } else {
          this.startTime = Date.now();
          this.updateUploadingParameters(json)
          if(action === 'pause'){
            this.paused = true;
            this.timeBeforePause = ( Date.now() - this.startTime ) + this.timeBeforePause;
            this.limiter.submit({priority: 0, id: Date.now()}, this.untilResumed);
            this.PauseTimeout = setTimeout(() => {this.handleOnPauseTimeout()}, this.OnPauseIdleTimeoutSec * 1000);
            this.props.handleProgress('paused')
          } else {
            clearTimeout(this.PauseTimeout)
            this.paused = false;
                this.startTime = Date.now();
            this.setState({OnPauseTime: null})
            this.props.handleProgress('uploading')
          }
        }
      })
    }
    getResponse()
  }

  pause() {
    this.getPauseResume('pause')
  }

  handleIdleTimeout = () => {
    this.IdleTimeout = true;
    this.props.handleProgress('error');
    this.pauseAllProcess();
    this.removeAll(false);
    this.setState({error: 'Upload was idle for too long. Please try.'})
  }

  startIdleTimeout = () => {
    this.cancelIdleTimeout()
    this.idleTimeStart = Date.now();
    this.IdleTimeout = setTimeout(this.handleIdleTimeout,this.IdleTimeoutSec*1000)
  }
  cancelIdleTimeout = () =>{
    if(this.IdleTimeout !== null){
      clearTimeout(this.IdleTimeout)
      this.IdleTimeout = null;
      this.idleTimeStart = null;
    }
  }
  OfflinePause = () => {
    if(this.props.uploadState === 'preparing' || this.props.uploadState === 'uploading' || this.props.uploadState === 'pausing'){
      this.startIdleTimeout()
    }
    this.props.handleIsOnline(false);
    this.setState({error: 'No internet connection. Waiting to reconnect.'})
  }

  BackOnline = () => {
    if(this.props.uploadState === 'preparing' || this.props.uploadState === 'uploading' || this.props.uploadState === 'pausing'){
      this.cancelIdleTimeout()
    }
    this.props.handleIsOnline(true);
    this.setState({error: null})
  }

  resume() {
      this.getPauseResume('resume');
  }

  updateSummary() {
    if(this.paused) { return; }
    let totalChunks = this.group && this.group.hasOwnProperty('chunks') ?  this.group.chunks.length : 0;

        this.setState( (state,props) => {
        return { totalChunks, progress: this.progress };
        })
  }

  renderProgress() {
    const { progress, totalChunks, eta, error, OnPauseTime } = this.state;
    const status = this.props.uploadState === 'uploading' ? 'active' : this.props.uploadState === 'error' ? 'exception' : null;
    const n = totalChunks || 0;
    let percent  = progress;
    if(this.props.uploadState !== 'done' && percent === 100 ){
          percent = 99;
    }

    let mssg;
    switch(true){
      case this.props.uploadState === 'preparing':
        mssg = 'preparing...'
        break;
      case this.props.uploadState === 'paused' && !this.paused:
        mssg = 'pausing...';
        break;
      case this.props.uploadState === 'resuming' && this.paused:
          mssg = 'resuming...';
          break;
      case this.props.uploadState !== 'done' && percent === 100:
        mssg = 'finishing upload...';
        break;
      case this.props.uploadState === 'done':
        mssg = 'Upload finished';
        break;
      default:
        break;
    }

    return <>
              { error && <p style={{color:'#f5222d', marginBottom:'0px'}}> {typeof error === 'string' ? error : 'Upload failed, please try again.'}</p>}
              { mssg && <p style={{ marginBottom:'0px'}}>{mssg}</p> }
              <div className="progress">
                  <Progress
                      percent={Math.floor(percent)}
                      size="small"
                      status={status}
                  />
              </div>
              {n > 0 &&
               <div className="progress-total">
                    <span className="progress-files">
                        <Tooltip title={filesTooltip(this.group.tooltip, this.group.totalFiles)}>
                            {quantity(this.group.totalFiles, 'file')}
                        </Tooltip>
                    </span>
                    <span className="progress-size">
                        {typeof(this.group.size) === 'number' ? <Tooltip title={quantity(totalChunks, 'chunk')}>{filesize(this.group.size,{round:1})}</Tooltip> : <Spin size="small" />}
                    </span>
                    {this.props.uploadState === 'uploading' && eta && <span className="progress-eta">{<>ETA: {eta}</>}</span>}
                    {this.paused && OnPauseTime && <span >{ <> On Pause: {formatEta(OnPauseTime)}</>}</span>}
                    {n === 1 && <span className="progress-group-header"></span>}
              </div>
              }
          </>
  }

  render() {
    return (
      <div className="Upload">
          <ChooseFolder />
          <input
            ref={this.fileInputRef}
            className="re-fileinput"
            type="file"
            multiple
            onChange={this.onFilesAdded}
            title='init'
          />
          <Button
             onClick={this.openFileDialog}
             block
             style={{marginBottom:'5px'}}
             disabled={this.props.disabled || this.props.uploadState === 'done'}
          >
              <Icon type="upload" /> Browse Files
          </Button>
          <input
                ref={this.folderInputRef}
                className="re-fileinput"
                type="file"
                webkitdirectory=""
                directory=""
                multiple
                title='init'
                onChange={this.onFilesAdded}
          />
          <Button
             onClick={this.openFolderDialog}
             block
             style={{marginBottom:'5px'}}
             disabled={this.props.disabled || this.props.uploadState === 'done'}
          >
              <Icon type="upload" /> Browse Folders
          </Button>

          <div
              className={`dropzone ${this.state.hightlight ? "highlight" : ""}`}
              onDragOver={this.onDragOver}
              onDragLeave={this.onDragLeave}
              onDrop={this.onDrop}
          >
              <Icon type="inbox" style={{textAlign:'center',fontSize:'50px',display:'block'}} />
              <div style={{marginBottom:'25px'}}>
                 <p style={{textAlign:'center', marginBottom:'0px'}}><b>Drop files</b> and/or <b>folders</b> here to upload</p>
                {this.state.requestID === -1 ? <kbd><Icon type="loading"/> requesting ID ...</kbd> : ""}
                {this.renderProgress()}
              </div>
              <ProgressButtons />
        </div>
      </div>
    );
  }
}
const mapStateToProps = (state) => {
    return {
             uploadState: state.uploadState,
             filesList: state.filesList,
             clear: state.clearUpload,
             formArchiver: state.formArchiver,
             isOnline: state.isOnline
           };
}

const mapDispatchToProps = dispatch => ({
  progressView: i => dispatch(progressView(i)),
  getFileList: i => dispatch(getFileList(i)),
  subFileList: i => dispatch(subFileList(i)),
  handleProgress: i => dispatch(handleProgress(i)),
  clearUpload: i => dispatch(clearUpload(i)),
  addToFormArchiver: i => dispatch(addToFormArchiver(i)),
  recordSVCArchiverId: i => dispatch(recordSVCArchiverId(i)),
  handleIsOnline: i => dispatch(handleIsOnline(i)),
  loadingFiles: i => dispatch(loadingFiles(i)),
  recordPipelineLink: i => dispatch(recordPipelineLink(i))
})

export default connect(mapStateToProps,mapDispatchToProps)(ChunkedUpload)
