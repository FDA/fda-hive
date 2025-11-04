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
describe('gridstack utils', function() {
    'use strict';

    var utils;

    beforeEach(function() {
        utils = window.GridStackUI.Utils;
    });

    describe('setup of utils', function() {

        it('should set gridstack utils.', function() {
            expect(utils).not.toBeNull();
            expect(typeof utils).toBe('object');
        });

    });

    describe('test toBool', function() {

        it('should return booleans.', function() {
            expect(utils.toBool(true)).toEqual(true);
            expect(utils.toBool(false)).toEqual(false);
        });

        it('should work with integer.', function() {
            expect(utils.toBool(1)).toEqual(true);
            expect(utils.toBool(0)).toEqual(false);
        });

        it('should work with Strings.', function() {
            expect(utils.toBool('')).toEqual(false);
            expect(utils.toBool('0')).toEqual(false);
            expect(utils.toBool('no')).toEqual(false);
            expect(utils.toBool('false')).toEqual(false);
            expect(utils.toBool('yes')).toEqual(true);
            expect(utils.toBool('yadda')).toEqual(true);
        });

    });

    describe('test isIntercepted', function() {
        var src = {x: 3, y: 2, width: 3, height: 2};

        it('should intercept.', function() {
            expect(utils.isIntercepted(src, {x: 0, y: 0, width: 4, height: 3})).toEqual(true);
            expect(utils.isIntercepted(src, {x: 0, y: 0, width: 40, height: 30})).toEqual(true);
            expect(utils.isIntercepted(src, {x: 3, y: 2, width: 3, height: 2})).toEqual(true);
            expect(utils.isIntercepted(src, {x: 5, y: 3, width: 3, height: 2})).toEqual(true);
        });

        it('shouldn\'t intercept.', function() {
            expect(utils.isIntercepted(src, {x: 0, y: 0, width: 3, height: 2})).toEqual(false);
            expect(utils.isIntercepted(src, {x: 0, y: 0, width: 13, height: 2})).toEqual(false);
            expect(utils.isIntercepted(src, {x: 1, y: 4, width: 13, height: 2})).toEqual(false);
            expect(utils.isIntercepted(src, {x: 0, y: 3, width: 3, height: 2})).toEqual(false);
            expect(utils.isIntercepted(src, {x: 6, y: 3, width: 3, height: 2})).toEqual(false);
        });
    });

    describe('test createStylesheet/removeStylesheet', function() {

        it('should create/remove style DOM', function() {
            var _id = 'test-123';

            utils.createStylesheet(_id);

            var style = $('STYLE[data-gs-style-id=' + _id + ']');

            expect(style.length).toEqual(1);
            expect(style.prop('tagName')).toEqual('STYLE');

            utils.removeStylesheet(_id)

            style = $('STYLE[data-gs-style-id=' + _id + ']');

            expect(style.length).toEqual(0);
        });

    });

    describe('test parseHeight', function() {

        it('should parse height value', function() {
            expect(utils.parseHeight(12)).toEqual(jasmine.objectContaining({height: 12, unit: 'px'}));
            expect(utils.parseHeight('12px')).toEqual(jasmine.objectContaining({height: 12, unit: 'px'}));
            expect(utils.parseHeight('12.3px')).toEqual(jasmine.objectContaining({height: 12.3, unit: 'px'}));
            expect(utils.parseHeight('12.3em')).toEqual(jasmine.objectContaining({height: 12.3, unit: 'em'}));
            expect(utils.parseHeight('12.3rem')).toEqual(jasmine.objectContaining({height: 12.3, unit: 'rem'}));
            expect(utils.parseHeight('12.3vh')).toEqual(jasmine.objectContaining({height: 12.3, unit: 'vh'}));
            expect(utils.parseHeight('12.3vw')).toEqual(jasmine.objectContaining({height: 12.3, unit: 'vw'}));
            expect(utils.parseHeight('12.5')).toEqual(jasmine.objectContaining({height: 12.5, unit: 'px'}));
            expect(function() { utils.parseHeight('12.5 df'); }).toThrowError('Invalid height');

        });

        it('should parse negative height value', function() {
            expect(utils.parseHeight(-12)).toEqual(jasmine.objectContaining({height: -12, unit: 'px'}));
            expect(utils.parseHeight('-12px')).toEqual(jasmine.objectContaining({height: -12, unit: 'px'}));
            expect(utils.parseHeight('-12.3px')).toEqual(jasmine.objectContaining({height: -12.3, unit: 'px'}));
            expect(utils.parseHeight('-12.3em')).toEqual(jasmine.objectContaining({height: -12.3, unit: 'em'}));
            expect(utils.parseHeight('-12.3rem')).toEqual(jasmine.objectContaining({height: -12.3, unit: 'rem'}));
            expect(utils.parseHeight('-12.3vh')).toEqual(jasmine.objectContaining({height: -12.3, unit: 'vh'}));
            expect(utils.parseHeight('-12.3vw')).toEqual(jasmine.objectContaining({height: -12.3, unit: 'vw'}));
            expect(utils.parseHeight('-12.5')).toEqual(jasmine.objectContaining({height: -12.5, unit: 'px'}));
            expect(function() { utils.parseHeight('-12.5 df'); }).toThrowError('Invalid height');
        });
    });
});
