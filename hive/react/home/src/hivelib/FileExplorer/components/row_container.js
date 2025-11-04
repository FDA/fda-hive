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

export const DraggableRow = props => {
        const { scrFolder, ...restProps } = props;

        const dragStartHandler = (event) => {
                    const write = scrFolder.perm.act.write;
                    const read = scrFolder.perm.act.read;

                    if (!read && !write){
                        return;
                    }

                    const key = event.target.key;
                    const actions = event.target._action;
                    if(actions.includes('cut')){
                        event.dataTransfer.setData('text/plain', key )
                        event.dataTransfer.effectAllowed = "copyMove";
        //                event
        //                .currentTarget
        //                .style
        //                .visibility = 'hidden';
                    }else{
                        event.dataTransfer.effectAllowed = "none";
                    }
                   event.stopPropagation();
    }

        return (
                <tr
                  {...restProps}
                  draggable="true"
                  onDragStart={ (event) => this.dragStartHandler(event)}
                  onDrag={(event) => event.preventDefault}
                />
        );
};