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
#include <violin/hiveion.hpp>


using namespace sviolin;


idx sHiveIonWander::_loadIonFile(sVec<sHiveId> & hiveIdList, const char * ionFileTmplt)
{
    if( !hiveIdList.dim() ) {
        return 0;
    }
    sStr tmpPath;
    idx res = 0;
    for(idx i = 0; i < hiveIdList.dim(); ++i) {
        tmpPath.cut(0);
        sUsrObj o(*myUser, hiveIdList[i]);

        o.getFilePathname(tmpPath, ionFileTmplt);
        if( tmpPath.length() ) {
            const char * ipath = strrchr(tmpPath.ptr(), '.');
            if( ipath ) {
                tmpPath.cut0cut(static_cast<idx>(ipath - tmpPath.ptr()), 2);
                res = attachIons(tmpPath.ptr(), sMex::fReadonly, 1);
            }
        }
    }
    return res;
}

bool sHiveIonWanderBuilder::compileQuery() {
    if (!parseQuery()) {
    }

    return traverseCompile(_qry_buf, sLen(_qry_buf));
}

bool sHiveIonWanderBuilder::composeQueryHeaders(sStr & buf) {
    if (!populateStatementNodes()) {
    }

    sStr tmp_buf_info,tmp_buf_headers;
    sHiveIonStatementIter it(getRootStatment());
    for(; it.valid(); ++it) {
        const sHiveIonStatementNode * current_node = *it;
        for(idx i = 0; i < current_node->output_fields.dim(); ++i) {
            tmp_buf_headers.printf(",%s", current_node->getHeader(i));
        }
    }

    if ( !tmp_buf_headers.length() ) {
        return false;
    }

    buf.printf("%s\n",tmp_buf_headers.ptr(1));
    return true;
}

sHiveIonWanderBuilder::sHiveIonStatementNode * sHiveIonWanderBuilder::sHiveIonStatementNode::getNextSibling()
{
    if ( likely(_tree && _sibling_index >=0) ) {
        return *_tree->ptr(_sibling_index);
    }
    return 0;
}
sHiveIonWanderBuilder::sHiveIonStatementNode * sHiveIonWanderBuilder::sHiveIonStatementNode::getChild()
{
    if ( likely(_tree && _child_index >=0) ) {
        return *_tree->ptr(_child_index);
    }
    return 0;
}

sHiveIonWanderBuilder::sHiveIonStatementNode * sHiveIonWanderBuilder::sHiveIonStatementNode::getParent()
{
    if ( likely(_tree && _parent_index >=0) ) {
        return *_tree->ptr(_parent_index);
    }
    return 0;
}


const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::valueLabel = "id";
const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::sequenceFieldName = "seqID";
const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::positionFieldName = "pos";

const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printFirstStatement(sStr & buf) {
    if( hasFilters() )
        return 0;
    buf.printf("%s=%" DEC ":", printNextLabel(), scope);
    const char * next_lbl = printNextLabel();
    buf.printf("foreach.seqID(\"\");%s=%" DEC ":find.annot(seqID=%s.1);",
        next_lbl, scope, printPreviousLabel());
    return next_lbl;
}

const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printJoinStatement(sStr & buf) {
    if( isJoinOnPosition() ) {
        return printJoinOnPositionStatement(buf);
    }
    return printJoinOnTypeStatement(buf);
}
const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printJoinOnPositionStatement(sStr & buf) {
    const char * next_lbl = printNextLabel();
    const char * parent_lbl = getParent()->printLastLabel();
    buf.printf("%s=%" DEC ":find.annot(#range=possort-max,%s.seqID,%s.pos,%s.seqID,%s.pos);",
        next_lbl, scope, parent_lbl, parent_lbl, parent_lbl, parent_lbl);
    return next_lbl;
}
const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printFindStatement(sStr & buf, const char * type) {
    const char * last_lbl = printLastLabel(), * next_lbl = printNextLabel();
    buf.printf("%s=%" DEC ":find.annot(seqID=%s.seqID,record=%s.record,type=\"%s\");",
        next_lbl, scope, last_lbl, last_lbl, type);
    return next_lbl;
}
const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printJoinOnTypeStatement(sStr & buf) {
    getParent()->ensurePrintFindStatement(buf, _foreignKey);
    const char * next_lbl = printNextLabel(), * parent_lbl = getParent()->printLastLabel();
    buf.printf("%s=%" DEC ":find.annot(seqID=%s.seqID,type=\"%s\",id=\"%s\");",
        next_lbl, scope, parent_lbl, _primaryKey, getParent()->printFieldLabelValue(_foreignKey));

    return next_lbl;
}


