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
//# sourceURL=jquery.filed.complex.base.js

$(function () {
    $.widget("ion.field_complex_base", $.ion.field_base, {
        options: {
            spec: {}    //  complex field specification
        },

        keywords: ["_type", "_field", "_abstr", "_limit", "_write", "_vital", "_default", "_plural", "_layout", "_public", "_hidden", "_inherit", "_infect", "descr", "__sub", "__dim", "__start", "__tot", "__cnt"],

        _destroy: function () {
            this.element.empty();
        },

        _collectFields: function (obj) {
            var that = this;

            var properties = {};

            if (obj != null) {
                if (obj.hasOwnProperty('_type') && $.ion.hasType(obj._type)) {
                    //  we are in mode when 'obj' is a real object with type definition, so we should collect properties for that specific type instead
                    obj = $.ion.getType(obj._type);
                }

                if (obj._inherit != null) {
                    if (obj._inherit instanceof Array) {
                        $(obj._inherit).each(function (index, name) {
                            if ($.ion.hasType(name)) {
                                $.extend(properties, that._collectFields($.ion.getType(name)))
                            }
                            else {
                                console.error('Unknown type: ' + name);
                            }
                        });
                    }
                    else {
                        if ($.ion.hasType(obj._inherit)) {
                            $.extend(properties, that._collectFields($.ion.getType(obj._inherit)))
                        }
                        else {
                            console.error('Unknown type: ' + obj._inherit);
                        }
                    }
                }

                if (obj._field != null) {
                    for (var key in obj._field) {
                        if (this.keywords.indexOf(key) > -1)
                            continue;

                        if (obj._field[key].hasOwnProperty('_limit') && !obj._field[key].hasOwnProperty('_type')) {
                            obj._field[key]._type = 'string';
                        }

                        if (obj._field[key].hasOwnProperty('_type')) {
                            properties[key] = obj._field[key];
                        }
                        else {
                            properties[key] = obj._field[key];
                        }
                    }
                }
                else if (obj._field == null && !obj.hasOwnProperty('_type')) {
                    for (var key in obj) {
                        if (this.keywords.indexOf(key) > -1)
                            continue;

                        properties[key] = obj[key];
                    }
                }
            }

            return properties;
        },

        _onChange: function () {
        },

        _displayControl: function (name, spec) {
            if (spec == null)
                return;

            var field = $(document.createElement('div'))
                            .addClass('field-container')
                            .attr({
                                'data-name': name,
                                'data-sub': spec.__sub,
                                'data-type': spec._type
                            });

            switch (spec._type) {
                case 'int':
                    field.field_int({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'uint':
                    field.field_uint({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'memory':
                    field.field_memory({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'string':
                    field.field_string({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'hiveid':
                    field.field_hiveid({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'password':
                    field.field_password({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'email':
                    field.field_email({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'bool':
                    field.field_bool({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'text':
                    field.field_text({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'cmdline':
                    field.field_cmdline({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'datetime':
                    field.field_datetime({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                case 'keyval':
                    field.field_keyval({ recordviewer: this.options.recordviewer, name: name, spec: spec });
                    break;
                default:
                    if (spec.hasOwnProperty('_type') && $.ion.hasType(spec._type)) {
                        field.field_complex({
                            recordviewer: this.options.recordviewer,
                            name: name,
                            spec: spec,
                            collapsed: this.options.collapsed
                        });
                    }
                    else {
                        if (typeof spec === 'string' || typeof spec === 'number' || typeof spec === 'boolean') {
                            //field.field_const({ value: spec });
                        }
                        else if (typeof spec === 'object') {
                            field.field_complex({
                                recordviewer: this.options.recordviewer,
                                name: name,
                                spec: spec,
                                collapsed: this.options.collapsed
                            });
                        }
                    }

                    break;
            }

            return field;
        },

        isPrimitive: function (property) {
            if (property != null) {
                switch (property._type) {
                    case 'int':
                    case 'uint':
                    case 'memory':
                    case 'string':
                    case 'hiveid':
                    case 'password':
                    case 'email':
                    case 'bool':
                    case 'text':
                    case 'cmdline':
                    case 'datetime':
                    case 'keyval':
                        return true;
                    default:
                        if ($.ion.hasType(property._type)) {
                            return false;
                        }
                        else {
                            if (typeof property === 'string' || typeof property === 'number' || typeof property === 'boolean') {
                                return true;
                            }
                            else if (typeof property === 'object') {
                                return false;
                            }
                        }
                }
            }

            return false;
        },

        isComplex: function(property) {
            return !this.isPrimitive(property);
        },

        _setItem: function (elem, val) {
            if (val.hasOwnProperty('_id')) {
                $(elem).attr({ 'data-id': val._id });
            }
            else {
                $(elem).removeAttr('data-id');
            }

            for (var name in this.fields) {
                if (val.hasOwnProperty(name)) {
                    var field = this.fields[name];

                    var fieldControl = $('div[data-name="' + name + '"]', elem).first();

                    switch (field._type) {
                        case 'int':
                            fieldControl.field_int('setValue', val[name]);
                            break;
                        case 'uint':
                            fieldControl.field_uint('setValue', val[name]);
                            break;
                        case 'memory':
                            fieldControl.field_memory('setValue', val[name]);
                            break;
                        case 'string':
                            fieldControl.field_string('setValue', val[name]);
                            break;
                        case 'hiveid':
                            fieldControl.field_hiveid('setValue', val[name]);
                            break;
                        case 'password':
                            fieldControl.field_password('setValue', val[name]);
                            break;
                        case 'email':
                            fieldControl.field_email('setValue', val[name]);
                            break;
                        case 'bool':
                            fieldControl.field_bool('setValue', val[name]);
                            break;
                        case 'text':
                            fieldControl.field_text('setValue', val[name]);
                            break;
                        case 'cmdline':
                            fieldControl.field_cmdline('setValue', val[name]);
                            break;
                        case 'datetime':
                            fieldControl.field_datetime('setValue', val[name]);
                            break;
                        case 'keyval':
                            fieldControl.field_keyval('setValue', val[name]);
                            break;
                        default:
                            if ($.ion.hasType(field._type)) {
                                fieldControl.field_complex('setValue', val[name]);
                            }
                            else {
                                //fieldControl.text('Unknown type: ' + property._type);
                            }
                            break;
                    }
                }
            }
        },

        _getItem: function (elem) {
            var val = {};
            
            if ($(elem).attr('data-id')) {
                val['_id'] = $(elem).data('id');
            }

            for (var name in this.fields) {
                var field = this.fields[name];

                var fieldControl = $('div[data-name="' + name + '"]', $(elem)).first();

                switch (field._type) {
                    case 'int':
                        val[name] = fieldControl.field_int('getValue');
                        break;
                    case 'uint':
                        val[name] = fieldControl.field_uint('getValue');
                        break;
                    case 'memory':
                        val[name] = fieldControl.field_memory('getValue');
                        break;
                    case 'string':
                        val[name] = fieldControl.field_string('getValue');
                        break;
                    case 'hiveid':
                        val[name] = fieldControl.field_hiveid('getValue');
                        break;
                    case 'password':
                        val[name] = fieldControl.field_password('getValue');
                        break;
                    case 'email':
                        val[name] = fieldControl.field_email('getValue');
                        break;
                    case 'bool':
                        val[name] = fieldControl.field_bool('getValue');
                        break;
                    case 'text':
                        val[name] = fieldControl.field_text('getValue');
                        break;
                    case 'cmdline':
                        val[name] = fieldControl.field_cmdline('getValue');
                        break;
                    case 'datetime':
                        val[name] = fieldControl.field_datetime('getValue');
                        break;
                    case 'keyval':
                        val[name] = fieldControl.field_keyval('getValue');
                        break;
                    default:
                        //if ($.ion.hasType(field._type)) {
                            val[name] = fieldControl.field_complex('getValue');
                        //}
                        //else {
                            //fieldControl.text('Unknown type: ' + property._type);
                        //}
                        break;
                }
            }
            
            return val;
        },
        
        setValue: function (val) {
            var self = this;

            if (this.isPlural()) {
                //  treat val as list of Ids or single Id...

                this.container.empty();

                if (val instanceof Array) {
                    for (var i = 0; i < val.length; i++) {
                        var v = val[i];

                        if(typeof val === 'int') {
                            var id = v;
                            
                            var itemControl = this.generateControl();
                            this.container.append(itemControl);

                            $(itemControl).attr({ 'data-id': id });

                            $.ion.lookup(id, function (value) {
                                self._setItem($('[data-id=' + value._id + ']', self.container), value);
                            });
                        }
                        else if (typeof v === 'object') {
                            var itemControl = this.generateControl();
                            this.container.append(itemControl);

                            this._setItem(itemControl, v);
                        }
                    }
                }
                else if (typeof val === 'int') {
                    var itemControl = this.generateControl();
                    this.container.append(itemControl);

                    $(itemControl).attr({ 'data-id': val });

                    $.ion.lookup(val, function (value) {
                        self._setItem($('[data-id=' + value._id + ']', self.container), value);
                    });
                }
                else if (typeof val === 'object') {
                    var itemControl = this.generateControl();
                    this.container.append(itemControl);

                    this._setItem(itemControl, val);
                }
            }
            else {
                //  treat val as object with properties...
                this._setItem(this.field, val);
            }
        },

        getValue: function() {
            var self = this;
            
            if (this.isPlural()) {
                var res = [];
                
                this.container.children('.complex').each(function() {
                    res.push(self._getItem(this));
                });
                
                return res;
            }
            else {
                return this._getItem(this.field);
            }
        },
        
        addControl: function () {
            var self = this;

            var control = this.generateControl();

            control.focusin(function() {
                //    save initial value in case we will have to undo it...
                //if(!$(this).data('initial-value')) {
                //    self.setInitialValue();
                //}
            })
            .change(function () {
                if($('.changed', this).length == 0)
                    $(this).removeClass('changed');
                else
                    $(this).addClass('changed');
            })
            .on({
                'delete-field': function() {
                    var container = $(this).parent();

                    $(this).remove();

                    $(container).trigger('change-field');

                    return false;
                }
            })
            .on('rollback-field', function() {
                $(this).change();

                return false;
            });

            this.container.append(control);

            if (this.options.recordviewer != null) {
                $(document).ready(function() {
                    self._evalProcessor();
                });
            }

            return control;
        },
    });

}(jQuery));
