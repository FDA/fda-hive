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
#pragma once
#ifndef sHiveIonhpp
#define sHiveIon_hpp

#include <slib/std.hpp>
#include <slib/core/iter.hpp>
#include <ulib/usr.hpp>
#include <ion/sIon.hpp>
#include <violin/hiveseq.hpp>
#include <violin/hiveal.hpp>


namespace sviolin


{
    class sHiveIonWander : public sIonWander
    {
        protected:
            sUsr * myUser;
        public:
            sHiveIonWander(sUsr * usr, sIon * lion = 0, void * param = 0, idx * searchTraj = 0, traverserCallback func = 0)
                : sIonWander(lion, param, searchTraj, func), myUser(usr)
            {

            }
            idx _loadIonFile(sVec<sHiveId> & hiveIdList, const char * ionFileTmplt);
    };

    class sHiveIonWanderBuilder : public sHiveIonWander {
        public:
            class sHiveIonStatementNode {
                public:
                    class sIonStatementFilter {
                        public:
                            sIonStatementFilter(){
                                field = value = 0;
                                is_regexp = false;
                            }
                            virtual ~sIonStatementFilter () {}
                            virtual bool isPositional (){return false;}
                            const char * field;
                            const char * value;
                            bool is_regexp;
                    };
                protected:
                    sVec<sHiveIonStatementNode*> * _tree;
                    idx _parent_index, _sibling_index, _child_index, _last_label_index, _first_print_index;
                    sStr _label;
                    sDic<const char *> _iterated_fields;

                    const char * _printLabel(idx lbl_cnt){
                        _label.add0();
                        return _label.printf("%" DEC "_%" DEC, scope, lbl_cnt);
                    }

                public:
                    sHiveIonStatementNode(sVec<sHiveIonStatementNode*> * l_tree) : _label(), filters(), output_fields()
                    {
                        is_left_join = false;
                        _tree = l_tree;
                        scope = 0;
                        _join_on_position = _filter_on_seqid = false;
                        _primaryKey = _foreignKey = 0;
                        _first_print_index = _last_label_index = _parent_index = my_index = _sibling_index = _child_index =-1;
                    }
                    virtual ~sHiveIonStatementNode() {
                        for( idx i = 0 ; i < filters.dim() ; ++i ) {
                            delete filters[i];
                            filters[i] = 0;
                        }
                    }
                    bool is_left_join;
                    idx my_index, scope;

                    virtual const char * printIterateStatement(sStr & buf) {
                        if( getParent() )
                            return printJoinStatement(buf);
                        return printFirstStatement(buf);
                    }
                    virtual const char * printFirstStatement(sStr & buf) = 0;
                    virtual const char * printJoinStatement(sStr & buf) = 0;
                    virtual const char * printFilterStatement(sStr & buf) = 0;
                    virtual const char * printOutputStatement(sStr & buf, sStr & collect_print_buf) = 0;
                    virtual const char * printFindStatement(sStr & buf, const char * field) = 0;

                    const char * ensurePrintFindStatement(sStr & buf, const char * field) {
                        if(!field)
                            return 0;
                        const char ** existing_lbl = _iterated_fields(field);
                        if( existing_lbl )
                            return *existing_lbl;
                        const char * current_lbl = printFindStatement(buf,field);
                        _iterated_fields[field] = current_lbl;
                        return current_lbl;
                    }

                    sHiveIonStatementNode * getNextSibling();
                    sHiveIonStatementNode * getChild();
                    sHiveIonStatementNode * getParent();
                    const char * _primaryKey;
                    const char * _foreignKey;
                    bool _join_on_position, _filter_on_seqid;
                    sVec<sIonStatementFilter *> filters;
                    sVec<const char *> output_fields;

                    virtual sIonStatementFilter * makeNewFilter() {
                        return new sIonStatementFilter();
                    }

                    const char * printNextLabel(){
                        return _printLabel(++_last_label_index);
                    }
                    const char * printLastLabel(){
                        return _printLabel(_last_label_index);
                    }