const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printFilterStatement(sStr & buf) {

    if( isJoinOnPosition() ) {
        const char * last_lbl = printLastLabel();
        bool is_record_dependent_statement_ahead = isRecordDependentStatementAhead();

        buf.printf("unique.1(%s.%s);",last_lbl, (is_record_dependent_statement_ahead?"record":"position") );
    }

    const char * last_lbl = printLastLabel(),  * next_lbl = 0;
    for( idx i = 0 ; i < filters.dim(); ++i) {
        sIonAnnotStatementFilter * current_filter = static_cast<sIonAnnotStatementFilter *>(filters[i]);
        if( current_filter->isPositional() ) {
            next_lbl = printNextLabel();
            buf.printf("%s=%" DEC ":find.annot(#range=possort-max, \"%s\", %" DEC ", \"%s\", %" DEC ");",
                next_lbl, scope, current_filter->seqID_start, current_filter->start, current_filter->seqID_end, current_filter->end);
            last_lbl = next_lbl;
            if( !this->getParent() ) {
                bool is_record_dependent_statement_ahead = isRecordDependentStatementAhead();

                buf.printf("unique.1(%s.%s);",last_lbl, (is_record_dependent_statement_ahead?"record":"pos") );
            }

        } else {
            if( likely(getParent() || i>0 ) ) {

                next_lbl = printNextLabel();
                if( current_filter->value ) {
                    buf.printf("%s=%" DEC ":%s.annot(record=%s.record, type=\"%s\", id=\"%s%s\");",
                            next_lbl, scope, (current_filter->is_regexp?"search":"find"),last_lbl
                            , current_filter->field
                            , (current_filter->is_regexp?"regex:":""), current_filter->value);
                } else {
                    buf.printf("%s=%" DEC ":%s.annot(seqID=%s.seqID,record=%s.record,type=\"%s%s\");",
                            next_lbl, scope, (current_filter->is_regexp?"search":"find"),last_lbl, last_lbl, (current_filter->is_regexp?"regex:":""), current_filter->field);

                }
            } else {
                next_lbl = printNextLabel();
                if( current_filter->value ) {
                    buf.printf("%s=%" DEC ":%s.annot(id=\"%s\", type=\"%s%s\");",
                                next_lbl, scope, (current_filter->is_regexp?"search":"find"), current_filter->value, (current_filter->is_regexp?"regex:":""), current_filter->field);
                } else {
                    buf.printf("%s=%" DEC ":%s.annot(type=\"%s%s\");",
                                next_lbl, scope, (current_filter->is_regexp?"search":"find"), (current_filter->is_regexp?"regex:":""), current_filter->field);
                }
            }
        }
    }

    return next_lbl;
}

const char * sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::printOutputStatement(sStr & buf, sStr & print_buf) {
    _first_print_index = _last_label_index + 1;
    const char * last_lbl = printLastLabel(), * current_lbl = 0;
    bool isPosition = false, isSequence = false;
    for( idx i = 0 ; i < output_fields.dim(); ++i) {
        isPosition = (strcmp(output_fields[i], positionFieldName) == 0);
        isSequence = (strcmp(output_fields[i], sequenceFieldName) == 0);
        if( isPosition || isSequence ) {
            print_buf.printf(",%s.%s", printLastLabel(), isPosition?positionFieldName:sequenceFieldName);
            continue;
        }

        if ( !_iterated_fields(output_fields[i])){
            current_lbl = printNextLabel();
            buf.printf("%s=%" DEC ":", current_lbl, scope);
            buf.printf("find.annot(seqID=%s.seqID,record=%s.record,type=\"%s\");",
                last_lbl, last_lbl, output_fields[i]);
            _iterated_fields[output_fields[i]] = current_lbl;
        } else {
            current_lbl = *_iterated_fields(output_fields[i]);
        }

        print_buf.printf(",%s.%s", current_lbl, getValueLabel());
    }
    return current_lbl;
}

bool sHiveIonWanderBuilder::sHiveIonAnnotStatementNode::isRecordDependentStatementAhead(void)
{
    sHiveIonStatementNode * c_node = getChild();
    bool res = false;

    for(idx i = 0; !res && i < filters.dim(); ++i) {
        if( !filters[i]->isPositional() )
            res = true;
    }

    while( c_node && c_node->filters.dim() && !res ) {
        if( !c_node->isJoinOnPosition() )
            res = true;
        c_node = c_node->getNextSibling();
    }

    for(idx i = 0; !res && i < output_fields.dim(); ++i) {
        if( (strcmp(output_fields[i], positionFieldName) != 0) && (strcmp(output_fields[i], sequenceFieldName) != 0) )
            res = true;
    }

    return res;
}



