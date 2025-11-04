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
export class CookieConstructor {
    constructor(cookies) {
        this.cookies = cookies;
        // Sctucture set so that we can add cookie properties
        //{ name: {value:'value', expires: int},
        //  name: {value:'value'}
        //}
        this.cookie_names = {
            folder_open: "explorer_curdir_open",
            folder_open_old: "HIVE-user-curdir_open",
            user_name: "userName"
        };
    }

    getCookie = (cookie_name) => {
        let cookie_value = '';
        if(this.cookie_names[cookie_name] === undefined) {
            console.error(`cookie_name ' ${cookie_name} ' is not part of this.cookie_names`)
            return cookie_value;
        }

        switch(cookie_name) {
            case 'folder_open':
                cookie_value = this.handleGetCookie( this.cookie_names[cookie_name] );
                if(cookie_value) break;
                cookie_name = 'folder_open_old'; //no-fallthrough
            default:
                cookie_value = this.handleGetCookie( this.cookie_names[cookie_name] );
        }
        return cookie_value;
    };

    handleGetCookie = (name) => {
        name = `${name}=`;
        let decodedCookie = decodeURIComponent(document.cookie);
        let ca = decodedCookie.split(";");
        for (var i = 0; i < ca.length; i++) {
            var c = ca[i];
            while (c.charAt(0) === " ") {
                c = c.substring(1);
            }
            if (c.indexOf(name) === 0) {
                return c.substring(name.length, c.length);
            }
        }
        return "";
    }

    setCookies = () => {
        let keys = Object.keys(this.cookies);
        keys.forEach(key => {
            this.handleEachCookie(this.cookies[key], this.cookie_names[key]);
        });
    };

    handleEachCookie = (cookie, cookie_name) => {
        let props = Object.keys(cookie);
        let cookie_set_up = '';
        props.forEach((prop, i) => {
            switch(prop){
                case 'value':
                    cookie_set_up = `${cookie_name}=${cookie[prop]}; Secure; SameSite=Strict; Path=/;`
                    break;
                case 'expires':
                    let date = new Date();
                    date.setTime(date.getTime() + cookie[prop] * 24 * 60 * 60 * 1000);
                    cookie_set_up += `expires=${date.toGMTString()};`;
                    break;
                default:
                    cookie_set_up += `${prop}=${cookie[prop]};`;
            }
        });
        document.cookie = cookie_set_up;
    };

}
