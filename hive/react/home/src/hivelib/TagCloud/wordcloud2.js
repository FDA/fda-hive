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
import React, { useRef, useEffect, useState } from 'react';
import { getPrefixPlain , getPrefix } from '../modal/url_modal';
import WordCloud from 'wordcloud';

const TagCloud = ({ data, onClick }) => {
    const canvasRef = useRef(null);
    const svgRef = useRef(null);
    const [wordsEx, setWordsEx] = useState([]);


    useEffect(() => {
        fetch( getPrefixPlain() + '/r/home/settings.json')
        .then(response => response.json())
        .then(data => setWordsEx(data.wordsEx))
        .catch(error => console.error('Error loading the settings:', error));
    }, []);

    const words = data.filter(item => item[1] > 2 && !wordsEx.includes(item[0].toLowerCase())).map(item => {
        if (item[1] > 6) {
            return [item[0], 10 + (item[1]-7)/4];
        } else {
            return [item[0], item[1]+3];
        }
      });

      let full_url = getPrefix();
      let full_url_aaa = getPrefixPlain();

    useEffect(() => {
        if (canvasRef.current && words.length > 0) {
            const canvas = canvasRef.current;
            const ctx = canvas.getContext('2d');
            const dpr = window.devicePixelRatio || 1;
            const rect = canvas.getBoundingClientRect();

            // Adjust canvas size for high DPI displays
            canvas.width = rect.width * dpr;
            canvas.height = rect.height * dpr;

            // Scale the context to ensure correct drawing dimensions
            //ctx.scale(dpr, dpr);

            ctx.fillStyle = "#FFFFFF"; // Set to any color, here it's white.
            ctx.fillRect(0, 0, canvas.width, canvas.height);

            //const list = words.map(word => [word.text, word.weight]);
            const list = words;
            //const fontSizeFactor = Math.min(canvas.width / dpr, canvas.height / dpr) / 25;

            WordCloud(canvas, {
                list: list,
                fontFamily: 'Times, serif',
                //fontWeight: 'bold',
                color: 'random-dark',
                rotationRatio: 0.5,
                backgroundColor: '#fff',
                weightFactor: 1.5,
                //weightFactor: () => fontSizeFactor,
                gridSize: 5,
                //gridSize: Math.round(16 * (canvas.width / 1024)),
                click: (item) => {
                    if (onClick) {
                        onClick(item);
                    }
                }
            });
        }
    }, [words, onClick]);

    return <canvas ref={canvasRef} style={{ width: '350px', height: '700px', padding: '0px 3px 0px 3px', msTransformOrigin: 'top left',
        transform: 'scale(1.0)', backgroundColor:'#ffffff' }} />;
};

export default TagCloud;