                    virtual const char * getValueLabel(void) = 0;

                    virtual const char * printLastValue()
                    {
                        const char * res = _printLabel(_last_label_index);
                        _label.printf(".%s", getValueLabel());
                        _label.add0();
                        return res;
                    }

                    const char * printFieldLabelValue(const char * field)
                    {
                        const char ** res = _iterated_fields(field);
                        if(!res)
                            return 0;
                        _label.add0();
                        const char * res1 = _label.printf("%s.%s",*res, getValueLabel());
                        return res1;
                    }

                    const char * printPreviousLabel(){
                        return _printLabel(_last_label_index-1);
                    }
                    const char * printFirstLabel() {
                        return _printLabel(1);
                    }

                    virtual bool hasFilters() const {
                        return filters.dim();
                    }

                    virtual const char * getHeader(idx i) const {
                        return (i>=0 && i < output_fields.dim())?output_fields[i]:0;
                    }

                    virtual bool isJoinOnPosition() {return _join_on_position;}

                    void setNextSibling(idx l_index){_sibling_index= l_index;};
                    void setChild(idx l_index){_child_index= l_index;};
                    void setParent(idx l_index){_parent_index= l_index;};

                    static sHiveIonStatementNode * makeNode(const sUsrType2 * node_type, sVec<sHiveIonStatementNode*> * l_tree) {
                        if(node_type->isDescendentOf("u-ionTable")) {
                            return new sHiveIonTableStatementNode(l_tree);
                        }
                        return new sHiveIonAnnotStatementNode(l_tree);
                    }
            };

            class sHiveIonStatementIter : public sIter<sHiveIonStatementNode * const, sHiveIonStatementIter>
            {
                protected:
                    sHiveIonStatementNode * _root,  * _current_node;
                    idx _counter;

                    void init(sHiveIonStatementNode * root) {
                        _root = root;
                        _current_node = _root;
                        _counter = 0;
                    }
                public:
                    inline void requestData_impl() {}
                    inline void releaseData_impl() {}
                    inline bool readyData_impl() const { return true; }
                    inline bool validData_impl() const { return _current_node; }

                    sHiveIonStatementIter(sHiveIonStatementNode * root)
                    {
                        init(root);
                    }

                    void reset(sHiveIonStatementNode * root)
                    {
                        init(root);
                    }

                    inline bool equals_impl(const sHiveIonStatementIter &rhs) const { return rhs._current_node->my_index == _current_node->my_index; }
                    inline bool lessThan_impl(const sHiveIonStatementIter &rhs) const { return _current_node->my_index - rhs._current_node->my_index; }
                    inline bool greaterThan_impl(const sHiveIonStatementIter &rhs) const { return _current_node->my_index - rhs._current_node->my_index; }
                    inline sHiveIonStatementIter& operator++()
                    {
                        ++_counter;
                        sHiveIonStatementNode * tmp_node = _current_node->getChild();
                        if(tmp_node) {
                            _current_node = tmp_node;
                            return *this;
                        }
                        tmp_node = _current_node->getNextSibling();
                        if(tmp_node) {
                            _current_node = tmp_node;
                            return *this;
                        }

                        while( !tmp_node) {
                            tmp_node = _current_node->getParent();
                            if(!tmp_node) {
                                _current_node = 0;
                                break;
                            }
                            _current_node = tmp_node;
                            tmp_node = _current_node->getNextSibling();
                            if(tmp_node) {
                                _current_node = tmp_node;
                                return *this;
                            }
                        }
                        return *this;
                    }
                    inline sHiveIonStatementNode * dereference_impl() const{ return _current_node; }
            };

            class sHiveIonAnnotStatementNode : public sHiveIonStatementNode{
                public:
                    class sIonAnnotStatementFilter : public sIonStatementFilter {
                        public:
                            sIonAnnotStatementFilter() : sIonStatementFilter() {
                                seqID_start = seqID_end = 0;
                                start = end = 0;
                            }