const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::valueLabel = "value";
const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::printFindStatement(sStr & buf, const char * name) {
    const char * last_lbl = printLastLabel(), * next_lbl = printNextLabel();
    buf.printf("%s=%" DEC ":find.row(tbl=%s.tbl,#R=%s.#R,name=\"%s\");",
        next_lbl, scope, last_lbl, last_lbl, name);
    return next_lbl;
}
const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::printFirstStatement(sStr & buf) {
    if( hasFilters() )
        return 0;
    const char * next_lbl = printNextLabel();
    buf.printf("foreach.tbl(\"\");%s=%" DEC ":find.row(tbl=%s.1);",
        next_lbl, scope, printPreviousLabel());
    return next_lbl;
}

const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::printJoinStatement(sStr & buf) {
    getParent()->ensurePrintFindStatement(buf, _foreignKey);
    const char * next_lbl = printNextLabel();
    buf.printf("%s=%" DEC ":find.row(name=\"%s\",value=%s);",
        next_lbl, scope, _primaryKey, getParent()->printFieldLabelValue(_foreignKey) );

    return next_lbl;
}

const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::printFilterStatement(sStr & buf) {
    const char * last_lbl = 0 , * next_lbl = 0;
    for( idx i = 0 ; i < filters.dim(); ++i) {
        last_lbl = printLastLabel();
        next_lbl = printNextLabel();
        buf.printf("%s=%" DEC ":", next_lbl, scope);

        sIonStatementFilter * current_filter = filters[i];
        if (current_filter->is_regexp)
            buf.printf("search");
        else
            buf.printf("find");
        buf.printf(".row(");
        if( likely(getParent() || i>0) ) {
            buf.printf("#R=%s.#R, ",last_lbl);
        }
        buf.printf("name=\"%s\"",
            current_filter->field);
        if( current_filter->value ) {
            buf.printf(", value=\"%s%s\"",
                (current_filter->is_regexp?"regex:":""),current_filter->value);
        }
        buf.printf(");");
    }
    return next_lbl;
}

const char * sHiveIonWanderBuilder::sHiveIonTableStatementNode::printOutputStatement(sStr & buf, sStr & print_buf)
{
    _first_print_index = _last_label_index + 1;
    const char * last_lbl = printLastLabel(), * current_lbl = 0;
    for( idx i = 0 ; i < output_fields.dim(); ++i) {
        if( !_iterated_fields(output_fields[i]) ) {
            current_lbl = printNextLabel();
            buf.printf("%s=%" DEC ":", current_lbl, scope);
            buf.printf("find.row(tbl=%s.tbl,#R=%s.#R,name=\"%s\");",
                last_lbl, last_lbl, output_fields[i]);
            _iterated_fields[output_fields[i]] = current_lbl;
        } else {
            current_lbl = *_iterated_fields(output_fields[i]);
        }
        print_buf.printf(",%s.%s", current_lbl, getValueLabel());
    }
    return current_lbl;
}

const char * sHiveIonWanderPropBuilder::propIonLevelPrefix = "add_lvl_";
const char * sHiveIonWanderPropBuilder::propIonIDName = "ionID";
const char * sHiveIonWanderPropBuilder::propOutputFieldName = "print";
const char * sHiveIonWanderPropBuilder::propFilterGroupName = "filters";
const char * sHiveIonWanderPropBuilder::propFilterChoiceName = "filters_choice";
const char * sHiveIonWanderPropBuilder::propFilterRangeGroupName = "range_filters";
const char * sHiveIonWanderPropBuilder::propFilterFieldName = "filter_field";
const char * sHiveIonWanderPropBuilder::propFilterValueName = "filter_value";
const char * sHiveIonWanderPropBuilder::propFilterRegexpName = "filter_regexp";
const char * sHiveIonWanderPropBuilder::propFilterSeqStartName = "filter_seq_start";
const char * sHiveIonWanderPropBuilder::propFilterPosStartName = "filter_pos_start";
const char * sHiveIonWanderPropBuilder::propFilterSeqEndName = "filter_seq_end";
const char * sHiveIonWanderPropBuilder::propFilterPosEndName = "filter_pos_end";
const char * sHiveIonWanderPropBuilder::propJoinGroupName = "join";
const char * sHiveIonWanderPropBuilder::propJoinOnPositionName = "join_on_position";
const char * sHiveIonWanderPropBuilder::propJoinPrimaryKeyName = "primary_key";
const char * sHiveIonWanderPropBuilder::propJoinForeignKeyName = "foreign_key";


