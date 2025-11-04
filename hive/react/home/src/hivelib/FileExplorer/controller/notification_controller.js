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
import { OpenNotificationWithIcon } from "./../view/notification_view";

export const handleNotifications = (result, action, key) => {
  let note_props = {
    action_title: action,
    key:key
  };
  if (result && result.status === 200) {
    if (typeof result.data === "string") {
      //When data returns HTML
      return null;
    }

    if (JSON.stringify(result.data).length < 3) {
      OpenNotificationWithIcon("error", action, result);
      return null;
    }

    let noError = [];
    let withInfo = [];
    let withError = [];

    Object.keys(result.data).forEach(item => {
      let itemData = result.data[item];

      if (itemData === "}") {
        noError.push({
          key: item,
          ...itemData
        });
      } else if (Object.keys(itemData.data).includes("error")) {
        withError.push({
          key: item,
          ...itemData
        });
      } else if (Object.keys(itemData.data).includes("info")) {
        withInfo.push({
          key: item,
          ...itemData
        });
      } else {
        noError.push({
          key: item,
          ...itemData
        });
      }
    });

    //notification with same keys will overwrite each other

    if ( noError.length > 0 ) {
      note_props.type = "success";
      note_props.key = withError.length + withInfo.length === 0 ? key : `${key}_noError`;

      let more_props = handleHiveResults('noError' , noError);
      OpenNotificationWithIcon({...more_props, ...note_props});
    }

    if ( withError.length > 0 ) {
      note_props.type = "error";
      note_props.key = noError.length + withInfo.length === 0 ? key : `${key}_withError`;

      let more_props = handleHiveResults('withError' , withError);
      OpenNotificationWithIcon({...more_props, ...note_props});
    }

    if ( withInfo.length > 0 ) {
      note_props.type = "info";
      //note_props.key = withError.length + noError.length === 0 ? key : `${key}_withInfo`;
      note_props.key = key;
      let more_props = handleHiveResults('withInfo' , withInfo);
      OpenNotificationWithIcon({...more_props, ...note_props});
    }
  } else if (result && result.status >= 400) {
    note_props.type = "error";
    note_props.message = "Error";
    note_props.description = 'Could not complete task. :(';

    OpenNotificationWithIcon(note_props);
  }

  function handleHiveResults ( result , result_data){
    let message , description;

    if (result === "noError") {
      let signal = result[0].signal;
      let movedItems = result_data.map(item => {
        if (item.name) {
          return item.name;
        } else {
          return item.key;
        }
      });

      let from = result_data[0].data.from;
      let to = result_data[0].data.to;
      if (signal === "trash" || signal === "delete") {
        to = signal;
        signal = "removal";
      }
      message = `${action} Complete`;
      description = signal ? `The ${signal} of ${movedItems} from ${from} to ${to}` : `Moved item(s): ${movedItems} from ${from} to ${to}`;

    } else if (result === "withError") {

      message = `${action} Error`;
      description = "";

      if (Array.isArray(result)) {
        result_data.map(
          item =>
            (description += `${item.name ? item.name : item.key} ${
              item.data.error
            } \n`)
        );
      } else {
        //description = "Could not complete task.";
        description = `${result_data[0].key} ${result_data[0].data.error}`;
      }

    } else if (result === "withInfo") {
      message = `${action} Complete`;
      description = "";
      result_data.map(
        item =>
          (description += `${item.name ? item.name : item.key} ${
            item.data.info
          } \n`)
      );
    }
    return { message , description }

  };

};