                            const char * seqID_start;
                            idx start;
                            const char * seqID_end;
                            idx end;
                            bool isPositional() {
                                return static_cast<bool>(seqID_start);
                            }
                    };

                    sHiveIonAnnotStatementNode(sVec<sHiveIonStatementNode*> * l_tree)
                        : sHiveIonStatementNode(l_tree)
                    {

                    }

                    static const char * valueLabel;
                    static const char * sequenceFieldName;
                    static const char * positionFieldName;

                    const char * getValueLabel(void) {
                        return valueLabel;
                    }


                    const char * printFindStatement(sStr & buf, const char * type);

                    const char * printFirstStatement(sStr & buf);
                    const char * printJoinStatement(sStr & buf);
                    const char * printJoinOnPositionStatement(sStr & buf);
                    const char * printJoinOnTypeStatement(sStr & buf);
                    const char * printFilterStatement(sStr & buf);
                    const char * printOutputStatement(sStr & buf, sStr & collect_print_buf);

                    bool isRecordDependentStatementAhead(void);

                    const char * printLastValue()
                    {
                        const char * res = _printLabel(_last_label_index);
                        _label.printf(".%s", valueLabel);
                        _label.add0();
                        return res;
                    }

                    const char * getHeader(idx i) const {
                        if(i<0 || i >= output_fields.dim())
                            return 0;
                        if(strcmp(output_fields[i], positionFieldName) == 0) {
                            return "start,end";
                        }
                        return sHiveIonStatementNode::getHeader(i);
                    }

                    sIonStatementFilter * makeNewFilter() {
                        return new sIonAnnotStatementFilter();
                    }
            };

            class sHiveIonTableStatementNode : public sHiveIonStatementNode{
                public:
                    sHiveIonTableStatementNode(sVec<sHiveIonStatementNode*> * l_tree)
                        : sHiveIonStatementNode(l_tree)
                    {

                    }
                    static const char * valueLabel;

                    const char * printFindStatement(sStr & buf, const char * name);

                    const char * printFirstStatement(sStr & buf);
                    const char * printJoinStatement(sStr & buf);
                    const char * printFilterStatement(sStr & buf);
                    const char * printOutputStatement(sStr & buf, sStr & collect_print_buf);

                    const char * getValueLabel(void) {
                        return valueLabel;
                    }
            };

        protected:
            sStr _qry_buf;
            sVec<sHiveId> _ion_scope_2_qry_ID_map;
            sDic<const sUsrType2 *> _ion_qry_ID_2_type_map;

            sVec<sHiveIonStatementNode *> _statement_nodes;
            sHiveIonStatementNode * getRootStatment(void) {
                if(unlikely(!_statement_nodes.dim()))
                    return 0;
                return _statement_nodes[0];
            }

            idx _getScope(sHiveId & myId) {
                for(idx i = 0 ; i < _ion_scope_2_qry_ID_map.dim() ; ++i ) {
                    if ( _ion_scope_2_qry_ID_map[i] == myId )
                        return i+1;
                }
                return 0;
            }
            virtual idx _parseIONIDs(sUsrObjPropsNode * node = 0) = 0;
        public:
            sHiveIonWanderBuilder(sUsr * usr, sIon * lion = 0, void * param = 0, idx * searchTraj = 0, traverserCallback func = 0)
                : sHiveIonWander(usr, lion, param, searchTraj, func), _ion_scope_2_qry_ID_map(),_ion_qry_ID_2_type_map()
            {
            }
            virtual ~sHiveIonWanderBuilder()
            {
            }

            virtual const char * parseQuery() = 0;
            virtual bool populateStatementNodes() = 0;

            virtual idx parsedIONsCnt() { return _ion_scope_2_qry_ID_map.dim(); }

            idx loadIONs(void)
            {
                if( _parseIONIDs() )
                    return _loadIonFile(_ion_scope_2_qry_ID_map, "ion.ion");
                return 0;
            }
            bool compileQuery();
            bool composeQueryHeaders(sStr & buf);
            const char * printOutQuery(){
                return _qry_buf.ptr();
            }