bool sHiveIonWanderPropBuilder::_populateJoinMembers(sHiveIonWanderPropBuilder::sHiveIonStatementNode * st_node, sUsrObjPropsNode * node)
{
    sUsrObjPropsNode * join_on_position_node = findChildNode(node, propJoinOnPositionName);
    if( join_on_position_node && join_on_position_node->boolvalue() ) {
        st_node->_join_on_position = true;
        return true;
    }

    sUsrObjPropsNode * join_node = findChildNode(node, propJoinGroupName);
    if( !join_node )
        return false;

    for( sUsrObjPropsNode * join_row = join_node?join_node->firstChild():0 ; join_row ; join_row = join_row->nextSibling() ) {
        sUsrObjPropsNode * pk_node = findChildNode(join_row, propJoinPrimaryKeyName);
        sUsrObjPropsNode * fk_node = findChildNode(join_row, propJoinForeignKeyName);
        if( fk_node || pk_node ) {
            if( fk_node && pk_node ) {
                st_node->_primaryKey = pk_node->value();
                st_node->_foreignKey = fk_node->value();
                return true;
            }
        }
    }

    return false;
}

idx sHiveIonWanderPropBuilder::_populateFilterMembers(sHiveIonStatementNode * st_node, sUsrObjPropsNode * node)
{
    sUsrObjPropsNode * filter = findNextSiblingNode(node, propFilterGroupName);

    for( sUsrObjPropsNode * filter_row = filter ? filter : 0 ; filter_row ; filter_row = findNextSiblingNode(filter_row,propFilterGroupName) ) {

        sUsrObjPropsNode * filterChoice_node = findNode(filter_row, propFilterChoiceName);

        if (filterChoice_node) {
            idx byRange = filterChoice_node->ivalue();
            if( byRange==0) {
                sUsrObjPropsNode * field_node = findNode(filter_row, propFilterFieldName);
                sUsrObjPropsNode * value_node = findNode(filter_row, propFilterValueName);
                sUsrObjPropsNode * regexp_node = findNode(filter_row, propFilterRegexpName);
                if (field_node) {
                    sHiveIonStatementNode::sIonStatementFilter * current_filter = st_node->makeNewFilter();
                    current_filter->field = field_node->value();
                    current_filter->value = value_node?value_node->value():0;
                    current_filter->is_regexp = regexp_node ? regexp_node->boolvalue() : false;
                    st_node->filters.vadd(1,current_filter);
                }
            } else {
                sUsrObjPropsNode * seqID_start_node = findNode(filter_row, propFilterSeqStartName);
                sUsrObjPropsNode * pos_start_node = findNode(filter_row, propFilterPosStartName);
                sUsrObjPropsNode * seqID_end_node = findNode(filter_row, propFilterSeqEndName);
                sUsrObjPropsNode * pos_end_node = findNode(filter_row, propFilterPosEndName);

                if( seqID_start_node && pos_start_node) {
                    if( !seqID_end_node )
                        seqID_end_node = seqID_start_node;
                    if( !pos_end_node )
                        pos_end_node = pos_start_node;

                    sHiveIonAnnotStatementNode::sIonAnnotStatementFilter * current_filter =
                        static_cast<sHiveIonAnnotStatementNode::sIonAnnotStatementFilter *>(st_node->makeNewFilter());
                    current_filter->seqID_start = seqID_start_node->value();
                    current_filter->start = pos_start_node->ivalue();
                    current_filter->seqID_end = seqID_end_node->value();
                    current_filter->end = pos_end_node->ivalue();
                    st_node->filters.vadd(1,current_filter);
                }
            }
        }
    }
    return st_node->filters.dim();
}

idx sHiveIonWanderPropBuilder::_populateOutputMembers(sHiveIonStatementNode * st_node, sUsrObjPropsNode * node)
{
    sUsrObjPropsNode * output_field_node = findNextSiblingNode(node, propOutputFieldName);
    while( output_field_node ) {
        *st_node->output_fields.add() = output_field_node->value();
        output_field_node = findNextSiblingNode(output_field_node, propOutputFieldName);
    }
    return st_node->output_fields.dim();
}

