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
export const UNIXtimeConverter = (UNIX_timestamp) => {
  var a = new Date(UNIX_timestamp * 1000);
  var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
  var year = a.getFullYear();
  var month = months[a.getMonth()];
  var date = a.getDate();
  var hour = a.getHours();
  var min = a.getMinutes();
  var sec = a.getSeconds();
  var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec ;
  return time;
}
function isDateInDate(check, d) //if d is missing it check if it is today
{
    if(!check)
        return ;
    var t_check = new Date(check);
    var t_d = d?new Date(d):new Date();
    return t_d.setHours(0,0,0,0) == t_check.setHours(0,0,0,0);
}

export const formatDatetime = (s,completeDate) => {
    var value = '';
    var d = null;
    if (typeof(s) === "number" || (typeof(s) === "string" && s.match(/^\s*\d*(\.\d*)?\s*$/))) {
        // seconds since epoch
        var sec = parseInt(s);
        d = new Date(sec * 1000);
    } else {
        // string representation
        if (typeof(s) == "string") {
            // ISO 8601 with space instead of 'T'
            s = s.replace(/^(\d{4}-\d{2}-\d{2}) (\d{2}:\d{2})/, "$1T$2");
        }
        d = new Date(s);
    }
    if (d && d.getTime() && !isNaN(d.getTime())) {
        var now = new Date();
        var timeDiff = now.getTime() - d.getTime();

        if (parseInt(timeDiff) < 0) {
            return d.toLocaleDateString();
        }

        if (parseInt(timeDiff) < 60000 )
            return "1 min ago";

        var min = parseInt((timeDiff) / 60000);
        now.setHours(0);
        now.setMinutes(0);
        now.setSeconds(0);
        now.setMilliseconds(0);

        if (min < 3 * 60  ) {
            var x = parseInt(min / 60);
            if (x) {
                value = x + 'h ';
            }
            x = parseInt(min % 60);
            if (x) {
                value += x + 'min ';
            }
            value += 'ago';
            if (value === 'ago') {
                value = 'recently';
            }
        }else if(!isDateInDate(d) && completeDate){
            value = d.toLocaleDateString() + " " + d.toLocaleTimeString();
        }else {
            value = d.toLocaleString('en-US',{month:"2-digit", day:"2-digit" , year:'numeric'});
        }
    }
    return value;
}
export const convertDate = (date) => {
    const now = new Date();
    let itemDate = new Date(date);
    let time_diff =  now-itemDate;
    //23 hours = 82800000 milliseconds

    /// more than 23 hours
    if( time_diff > 82800000){
        return `${itemDate.getMonth()}/${itemDate.getMonth()}/${itemDate.getFullYear()}`;

    /// 1 hour or more
    }else if( time_diff > 3599999){
        time_diff = time_diff / 1000 / 60 / 60;
        return Math.round(time_diff) + 'h ago';

    /// 1 minute  or more
    }else if( time_diff > 59999){
        time_diff = time_diff / 1000 / 60;
        return Math.round(time_diff)  + 'm ago';

    /// get seconds
    }else{
        time_diff = time_diff / 1000 ;
        return Math.round(time_diff) + 's ago';
    }
}

export const timeElapsed = (seconds) => {
    let time_diff = seconds;
    let time_elapsed = ''

    /// more than 23 hours
    if( time_diff > 86399){
        let days = time_diff / 24 / 60 / 60
        days = Math.floor(days)
        time_elapsed  += `${days}d `
        time_diff = time_diff - ( days * 24 * 60 * 60 )
    /// 1 hour or more
    }
    if( time_diff > 3599){
        let hours = time_diff / 60 / 60;
        hours = Math.floor(hours)
        time_elapsed  += `${hours}h `
        time_diff = time_diff - ( hours  * 60 * 60 )

    /// 1 minute  or more
    }
    if( time_diff > 59){
        let minutes = time_diff / 60;
        minutes = Math.floor(minutes)
        time_elapsed  += `${minutes}m `
        time_diff = time_diff - ( minutes  * 60 )

    }
    /// get seconds
    if(time_diff >= 0){
        time_elapsed  += `${time_diff}s `
    }

    return `${time_elapsed}`

}