            void resetContainers() {
                _qry_buf.cut(0);
                _statement_nodes.empty();
            }

            void ensureIONScopeMaps()
            {
                if( !parsedIONsCnt() ) {
                    _parseIONIDs();
                }
            }
            const sUsrType2 * getIONTypebyId(const char *  objID) {
                const sUsrType2 ** r = _ion_qry_ID_2_type_map(objID);
                return r?(*r):0;
            }
    };

    class sHiveIonWanderPropBuilder : public sHiveIonWanderBuilder {
        private:
            sUsrObjPropsTree _propTree, *_pPropTree;
            idx _populatePropStatementNodes(sHiveIonStatementNode * st_node = 0, sUsrObjPropsNode * node = 0);

            idx _parseIONIDs(sUsrObjPropsNode * node = 0);

            inline idx _getLvl(sUsrObjPropsNode * node) {
                if (unlikely(!node))
                    return -1;
                if (unlikely(node->isRoot()))
                    return 0;
                static idx propIonLevelPrefix_length = strlen(propIonLevelPrefix);
                const char * lvl_name = node->field()->name();
                idx prop_lvl = 0;
                lvl_name = strstr(lvl_name, propIonLevelPrefix);
                if ( lvl_name )
                    sString::bufscanf(lvl_name + propIonLevelPrefix_length, 0, "%" DEC , &prop_lvl);
                return prop_lvl;
            }

            idx _getScope( sUsrObjPropsNode * node) {
                sHiveId myId;
                node->hiveidvalue(myId);
                return sHiveIonWanderBuilder::_getScope(myId);
            }

            bool _isInJoinLevel(sUsrObjPropsNode * node) {
                static sStr lcl_buf;
                idx my_lvl =_getLvl(node);
                lcl_buf.cut0cut();
                if(my_lvl>=0) {
                    lcl_buf.printf("%s%" DEC,propIonLevelPrefix,my_lvl);
                } else {
                    return true;
                }
                return (strcmp(node->name(),lcl_buf.ptr()) == 0 );
            }

            sUsrObjPropsNode * _getSameLvl(sUsrObjPropsNode * node)
            {
                return node->nextSibling(_getPropLvlName(_getLvl(node)));
            }
            sUsrObjPropsNode * _getNextLvl(sUsrObjPropsNode * node)
            {
                while(node) {
                    if( _isInJoinLevel(node) )
                        break;
                    node = node->parentNode();
                }
                return node?node->firstChild(_getPropLvlName(_getLvl(node)+1)):0;
            }
            sUsrObjPropsNode * _getSameLvlIon(sUsrObjPropsNode * node) {
                if(!node || !node->parentNode()) {
                    return 0;
                }
                sUsrObjPropsNode * next_node = _getSameLvl(node->parentNode());
                return next_node?findNode(next_node,propIonIDName):0;
            }
            sUsrObjPropsNode * _getNextLvlIon(sUsrObjPropsNode * node) {
                sUsrObjPropsNode * next_node = _getNextLvl(node);
                return next_node?findNode(next_node,propIonIDName):0;
            }

