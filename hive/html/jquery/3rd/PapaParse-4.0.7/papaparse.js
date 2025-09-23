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
(function(global)
{
    "use strict";

    var IS_WORKER = !global.document, SCRIPT_PATH;
    var workers = {}, workerIdCounter = 0;

    var DEFAULTS = {
        delimiter: "",
        newline: "",
        header: false,
        dynamicTyping: false,
        preview: 0,
        step: undefined,
        encoding: "",
        worker: false,
        comments: false,
        complete: undefined,
        error: undefined,
        download: false,
        chunk: undefined,
        skipEmptyLines: false,
        fastMode: false
    };

    global.Papa = {};

    global.Papa.parse = CsvToJson;
    global.Papa.unparse = JsonToCsv;

    global.Papa.RECORD_SEP = String.fromCharCode(30);
    global.Papa.UNIT_SEP = String.fromCharCode(31);
    global.Papa.BYTE_ORDER_MARK = "\ufeff";
    global.Papa.BAD_DELIMITERS = ["\r", "\n", "\"", global.Papa.BYTE_ORDER_MARK];
    global.Papa.WORKERS_SUPPORTED = !!global.Worker;

    global.Papa.LocalChunkSize = 1024 * 1024 * 10;
    global.Papa.RemoteChunkSize = 1024 * 1024 * 5;
    global.Papa.DefaultDelimiter = ",";

    global.Papa.Parser = Parser;
    global.Papa.ParserHandle = ParserHandle;
    global.Papa.NetworkStreamer = NetworkStreamer;
    global.Papa.FileStreamer = FileStreamer;

    if (global.jQuery)
    {
        var $ = global.jQuery;
        $.fn.parse = function(options)
        {
            var config = options.config || {};
            var queue = [];

            this.each(function(idx)
            {
                var supported = $(this).prop('tagName').toUpperCase() == "INPUT"
                                && $(this).attr('type').toLowerCase() == "file"
                                && global.FileReader;

                if (!supported || !this.files || this.files.length == 0)
                    return true;

                for (var i = 0; i < this.files.length; i++)
                {
                    queue.push({
                        file: this.files[i],
                        inputElem: this,
                        instanceConfig: $.extend({}, config)
                    });
                }
            });

            parseNextFile();
            return this;


            function parseNextFile()
            {
                if (queue.length == 0)
                {
                    if (isFunction(options.complete))
                        options.complete();
                    return;
                }

                var f = queue[0];

                if (isFunction(options.before))
                {
                    var returned = options.before(f.file, f.inputElem);

                    if (typeof returned === 'object')
                    {
                        if (returned.action == "abort")
                        {
                            error("AbortError", f.file, f.inputElem, returned.reason);
                            return;
                        }
                        else if (returned.action == "skip")
                        {
                            fileComplete();
                            return;
                        }
                        else if (typeof returned.config === 'object')
                            f.instanceConfig = $.extend(f.instanceConfig, returned.config);
                    }
                    else if (returned == "skip")
                    {
                        fileComplete();
                        return;
                    }
                }

                var userCompleteFunc = f.instanceConfig.complete;
                f.instanceConfig.complete = function(results)
                {
                    if (isFunction(userCompleteFunc))
                        userCompleteFunc(results, f.file, f.inputElem);
                    fileComplete();
                };

                Papa.parse(f.file, f.instanceConfig);
            }

            function error(name, file, elem, reason)
            {
                if (isFunction(options.error))
                    options.error({name: name}, file, elem, reason);
            }

            function fileComplete()
            {
                queue.splice(0, 1);
                parseNextFile();
            }
        }
    }


    if (IS_WORKER)
        global.onmessage = workerThreadReceivedMessage;




    function CsvToJson(_input, _config)
    {
        var config = IS_WORKER ? _config : copyAndValidateConfig(_config);
        var useWorker = config.worker && Papa.WORKERS_SUPPORTED && SCRIPT_PATH;

        if (useWorker)
        {
            var w = newWorker();

            w.userStep = config.step;
            w.userChunk = config.chunk;
            w.userComplete = config.complete;
            w.userError = config.error;

            config.step = isFunction(config.step);
            config.chunk = isFunction(config.chunk);
            config.complete = isFunction(config.complete);
            config.error = isFunction(config.error);
            delete config.worker;

            w.postMessage({
                input: _input,
                config: config,
                workerId: w.id
            });
        }
        else
        {
            if (typeof _input === 'string')
            {
                if (config.download)
                {
                    var streamer = new NetworkStreamer(config);
                    streamer.stream(_input);
                }
                else
                {
                    var ph = new ParserHandle(config);
                    var results = ph.parse(_input);
                    return results;
                }
            }
            else if ((global.File && _input instanceof File) || _input instanceof Object)
            {
                if (config.step || config.chunk)
                {
                    var streamer = new FileStreamer(config);
                    streamer.stream(_input);
                }
                else
                {
                    var ph = new ParserHandle(config);

                    if (IS_WORKER)
                    {
                        var reader = new FileReaderSync();
                        var input = reader.readAsText(_input, config.encoding);
                        return ph.parse(input);
                    }
                    else
                    {
                        reader = new FileReader();
                        reader.onload = function(event)
                        {
                            var ph = new ParserHandle(config);
                            var results = ph.parse(event.target.result);
                        };
                        reader.onerror = function()
                        {
                            if (isFunction(config.error))
                                config.error(reader.error, _input);
                        };
                        reader.readAsText(_input, config.encoding);
                    }
                }
            }
        }
    }






    function JsonToCsv(_input, _config)
    {
        var _output = "";
        var _fields = [];

        var _quotes = false;
        var _delimiter = ",";
        var _newline = "\r\n";

        unpackConfig();

        if (typeof _input === 'string')
            _input = JSON.parse(_input);

        if (_input instanceof Array)
        {
            if (!_input.length || _input[0] instanceof Array)
                return serialize(null, _input);
            else if (typeof _input[0] === 'object')
                return serialize(objectKeys(_input[0]), _input);
        }
        else if (typeof _input === 'object')
        {
            if (typeof _input.data === 'string')
                _input.data = JSON.parse(_input.data);

            if (_input.data instanceof Array)
            {
                if (!_input.fields)
                    _input.fields = _input.data[0] instanceof Array
                                    ? _input.fields
                                    : objectKeys(_input.data[0]);

                if (!(_input.data[0] instanceof Array) && typeof _input.data[0] !== 'object')
                    _input.data = [_input.data];
            }

            return serialize(_input.fields || [], _input.data || []);
        }

        throw "exception: Unable to serialize unrecognized input";


        function unpackConfig()
        {
            if (typeof _config !== 'object')
                return;

            if (typeof _config.delimiter === 'string'
                && _config.delimiter.length == 1
                && global.Papa.BAD_DELIMITERS.indexOf(_config.delimiter) == -1)
            {
                _delimiter = _config.delimiter;
            }

            if (typeof _config.quotes === 'boolean'
                || _config.quotes instanceof Array)
                _quotes = _config.quotes;

            if (typeof _config.newline === 'string')
                _newline = _config.newline;
        }


        function objectKeys(obj)
        {
            if (typeof obj !== 'object')
                return [];
            var keys = [];
            for (var key in obj)
                keys.push(key);
            return keys;
        }

        function serialize(fields, data)
        {
            var csv = "";

            if (typeof fields === 'string')
                fields = JSON.parse(fields);
            if (typeof data === 'string')
                data = JSON.parse(data);

            var hasHeader = fields instanceof Array && fields.length > 0;
            var dataKeyedByField = !(data[0] instanceof Array);

            if (hasHeader)
            {
                for (var i = 0; i < fields.length; i++)
                {
                    if (i > 0)
                        csv += _delimiter;
                    csv += safe(fields[i], i);
                }
                if (data.length > 0)
                    csv += _newline;
            }

            for (var row = 0; row < data.length; row++)
            {
                var maxCol = hasHeader ? fields.length : data[row].length;

                for (var col = 0; col < maxCol; col++)
                {
                    if (col > 0)
                        csv += _delimiter;
                    var colIdx = hasHeader && dataKeyedByField ? fields[col] : col;
                    csv += safe(data[row][colIdx], col);
                }

                if (row < data.length - 1)
                    csv += _newline;
            }

            return csv;
        }

        function safe(str, col)
        {
            if (typeof str === "undefined" || str === null)
                return "";

            str = str.toString().replace(/"/g, '""');

            var needsQuotes = (typeof _quotes === 'boolean' && _quotes)
                            || (_quotes instanceof Array && _quotes[col])
                            || hasAny(str, global.Papa.BAD_DELIMITERS)
                            || str.indexOf(_delimiter) > -1
                            || str.charAt(0) == ' '
                            || str.charAt(str.length - 1) == ' ';

            return needsQuotes ? '"' + str + '"' : str;
        }

        function hasAny(str, substrings)
        {
            for (var i = 0; i < substrings.length; i++)
                if (str.indexOf(substrings[i]) > -1)
                    return true;
            return false;
        }
    }



    function NetworkStreamer(config)
    {
        config = config || {};
        if (!config.chunkSize)
            config.chunkSize = Papa.RemoteChunkSize;

        var start = 0, fileSize = 0, rowCount = 0;
        var aggregate = "";
        var partialLine = "";
        var xhr, url, nextChunk, finishedWithEntireFile;
        var userComplete, handle, configCopy;
        replaceConfig(config);

        this.resume = function()
        {
            paused = false;
            nextChunk();
        };

        this.finished = function()
        {
            return finishedWithEntireFile;
        };

        this.pause = function()
        {
            paused = true;
        };

        this.abort = function()
        {
            finishedWithEntireFile = true;
            if (isFunction(userComplete))
                userComplete({ data: [], errors: [], meta: { aborted: true } });
        };

        this.stream = function(u)
        {
            url = u;
            if (IS_WORKER)
            {
                nextChunk = function()
                {
                    readChunk();
                    chunkLoaded();
                };
            }
            else
            {
                nextChunk = function()
                {
                    readChunk();
                };
            }

            nextChunk();
        };

        function readChunk()
        {
            if (finishedWithEntireFile)
            {
                chunkLoaded();
                return;
            }

            xhr = new XMLHttpRequest();
            
            if (!IS_WORKER)
            {
                xhr.onload = chunkLoaded;
                xhr.onerror = chunkError;
            }

            xhr.open("GET", url, !IS_WORKER);
            
            if (config.step || config.chunk)
            {
                var end = start + configCopy.chunkSize - 1;
                if (fileSize && end > fileSize)
                    end = fileSize;
                xhr.setRequestHeader("Range", "bytes="+start+"-"+end);
            }

            try {
                xhr.send();
            }
            catch (err) {
                chunkError(err.message);
            }

            if (IS_WORKER && xhr.status == 0)
                chunkError();
            else
                start += configCopy.chunkSize;
        }

        function chunkLoaded()
        {
            if (xhr.readyState != 4)
                return;

            if (xhr.status < 200 || xhr.status >= 400)
            {
                chunkError();
                return;
            }

            aggregate += partialLine + xhr.responseText;
            partialLine = "";

            finishedWithEntireFile = (!config.step && !config.chunk) || start > getFileSize(xhr);

            if (!finishedWithEntireFile)
            {
                var lastLineEnd = aggregate.lastIndexOf("\r");

                if (lastLineEnd == -1)
                    lastLineEnd = aggregate.lastIndexOf("\n");

                if (lastLineEnd != -1)
                {
                    partialLine = aggregate.substring(lastLineEnd + 1);
                    aggregate = aggregate.substring(0, lastLineEnd);
                }
                else
                {
                    nextChunk();
                    return;
                }
            }

            var results = handle.parse(aggregate);
            aggregate = "";
            if (results && results.data)
                rowCount += results.data.length;

            var finishedIncludingPreview = finishedWithEntireFile || (configCopy.preview && rowCount >= configCopy.preview);

            if (IS_WORKER)
            {
                global.postMessage({
                    results: results,
                    workerId: Papa.WORKER_ID,
                    finished: finishedIncludingPreview
                });
            }
            else if (isFunction(config.chunk))
            {
                config.chunk(results, handle);
                results = undefined;
            }

            if (isFunction(userComplete) && finishedIncludingPreview)
                userComplete(results);

            if (!finishedIncludingPreview && (!results || !results.meta.paused))
                nextChunk();
        }

        function chunkError(errorMessage)
        {
            var errorText = xhr.statusText || errorMessage;
            if (isFunction(config.error))
                config.error(errorText);
            else if (IS_WORKER && config.error)
            {
                global.postMessage({
                    workerId: Papa.WORKER_ID,
                    error: errorText,
                    finished: false
                });
            }
        }

        function getFileSize(xhr)
        {
            var contentRange = xhr.getResponseHeader("Content-Range");
            return parseInt(contentRange.substr(contentRange.lastIndexOf("/") + 1));
        }

        function replaceConfig(config)
        {
            configCopy = copy(config);
            userComplete = configCopy.complete;
            configCopy.complete = undefined;
            configCopy.chunkSize = parseInt(configCopy.chunkSize);
            handle = new ParserHandle(configCopy);
            handle.streamer = this;
        }
    }









    function FileStreamer(config)
    {
        config = config || {};
        if (!config.chunkSize)
            config.chunkSize = Papa.LocalChunkSize;

        var start = 0;
        var file;
        var slice;
        var aggregate = "";
        var partialLine = "";
        var rowCount = 0;
        var paused = false;
        var self = this;
        var reader, nextChunk, slice, finishedWithEntireFile;
        var userComplete, handle, configCopy;
        replaceConfig(config);

        var usingAsyncReader = typeof FileReader !== 'undefined';

        this.stream = function(f)
        {
            file = f;
            slice = file.slice || file.webkitSlice || file.mozSlice;

            if (usingAsyncReader)
            {
                reader = new FileReader();
                reader.onload = chunkLoaded;
                reader.onerror = chunkError;
            }
            else
                reader = new FileReaderSync();

            nextChunk();
        };

        this.finished = function()
        {
            return finishedWithEntireFile;
        };

        this.pause = function()
        {
            paused = true;
        };

        this.resume = function()
        {
            paused = false;
            nextChunk();
        };

        this.abort = function()
        {
            finishedWithEntireFile = true;
            if (isFunction(userComplete))
                userComplete({ data: [], errors: [], meta: { aborted: true } });
        };

        function nextChunk()
        {
            if (!finishedWithEntireFile && (!configCopy.preview || rowCount < configCopy.preview))
                readChunk();
        }

        function readChunk()
        {
            var end = Math.min(start + configCopy.chunkSize, file.size);
            var txt = reader.readAsText(slice.call(file, start, end), config.encoding);
            if (!usingAsyncReader)
                chunkLoaded({ target: { result: txt } });
        }

        function chunkLoaded(event)
        {
            start += configCopy.chunkSize;

            aggregate += partialLine + event.target.result;
            partialLine = "";

            finishedWithEntireFile = start >= file.size;

            if (!finishedWithEntireFile)
            {
                var lastLineEnd = aggregate.lastIndexOf("\r");

                if (lastLineEnd == -1)
                    lastLineEnd = aggregate.lastIndexOf("\n");

                if (lastLineEnd != -1)
                {
                    partialLine = aggregate.substring(lastLineEnd + 1);
                    aggregate = aggregate.substring(0, lastLineEnd);
                }
                else
                {
                    nextChunk();
                    return;
                }
            }

            var results = handle.parse(aggregate);
            aggregate = "";
            if (results && results.data)
                rowCount += results.data.length;

            var finishedIncludingPreview = finishedWithEntireFile || (configCopy.preview && rowCount >= configCopy.preview);

            if (IS_WORKER)
            {
                global.postMessage({
                    results: results,
                    workerId: Papa.WORKER_ID,
                    finished: finishedIncludingPreview
                });
            }
            else if (isFunction(config.chunk))
            {
                config.chunk(results, self, file);
                if (paused)
                    return;
                results = undefined;
            }

            if (isFunction(userComplete) && finishedIncludingPreview)
                userComplete(results);

            if (!finishedIncludingPreview && (!results || !results.meta.paused))
                nextChunk();
        }

        function chunkError()
        {
            if (isFunction(config.error))
                config.error(reader.error, file);
            else if (IS_WORKER && config.error)
            {
                global.postMessage({
                    workerId: Papa.WORKER_ID,
                    error: reader.error,
                    file: file,
                    finished: false
                });
            }
        }

        function replaceConfig(config)
        {
            configCopy = copy(config);
            userComplete = configCopy.complete;
            configCopy.complete = undefined;
            configCopy.chunkSize = parseInt(configCopy.chunkSize);
            handle = new ParserHandle(configCopy);
            handle.streamer = this;
        }

    }





    function ParserHandle(_config)
    {
        var FLOAT = /^\s*-?(\d*\.?\d+|\d+\.?\d*)(e[-+]?\d+)?\s*$/i;

        var self = this;
        var _stepCounter = 0;
        var _input;
        var _parser;
        var _paused = false;
        var _delimiterError;
        var _fields = [];
        var _results = {
            data: [],
            errors: [],
            meta: {}
        };

        if (isFunction(_config.step))
        {
            var userStep = _config.step;
            _config.step = function(results)
            {
                _results = results;

                if (needsHeaderRow())
                    processResults();
                else
                {
                    processResults();

                    if (_results.data.length == 0)
                        return;

                    _stepCounter += results.data.length;
                    if (_config.preview && _stepCounter > _config.preview)
                        _parser.abort();
                    else
                        userStep(_results, self);
                }
            };
        }

        this.parse = function(input)
        {
            if (!_config.newline)
                _config.newline = guessLineEndings(input);

            _delimiterError = false;
            if (!_config.delimiter)
            {
                var delimGuess = guessDelimiter(input);
                if (delimGuess.successful)
                    _config.delimiter = delimGuess.bestDelimiter;
                else
                {
                    _delimiterError = true;
                    _config.delimiter = Papa.DefaultDelimiter;
                }
                _results.meta.delimiter = _config.delimiter;
            }

            var parserConfig = copy(_config);
            if (_config.preview && _config.header)
                parserConfig.preview++;

            _input = input;
            _parser = new Parser(parserConfig);
            _results = _parser.parse(_input);
            processResults();
            if (isFunction(_config.complete) && !_paused && (!self.streamer || self.streamer.finished()))
                _config.complete(_results);
            return _paused ? { meta: { paused: true } } : (_results || { meta: { paused: false } });
        };

        this.pause = function()
        {
            _paused = true;
            _parser.abort();
            _input = _input.substr(_parser.getCharIndex());
        };

        this.resume = function()
        {
            _paused = false;
            _parser = new Parser(_config);
            _parser.parse(_input);
            if (!_paused)
            {
                if (self.streamer && !self.streamer.finished())
                    self.streamer.resume();
                else if (isFunction(_config.complete))
                    _config.complete(_results);
            }
        };

        this.abort = function()
        {
            _parser.abort();
            if (isFunction(_config.complete))
                _config.complete(_results);
            _input = "";
        };

        function processResults()
        {
            if (_results && _delimiterError)
            {
                addError("Delimiter", "UndetectableDelimiter", "Unable to auto-detect delimiting character; defaulted to '"+Papa.DefaultDelimiter+"'");
                _delimiterError = false;
            }

            if (_config.skipEmptyLines)
            {
                for (var i = 0; i < _results.data.length; i++)
                    if (_results.data[i].length == 1 && _results.data[i][0] == "")
                        _results.data.splice(i--, 1);
            }

            if (needsHeaderRow())
                fillHeaderFields();

            return applyHeaderAndDynamicTyping();
        }

        function needsHeaderRow()
        {
            return _config.header && _fields.length == 0;
        }

        function fillHeaderFields()
        {
            if (!_results)
                return;
            for (var i = 0; needsHeaderRow() && i < _results.data.length; i++)
                for (var j = 0; j < _results.data[i].length; j++)
                    _fields.push(_results.data[i][j]);
            _results.data.splice(0, 1);
        }

        function applyHeaderAndDynamicTyping()
        {
            if (!_results || (!_config.header && !_config.dynamicTyping))
                return _results;

            for (var i = 0; i < _results.data.length; i++)
            {
                var row = {};

                for (var j = 0; j < _results.data[i].length; j++)
                {
                    if (_config.dynamicTyping)
                    {
                        var value = _results.data[i][j];
                        if (value == "true")
                            _results.data[i][j] = true;
                        else if (value == "false")
                            _results.data[i][j] = false;
                        else
                            _results.data[i][j] = tryParseFloat(value);
                    }

                    if (_config.header)
                    {
                        if (j >= _fields.length)
                        {
                            if (!row["__parsed_extra"])
                                row["__parsed_extra"] = [];
                            row["__parsed_extra"].push(_results.data[i][j]);
                        }
                        else
                            row[_fields[j]] = _results.data[i][j];
                    }
                }

                if (_config.header)
                {
                    _results.data[i] = row;
                    if (j > _fields.length)
                        addError("FieldMismatch", "TooManyFields", "Too many fields: expected " + _fields.length + " fields but parsed " + j, i);
                    else if (j < _fields.length)
                        addError("FieldMismatch", "TooFewFields", "Too few fields: expected " + _fields.length + " fields but parsed " + j, i);
                }
            }

            if (_config.header && _results.meta)
                _results.meta.fields = _fields;
            return _results;
        }

        function guessDelimiter(input)
        {
            var delimChoices = [",", "\t", "|", ";", Papa.RECORD_SEP, Papa.UNIT_SEP];
            var bestDelim, bestDelta, fieldCountPrevRow;

            for (var i = 0; i < delimChoices.length; i++)
            {
                var delim = delimChoices[i];
                var delta = 0, avgFieldCount = 0;
                fieldCountPrevRow = undefined;

                var preview = new Parser({
                    delimiter: delim,
                    preview: 10
                }).parse(input);

                for (var j = 0; j < preview.data.length; j++)
                {
                    var fieldCount = preview.data[j].length;
                    avgFieldCount += fieldCount;

                    if (typeof fieldCountPrevRow === 'undefined')
                    {
                        fieldCountPrevRow = fieldCount;
                        continue;
                    }
                    else if (fieldCount > 1)
                    {
                        delta += Math.abs(fieldCount - fieldCountPrevRow);
                        fieldCountPrevRow = fieldCount;
                    }
                }

                avgFieldCount /= preview.data.length;

                if ((typeof bestDelta === 'undefined' || delta < bestDelta)
                    && avgFieldCount > 1.99)
                {
                    bestDelta = delta;
                    bestDelim = delim;
                }
            }

            _config.delimiter = bestDelim;

            return {
                successful: !!bestDelim,
                bestDelimiter: bestDelim
            }
        }

        function guessLineEndings(input)
        {
            input = input.substr(0, 1024*1024);

            var r = input.split('\r');

            if (r.length == 1)
                return '\n';

            var numWithN = 0;
            for (var i = 0; i < r.length; i++)
            {
                if (r[i][0] == '\n')
                    numWithN++;
            }

            return numWithN >= r.length / 2 ? '\r\n' : '\r';
        }

        function tryParseFloat(val)
        {
            var isNumber = FLOAT.test(val);
            return isNumber ? parseFloat(val) : val;
        }

        function addError(type, code, msg, row)
        {
            _results.errors.push({
                type: type,
                code: code,
                message: msg,
                row: row
            });
        }
    }





    function Parser(config)
    {
        config = config || {};
        var delim = config.delimiter;
        var newline = config.newline;
        var comments = config.comments;
        var step = config.step;
        var preview = config.preview;
        var fastMode = config.fastMode;

        if (typeof delim !== 'string'
            || delim.length != 1
            || Papa.BAD_DELIMITERS.indexOf(delim) > -1)
            delim = ",";

        if (comments === delim)
            throw "Comment character same as delimiter";
        else if (comments === true)
            comments = "#";
        else if (typeof comments !== 'string'
            || Papa.BAD_DELIMITERS.indexOf(comments) > -1)
            comments = false;

        if (newline != '\n' && newline != '\r' && newline != '\r\n')
            newline = '\n';

        var cursor = 0;
        var aborted = false;

        this.parse = function(input)
        {
            if (typeof input !== 'string')
                throw "Input must be a string";

            var inputLen = input.length,
                delimLen = delim.length,
                newlineLen = newline.length,
                commentsLen = comments.length;
            var stepIsFunction = typeof step === 'function';

            cursor = 0;
            var data = [], errors = [], row = [];

            if (!input)
                return returnable();

            if (fastMode)
            {
                var rows = input.split(newline);
                for (var i = 0; i < rows.length; i++)
                {
                    if (comments && rows[i].substr(0, commentsLen) == comments)
                        continue;
                    if (stepIsFunction)
                    {
                        data = [ rows[i].split(delim) ];
                        doStep();
                        if (aborted)
                            return returnable();
                    }
                    else
                        data.push(rows[i].split(delim));
                    if (preview && i >= preview)
                    {
                        data = data.slice(0, preview);
                        return returnable(true);
                    }
                }
                return returnable();
            }

            var nextDelim = input.indexOf(delim, cursor);
            var nextNewline = input.indexOf(newline, cursor);

            for (;;)
            {
                if (input[cursor] == '"')
                {
                    var quoteSearch = cursor;

                    cursor++;

                    for (;;)
                    {
                        var quoteSearch = input.indexOf('"', quoteSearch+1);

                        if (quoteSearch == -1)
                        {
                            errors.push({
                                type: "Quotes",
                                code: "MissingQuotes",
                                message: "Quoted field unterminated",
                                row: data.length,
                                index: cursor
                            });
                            return finish();
                        }

                        if (quoteSearch == inputLen-1)
                        {
                            row.push(input.substring(cursor, quoteSearch).replace(/""/g, '"'));
                            data.push(row);
                            if (stepIsFunction)
                                doStep();
                            return returnable();
                        }

                        if (input[quoteSearch+1] == '"')
                        {
                            quoteSearch++;
                            continue;
                        }

                        if (input[quoteSearch+1] == delim)
                        {
                            row.push(input.substring(cursor, quoteSearch).replace(/""/g, '"'));
                            cursor = quoteSearch + 1 + delimLen;
                            nextDelim = input.indexOf(delim, cursor);
                            nextNewline = input.indexOf(newline, cursor);
                            break;
                        }

                        if (input.substr(quoteSearch+1, newlineLen) == newline)
                        {
                            row.push(input.substring(cursor, quoteSearch).replace(/""/g, '"'));
                            saveRow(quoteSearch + 1 + newlineLen);
                            nextDelim = input.indexOf(delim, cursor);

                            if (stepIsFunction)
                            {
                                doStep();
                                if (aborted)
                                    return returnable();
                            }
                            
                            if (preview && data.length >= preview)
                                return returnable(true);

                            break;
                        }
                    }

                    continue;
                }

                if (comments && row.length == 0 && input.substr(cursor, commentsLen) == comments)
                {
                    if (nextNewline == -1)
                        return returnable();
                    cursor = nextNewline + newlineLen;
                    nextNewline = input.indexOf(newline, cursor);
                    nextDelim = input.indexOf(delim, cursor);
                    continue;
                }

                if (nextDelim != -1 && (nextDelim < nextNewline || nextNewline == -1))
                {
                    row.push(input.substring(cursor, nextDelim));
                    cursor = nextDelim + delimLen;
                    nextDelim = input.indexOf(delim, cursor);
                    continue;
                }

                if (nextNewline != -1)
                {
                    row.push(input.substring(cursor, nextNewline));
                    saveRow(nextNewline + newlineLen);

                    if (stepIsFunction)
                    {
                        doStep();
                        if (aborted)
                            return returnable();
                    }

                    if (preview && data.length >= preview)
                        return returnable(true);

                    continue;
                }

                break;
            }


            return finish();


            function finish()
            {
                row.push(input.substr(cursor));
                data.push(row);
                cursor = inputLen;
                if (stepIsFunction)
                    doStep();
                return returnable();
            }

            function saveRow(newCursor)
            {
                data.push(row);
                row = [];
                cursor = newCursor;
                nextNewline = input.indexOf(newline, cursor);
            }

            function returnable(stopped)
            {
                return {
                    data: data,
                    errors: errors,
                    meta: {
                        delimiter: delim,
                        linebreak: newline,
                        aborted: aborted,
                        truncated: !!stopped
                    }
                };
            }

            function doStep()
            {
                step(returnable());
                data = [], errors = [];
            }
        };

        this.abort = function()
        {
            aborted = true;
        };

        this.getCharIndex = function()
        {
            return cursor;
        };
    }


    function getScriptPath()
    {
        var id = "worker" + String(Math.random()).substr(2);
        document.write('<script id="'+id+'"></script>');
        return document.getElementById(id).previousSibling.src;
    }

    function newWorker()
    {
        if (!Papa.WORKERS_SUPPORTED)
            return false;
        var w = new global.Worker(SCRIPT_PATH);
        w.onmessage = mainThreadReceivedMessage;
        w.id = workerIdCounter++;
        workers[w.id] = w;
        return w;
    }

    function mainThreadReceivedMessage(e)
    {
        var msg = e.data;
        var worker = workers[msg.workerId];

        if (msg.error)
            worker.userError(msg.error, msg.file);
        else if (msg.results && msg.results.data)
        {
            if (isFunction(worker.userStep))
            {
                for (var i = 0; i < msg.results.data.length; i++)
                {
                    worker.userStep({
                        data: [msg.results.data[i]],
                        errors: msg.results.errors,
                        meta: msg.results.meta
                    });
                }
                delete msg.results;
            }
            else if (isFunction(worker.userChunk))
            {
                worker.userChunk(msg.results, msg.file);
                delete msg.results;
            }
        }

        if (msg.finished)
        {
            if (isFunction(workers[msg.workerId].userComplete))
                workers[msg.workerId].userComplete(msg.results);
            workers[msg.workerId].terminate();
            delete workers[msg.workerId];
        }
    }

    function workerThreadReceivedMessage(e)
    {
        var msg = e.data;

        if (typeof Papa.WORKER_ID === 'undefined' && msg)
            Papa.WORKER_ID = msg.workerId;

        if (typeof msg.input === 'string')
        {
            global.postMessage({
                workerId: Papa.WORKER_ID,
                results: Papa.parse(msg.input, msg.config),
                finished: true
            });
        }
        else if ((global.File && msg.input instanceof File) || msg.input instanceof Object)
        {
            var results = Papa.parse(msg.input, msg.config);
            if (results)
                global.postMessage({
                    workerId: Papa.WORKER_ID,
                    results: results,
                    finished: true
                });
        }
    }

    function copyAndValidateConfig(origConfig)
    {
        if (typeof origConfig !== 'object')
            origConfig = {};

        var config = copy(origConfig);

        if (typeof config.delimiter !== 'string'
            || config.delimiter.length != 1
            || Papa.BAD_DELIMITERS.indexOf(config.delimiter) > -1)
            config.delimiter = DEFAULTS.delimiter;

        if (config.newline != '\n'
            && config.newline != '\r'
            && config.newline != '\r\n')
            config.newline = DEFAULTS.newline;

        if (typeof config.header !== 'boolean')
            config.header = DEFAULTS.header;

        if (typeof config.dynamicTyping !== 'boolean')
            config.dynamicTyping = DEFAULTS.dynamicTyping;

        if (typeof config.preview !== 'number')
            config.preview = DEFAULTS.preview;

        if (typeof config.step !== 'function')
            config.step = DEFAULTS.step;

        if (typeof config.complete !== 'function')
            config.complete = DEFAULTS.complete;

        if (typeof config.error !== 'function')
            config.error = DEFAULTS.error;

        if (typeof config.encoding !== 'string')
            config.encoding = DEFAULTS.encoding;

        if (typeof config.worker !== 'boolean')
            config.worker = DEFAULTS.worker;

        if (typeof config.download !== 'boolean')
            config.download = DEFAULTS.download;

        if (typeof config.skipEmptyLines !== 'boolean')
            config.skipEmptyLines = DEFAULTS.skipEmptyLines;

        if (typeof config.fastMode !== 'boolean')
            config.fastMode = DEFAULTS.fastMode;

        return config;
    }

    function copy(obj)
    {
        if (typeof obj !== 'object')
            return obj;
        var cpy = obj instanceof Array ? [] : {};
        for (var key in obj)
            cpy[key] = copy(obj[key]);
        return cpy;
    }

    function isFunction(func)
    {
        return typeof func === 'function';
    }
})(this);