idx sHiveIonWanderPropBuilder::_populatePropStatementNodes(sHiveIonStatementNode * st_parent_node, sUsrObjPropsNode * node)
{
    if (st_parent_node && !node)
        return 0;

    node = findNode(node, propIonIDName);
    if( !node )
        return 0;

    idx parent_node_index = -1;
    if( st_parent_node ) {
        parent_node_index = st_parent_node->my_index;
    }

    sHiveIonStatementNode * previous_st_node = 0;
    while( node ) {
        sHiveIonStatementNode ** st_node = _statement_nodes.add();
        *st_node = sHiveIonStatementNode::makeNode(getIONTypebyId(node->value()), &_statement_nodes);
        (*st_node)->setParent(parent_node_index);
        (*st_node)->my_index = _statement_nodes.dim() - 1;
        (*st_node)->scope = _getScope(node);

        if( (*st_node)->getParent() && !_populateJoinMembers(*st_node, node->parentNode()) ) {
        }
        _populateFilterMembers(*st_node, node);
        _populateOutputMembers(*st_node, node);

        if( previous_st_node )
            previous_st_node->setNextSibling((*st_node)->my_index);
        previous_st_node = *st_node;
        if( st_parent_node ) {
            st_parent_node->setChild((*st_node)->my_index);
            st_parent_node = 0;
        }

        _populatePropStatementNodes(*st_node, _getNextLvl(node));

        node = _getSameLvlIon(node);
    }
    return _statement_nodes.dim();
}

void traverseStatementSubTree(sHiveIonWanderBuilder::sHiveIonStatementNode * node, sStr & qry_buf)
{
    sHiveIonWanderBuilder::sHiveIonStatementIter it(node);
    for(; it.valid(); ++it) {
        sHiveIonWanderBuilder::sHiveIonStatementNode * current_node = *it;
        if( current_node->is_left_join )
            qry_buf.printf("check_off;");
        current_node->printIterateStatement(qry_buf);
        current_node->printFilterStatement(qry_buf);
        if( current_node->is_left_join )
            qry_buf.printf("check_on;");
    }

    sStr last_print_statement_buf;
    for(it.reset(node); it.valid(); ++it) {
        sHiveIonWanderBuilder::sHiveIonStatementNode * current_node = *it;
        current_node->printOutputStatement(qry_buf, last_print_statement_buf);
    }
    qry_buf.printf("printCSV(%s);",last_print_statement_buf.ptr(1));
}
bool old_traverseStatementSubTree(sHiveIonWanderBuilder::sHiveIonStatementNode * node, sStr & qry_buf, sStr * collect_print_buf = 0)
{
    if( !node )
        return false;

    if( !collect_print_buf ) {
        if( node->is_left_join )
            qry_buf.printf("check_off;");
        node->printIterateStatement(qry_buf);
        node->printFilterStatement(qry_buf);
        if( node->is_left_join )
            qry_buf.printf("check_on;");
    } else {
        node->printOutputStatement(qry_buf, *collect_print_buf);
    }

    old_traverseStatementSubTree(node->getChild(), qry_buf, collect_print_buf);
    sHiveIonWanderBuilder::sHiveIonStatementNode * sibling = node->getNextSibling();
    while( sibling ) {
        old_traverseStatementSubTree(sibling, qry_buf, collect_print_buf);
        sibling = sibling->getNextSibling();
    }

    if( !collect_print_buf && !node->getParent() ) {
        sStr l_collect_print_buf;
        qry_buf.printf("check_off;");
        old_traverseStatementSubTree(node, qry_buf, &l_collect_print_buf);
        if( l_collect_print_buf.length() )
            qry_buf.printf("printCSV(%s);", l_collect_print_buf.ptr(1));
    }

    return true;
}

idx sHiveIonWanderPropBuilder::_parseIONIDs(sUsrObjPropsNode * node)
{
    if( !node )
        node = _pPropTree;
    node = findNode(node, propIonIDName);
    sUsrObjPropsNode * child = 0;
    while( node ) {
        node->hiveidvalue(*_ion_scope_2_qry_ID_map.add());
        sUsrObj tmp(*myUser, *_ion_scope_2_qry_ID_map.last());
        _ion_qry_ID_2_type_map[_ion_scope_2_qry_ID_map.last()->print()] = tmp.getType();
        child = _getNextLvl(node);
        if( child )
            _parseIONIDs(child);
        node = _getSameLvlIon(node);
    }
    return _ion_scope_2_qry_ID_map.dim();
}


const char * sHiveIonWanderPropBuilder::parseQuery(void)
{
    ensureIONScopeMaps();
    if( !parsedIONsCnt())
        return 0;
    _populatePropStatementNodes(0,_pPropTree);


    traverseStatementSubTree(getRootStatment() ,_qry_buf);


    return _qry_buf.ptr();
}