            sStr _buf;
            const char * _getPropLvlName(idx lvl) {
                _buf.add0();
                return _buf.printf("%s%" DEC, propIonLevelPrefix, lvl);
            }
        public:
            static const char * propIonLevelPrefix;
            static const char * propIonIDName;
            static const char * propJoinGroupName;
            static const char * propJoinOnPositionName;
            static const char * propJoinPrimaryKeyName;
            static const char * propJoinForeignKeyName;
            static const char * propFilterGroupName;
            static const char * propFilterRangeGroupName;
            static const char * propFilterFieldName;
            static const char * propFilterValueName;
            static const char * propFilterRegexpName;
            static const char * propFilterSeqStartName;
            static const char * propFilterPosStartName;
            static const char * propFilterSeqEndName;
            static const char * propFilterPosEndName;
            static const char * propOutputFieldName;
            static const char * propFilterChoiceName;
        private:
            const char * _composeName(sUsrObjPropsNode * node,const char * nm) {
                if( !node || node->depth() < 0 )
                    return nm;

                idx prop_lvl = _getLvl(node);
                if( prop_lvl >= 0) {
                    _buf.add0();
                    return _buf.printf("%s%" DEC "_%s",propIonLevelPrefix, prop_lvl, nm);
                }
                return nm;
            };
            idx _populateFilterMembers(sHiveIonStatementNode * st_node, sUsrObjPropsNode * node);
            idx _populateOutputMembers(sHiveIonStatementNode * st_node, sUsrObjPropsNode * node);
            bool _populateJoinMembers(sHiveIonWanderPropBuilder::sHiveIonStatementNode * st_node, sUsrObjPropsNode * node);

        public:
            sHiveIonWanderPropBuilder(sUsr * usr, sIon * lion = 0, void * param = 0, idx * searchTraj = 0, traverserCallback func = 0)
                : sHiveIonWanderBuilder(usr, lion, param, searchTraj, func), _propTree(*myUser, getPropTypeName()), _buf()
            {
                _pPropTree = & _propTree;
            }
            virtual ~sHiveIonWanderPropBuilder() {
                for (idx i = 0 ; i < _statement_nodes.dim() ; ++i ) {
                    delete _statement_nodes[i];
                }
            }

            sUsrObjPropsTree * getPropTree() {
                return _pPropTree;
            }

            bool useForm (sVar * pForm) {
                return _propTree.useForm(*pForm);
            }
            bool usePropTable(sVarSet & table) {
                return _pPropTree->useTable(table);
            }

            sHiveIonStatementNode * getStatementNode(idx l_index)
            {
                if ( likely(l_index>_statement_nodes.dim() ) )
                    return _statement_nodes[l_index];
                return 0;
            }
            const char * parseQuery(void);
            bool populateStatementNodes() {
                return _populatePropStatementNodes();
            }
            static const char * getPropTypeName() {
                return "ion_expansion";
            }
            sUsrObjPropsNode * findNode(sUsrObjPropsNode * node,const char * nm) {
                if ( !node ) {
                    return _pPropTree->find(nm);
                }
                const char * composed_name = _composeName(node,nm);
                return node->find(composed_name);
            }
            sUsrObjPropsNode * findNextSiblingNode(sUsrObjPropsNode * node,const char * nm) {
                if ( !node ) {
                    return _pPropTree->find(nm);
                }
                const char * composed_name = _composeName(node,nm);
                return node->nextSibling(composed_name);
            }
            sUsrObjPropsNode * findChildNode(sUsrObjPropsNode * node,const char * nm) {
                if ( !node ) {
                    return _pPropTree->find(nm);
                }
                const char * composed_name = _composeName(node,nm);
                return node->firstChild(composed_name);
            }
    };

    class sHiveIonBase
    {
        protected:
            sUsr * myUser;
            const char * _fileNameWithoutExtension00;

        public:
            idx ionCnt;
            sDic < sIonWander> wanderList;
            sStr pathList00;

        public:

            sHiveIonBase(sUsr * user = 0, const char * objList = 0, const char * file = "ion" __) : _fileNameWithoutExtension00(file)
            {
                init(user, objList);
            }
            virtual ~sHiveIonBase() {}

            virtual const char * getIONFileNameWithoutExtension00() const
            {
                return _fileNameWithoutExtension00;
            }

            virtual void reset(void)
            {
                ionCnt = 0;
                pathList00.cut(0);
                wanderList.empty();
            }

            sHiveIonBase * initAll(const char * typeStr);
            sHiveIonBase * initAll(sUsrType2 * type) { return initAll(type->name());}
            sHiveIonBase * init(sUsr * user, const char * objList=0 );
            sHiveIonBase * init(sUsr * user, sVec<sHiveId> & hiveIDList );

            virtual sHiveIonBase * init(sVec<sUsrObj> & objList) ;
            virtual sHiveIonBase * init(sUsrObj & obj);
            sIonWander * addIonWander(const char * wandername, const char * iql, idx iqllen);
            sIonWander * addIonWander(const char * wandername, const char * iql, ... ){
                sStr buf;
                sCallVarg(buf.vprintf,iql);
                return addIonWander(wandername,buf.ptr(),buf.length());
            }
            idx attachIonWander(sIonWander &wi, const char * iql, idx iqllen) {
                wi.attachIons(pathList00.ptr(0),sMex::fReadonly,ionCnt);
                if( wi.traverseCompile(iql,iqllen ? iqllen : sLen(iql)) )
                    return 0;
                return wi.ionList.dim();
            }
            idx attachIonWander(sIonWander &wi, const char * iql, ... ){
                sStr buf;
                sCallVarg(buf.vprintf,iql);
                return attachIonWander(wi,buf.ptr(),buf.length());
            }

            virtual const char * getHeaders00(sStr & buf, const char * separator = 0 ) {
                buf.cut0cut();
                sIonWander * iw = addIonWander("query","a=1:foreach.type(\"\");print(a.1);");
                iw->traverseRecordSeparator= separator?separator: _ ;
                iw->traverse();
                buf.add(iw->traverseBuf,iw->traverseBuf.length());
                return buf.add0(2);
            }

            void setSeparators(const char * sepField, const char * sepRecord, const char * wandername=0)
            {
                for (idx i=0 ;i < wanderList.dim() ; ++i ) {
                    if(wandername && strcmp(wandername,(const char * ) wanderList.id(i))!=0 )
                        continue;

                    if(sepField)wanderList[i].traverseFieldSeparator=sepField;
                    if(sepRecord)wanderList[i].traverseRecordSeparator=sepRecord;
                }
            }

            sIonWander & operator [](const char * nam)
            {
                return wanderList[nam];
            }
            sIonWander * operator()(const char * nam)
            {
                return wanderList.get((const void*)nam,0,0);
            }

            static sHiveIonBase * make_HiveIon_by_type(const char * type);
            static sHiveIonBase * make_HiveIon_by_type(const sUsrType2 * type);

            static sHiveIonBase * make_HiveIon_by_object(sUsr * user, const char * objs);
            static sHiveIonBase * make_HiveIon_by_object(sUsr * user, sHiveId & m_hiveID);
            static sHiveIonBase * make_HiveIon_by_object(sUsrObj * objs);

    };

    class sHiveIon  : public sHiveIonBase {
        private:
            static const char * listCommands;
        public:
            sHiveIon(sUsr * user = 0, const char * objList = 0, const char * file = "ion" __ )
                : sHiveIonBase(user, objList, file)
            {

            }
            struct geneExpr {
               sDic < sDic < sVec < real > > > samplePassageReplicaDic;

               sDic < idx >  rowIds;
               sMex largeDicBuf;
               sDic < sDic < sMex::Pos  > > valueDic;
               sStr compositePassage;
               bool collapsePassage;
               bool exportStDev;
               geneExpr () {
                 collapsePassage=false;
                 exportStDev=false;
                 compositePassage.cut(0);
               };
           };
            idx retrieveListOfObjects(sVec<sHiveId> * objList, sDir * fileList00, const char * objIDList, const char * typeList, const char * parList00, const char * valList00, const char * filePattern00);
            static idx realValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist , sIon::Bucket * cbBucket);
            static idx anyValStatCallback(sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist , sIon::Bucket * cbBucket);
            void dicDicVecPrint(sIO * buf, sDic < sDic < sVec < real > > > & sPrDic, idx characteristics, idx printMeasurementAsHeader=0 , idx doSort=false, void * params=0);

            idx Cmd(sIO * out, const char * cmd, sVar * pForm);

    };

    class sHiveIonAnnot: public sHiveIonBase {
        public:
            sHiveIonAnnot(sUsr * user = 0 , const char * objList = 0)
                : sHiveIonBase(user, objList, 0)
            {
            }
            virtual const char * getIONFileNameWithoutExtension00() const
            {
                return "ion" __;
            }

            const char * getHeaders00(sStr & buf, const char * separator = 0 ) {
                buf.cut0cut();
                buf.addString("seqID");
                buf.addSeparator(separator);
                buf.addString("pos");
                buf.addSeparator(separator);
                sIonWander * iw = addIonWander("query","a=1:foreach.type(\"\");print(a.1);");
                iw->traverseRecordSeparator= separator?separator: _ ;
                iw->traverse();
                buf.add(iw->traverseBuf,iw->traverseBuf.length());
                return buf.add0(2);
            }

            const char * getSequences00(sStr & buf, const char * separator = 0) {
                buf.cut0cut();
                sIonWander * iw = addIonWander("query","a=1:foreach.seqID(\"\");print(a.1);");
                iw->traverseRecordSeparator= separator?separator: _ ;
                iw->traverse();
                buf.add(iw->traverseBuf,iw->traverseBuf.length());
                return buf.add0(2);
            }
            sHiveIonAnnot * init(sUsr * user, const char * objListStr, bool allowPartial = false);
    };

    class sHiveIonTable: public sHiveIonBase {
        public:
            sHiveIonTable(sUsr * user = 0 , const char * objList = 0)
                : sHiveIonBase(user, objList, 0)
            {

            }
            virtual const char * getIONFileNameWithoutExtension00() const
            {
                return "ion" __;
            }

            const char * getHeaders00(sStr & buf, const char * separator = 0 ) {
                buf.cut0cut();
                sIonWander * iw = addIonWander("query","a=1:foreach.name(\"\");print(a.1);");
                iw->traverseRecordSeparator= separator?separator: _ ;
                iw->traverse();
                buf.add(iw->traverseBuf,iw->traverseBuf.length());
                return buf.add0(2);
            }


    };

    class sHiveIonSeq: public sHiveIonBase {

        public:
            sHiveIonSeq(sUsr * user = 0 , const char * objList = 0, const char * file = 0)
                : sHiveIonBase(user, objList, file)
            {

            }
            virtual const char * getIONFileNameWithoutExtension00() const
            {
                return "ion" __;
            }
            struct infoParams {
                    idx cntStart;
                    idx cnt;
                    idx curIndex;
                    bool printDots;
                    infoParams() {cntStart=curIndex=0;cnt=20;printDots=true;};
            };

            idx annotMapPosition(sStr *output, sDic < sStr > * dic, const char * seqidFrom00,idx pos_start=0, idx pos_end=0, idx countResultMax=sIdxMax, idx startResult=0, idx header=1, sBioal * seqMultipleAlign=0);
            idx standardTraverse(sStr & output, sDic < sStr > * dic, idx countResultMax=20, idx startResult=0, bool printDots=true, idx wanderIndex=-1);
            idx annotMap(sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax=sIdxMax, idx startResult=0, idx contSequencesMax=sIdxMax , idx outPutWithHeader=0, sStr * header=0);

            static idx annotMap(void * hiveIonPointer ,sIO * io, sBioseq * sub, sDic <sStr > * dic,const char * seqidFrom00, idx countResultMax=sIdxMax, idx startResult=0, idx contSequencesMax=sIdxMax ,idx outPutWithHeader=0, sStr * header=0)
            {
                return ((sHiveIonSeq * )hiveIonPointer)->annotMap(io, sub, dic,seqidFrom00, countResultMax, startResult, contSequencesMax,outPutWithHeader, header );
            }
            static idx traverserCallback (sIon * ion, sIonWander * wander, sIonWander::StatementHeader * statement, sIon::RecordResult * reslist, sIon::Bucket * cbBucket);
            static idx locateSeqId(const char * seqId, idx * seqLen);

    };
}


#endif 



