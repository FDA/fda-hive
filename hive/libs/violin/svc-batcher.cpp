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

#include <violin/svc-batcher.hpp>
#include <violin/hivetools.hpp>
#include <slib/utils/json/printer.hpp>

/*! To keep memory usage with multiple batch parameters reasonably low, we use two main data structures:
 *  BatchingIterator which generates forms for submitting batch computations one-by-one, and a compact
 *  BatchingDescriptor structure for the read-only data that BatchingIterator requires while running.
 *  We do not want to keep all the forms in memory at once! */
class BatchingDescriptor
{
private:
    //! One value of one batch typefield
    struct BatchValue
    {
        const sUsrObjPropsNode * value_node; //! node with this value in BatchingDescriptor::tree
        const sUsrObjPropsNode * branch_node; //! most distant ancestor node of value_node whose only batched value descendent is value_node
        idx singleton_split_offset; //! offset of the value if it's an element in a ';'-separated string
        idx label_offset; //! offset in BatchingDescriptor::buf of a human-readable label for this value

        BatchValue()
        {
            value_node = branch_node = 0;
            singleton_split_offset = label_offset = -1;
        }
    };

    //! One batch typefield
    struct BatchField
    {
        idx iname; //! index of name in BatchingDescriptor::name2field
        idx batch_cnt; //! number of batches of values of this field (may be further clamped by the BatchList)
        idx batch_size; //! max number of values in each batch
        idx values_start; //! index of first value of this typefield in BatchingDescriptor::batch_values
        idx values_cnt; //! total number of values of this typefield in BatchingDescriptor::batch_values

        BatchField()
        {
            batch_cnt = batch_size = values_cnt = 0;
            values_start = iname = -1;
        }
    };

    //! A list of batch typefields whose values will be combined into batches sequentially (non-combinatorially)
    struct BatchList
    {
        idx batch_cnt; //! number of batches for this list
        idx fields_start; //! index of first typefield for this list in BatchingDescriptor::batch_fields
        idx fields_cnt; //! total number of typefields in BatchingDescriptor::batch_fields
        bool saturated; //! whether to force every batch for this list to contain at least one value from every field

        BatchList()
        {
            batch_cnt = fields_cnt = 0;
            fields_start = -1;
            saturated = false;
        }
    };

    //! type of param for BatchingDescriptor::addToFormWorker()
    struct AddToFormWorkerParam
    {
        const BatchingDescriptor * desc;
        const BatchValue * bv;
        sVar * pForm;

        AddToFormWorkerParam(const BatchingDescriptor * desc_, const BatchValue * bv_, sVar * pForm_): desc(desc_), bv(bv_), pForm(pForm_) {}
    };

    struct BatchFieldIndex
    {
        idx ilist;
        idx ifld;
        BatchFieldIndex()
        {
            ilist = ifld = -1;
        }
    };

    sUsr & user;
    sQPrideProc * proc;
    sUsrObjPropsTree tree;
    sStr buf;
    mutable sStr keybuf; //! scratch space for printFormKey()
    sDic<BatchFieldIndex> name2field; //! map field name -> index of list and field in the list
    sVec<BatchList> batch_lists; //! flat list of batch typefield lists
    sVec<BatchField> batch_fields; //! flat list of all batch typefields
    sVec<BatchValue> batch_values; //! flat list of batch field values for all batch fields
    const sUsrObjPropsNode * name_node; //! "name" field node in tree
    const sUsrObjPropsNode * reqPriority_node; //! "reqPriority" field node in tree
    regex_t submitter_batch_re; //! regexp for detecting "-batch" suffix in "cmdMode" param in "submitter" field
    sVar skel_form; //! form values which will be the same in all batch-created objects

    //! for sorting batch values in the same order as their fields in batch_fields
    static idx cmpBatchValues(void * param, void * arr, idx i1, idx i2);

    //! callback for BatchValue::branch_node->find()
    static sUsrObjPropsNode::FindStatus addToFormWorker(const sUsrObjPropsNode & node, void * param);

    //! get index in batch_values of the value for a given field, given batch #, and value # in that batch
    idx getBatchValueIndex(idx ilist, idx ifld, idx ibatch, idx ivalue) const
    {
        const BatchList & lst = batch_lists[ilist];
        const BatchField & fld = batch_fields[lst.fields_start + ifld];
        return fld.values_start + ibatch * fld.batch_size + ivalue;
    }

    //! check if ivalue is a valid value # for a given batch # for the given field
    bool validIndex(idx ilist, idx ifld, idx ibatch, idx ivalue) const
    {
        if( unlikely(ilist >= batch_lists.dim()) ) {
            return false;
        }
        const BatchList & lst = batch_lists[ilist];
        if( unlikely(ifld >= lst.fields_cnt) ) {
            return false;
        }
        const BatchField & fld = batch_fields[lst.fields_start + ifld];
        idx ibatch_irregular = sMin<idx>(fld.batch_cnt, lst.batch_cnt) - 1; // index of (possible) batch that doesn't contain exactly fld.batch_size values
        return ibatch < lst.batch_cnt &&
            (ivalue < fld.batch_size || ibatch == ibatch_irregular) &&
            ivalue + ibatch * fld.batch_size < fld.values_cnt;
    }

    //! generate human-readable label for bv and store it in buf at offset bv->label_offset
    void makeBatchLabel(BatchValue * bv);

public:
    BatchingDescriptor(sUsr & user_, sQPrideProc * proc_): user(user_), tree(user_, (const char *)0)
    {
        proc = proc_;
        name_node = 0;
        reqPriority_node = 0;
        regcomp(&submitter_batch_re, "(&?cmdMode=)([^=&]+[^=&-])?(-?batch)", REG_EXTENDED);
    }

    ~BatchingDescriptor()
    {
        regfree(&submitter_batch_re);
    }

    bool init(sVar * pForm, const char * svcToSubmit, const char * svcTitle, sStr * err);

    //! number of lists of fields to batch over
    idx dimLists() const { return batch_lists.dim(); }

    //! number of fields in a list
    idx dimFields(idx ilist) const { return ilist < batch_lists.dim() ? batch_lists[ilist].fields_cnt : 0; }

    //! number of batches for the specified field list
    idx dimBatches(idx ilist) const
    {
        if( unlikely(ilist >= batch_lists.dim()) ) {
            return 0;
        }
        return batch_lists[ilist].batch_cnt;
    }

    //! number of values for a specified field in the specified batch of a field list
    idx dimValues(idx ilist, idx ifld, idx ibatch) const
    {
        if( unlikely(ilist >= batch_lists.dim()) ) {
            return 0;
        }
        const BatchList & lst = batch_lists[ilist];
        if (unlikely(ifld >= lst.fields_cnt)) {
            return 0;
        }
        const BatchField & fld = batch_fields[lst.fields_start + ifld];

        idx ibatch_irregular = sMin<idx>(fld.batch_cnt, lst.batch_cnt) - 1; // index of (possible) batch that doesn't contain exactly fld.batch_size values
        if( ibatch < ibatch_irregular ) {
            return fld.batch_size;
        } else if( ibatch < lst.batch_cnt && ibatch == ibatch_irregular ) {
            return fld.values_cnt - fld.batch_size * ibatch;
        } else {
            return 0;
        }
    }

    //! human-readable label for the given value in given batch # for given field, or 0 on failure
    const char * getBatchLabel(idx ilist, idx ifld, idx ibatch, idx ivalue) const
    {
        return validIndex(ilist, ifld, ibatch, ivalue) ? buf.ptr(batch_values[getBatchValueIndex(ilist, ifld, ibatch, ivalue)].label_offset) : 0;
    }

    //! request priority
    idx getReqPriority(idx cur_batch_step) const
    {
        // absolute value so different sub-requests in one submission will have different priorities
        idx reqPriority = reqPriority_node ? sAbs(reqPriority_node->ivalue()) : 0;
        // large offset to ensure batched requests are scheduled after normal requests
        reqPriority += 1000;
        // if the total batch count is large, spread submissions over a range of priorities to avoid thundering herd effect
        // factor of 3 to help sub-requests from one submission complete before other submissions' sub-requests are scheduled
        return reqPriority + floor(3 * log(1 + cur_batch_step));
    }

    //! print a given value in given batch # for given field (and also other values in the same branch_node) into the form
    bool addToForm(sVar * pForm, idx ilist, idx ifld, idx ibatch, idx ivalue) const;

    //! print prop-format sVar key for field value with given name and path into keybuf
    const char * printFormKey(const char * name, const char * path) const
    {
        keybuf.cut(0);
        keybuf.addString("prop.");
        keybuf.addString(tree.objTypeName());
        keybuf.addString(".");
        keybuf.addString(name);
        if (sLen(path)) {
            keybuf.addString(".");
            keybuf.addString(path);
        }
        return keybuf.ptr();
    }

    //! form values which will be the same in all batch-created objects
    const sVar * getSkeletonForm() const { return &skel_form; }

    //! print prop-format sVar key for "name" field value into keybuf
    const char * printFormLabelKey() const
    {
        return printFormKey("name", name_node ? name_node->path() : 0);
    }

    //! print prop-format sVar key for "reqPriority" field value into keybuf
    const char * printFormReqPriorityKey() const
    {
        return printFormKey("reqPriority", reqPriority_node ? reqPriority_node->path() : 0);
    }

    //! for debugging
    const char * printDump(sStr * s=0) const;
};

//! Iterates over BatchingDescriptor without modifying it and generates forms for submission one by one
class BatchingIterator
{
private:
    sVec<idx> batches; //! map from field list # to value batch # for that list at the current iteration step
    const BatchingDescriptor * desc;
    idx cur_it, max_it;

public:
    BatchingIterator(const BatchingDescriptor * desc_=0): batches(sMex::fExactSize|sMex::fSetZero) { init(desc_); }

    void init(const BatchingDescriptor * desc_)
    {
        desc = desc_;
        batches.cut(0);
        if (desc) {
            batches.resize(desc->dimLists());
        }
        cur_it = 0;
        max_it = 0;
        for (idx ilist=0; ilist<batches.dim(); ilist++) {
            if (max_it) {
                max_it *= desc->dimBatches(ilist);
            } else {
                max_it = desc->dimBatches(ilist);
            }
        }
    }

    //! did we finish iterating?
    operator bool() const { return cur_it < max_it; }

    //! current iteration step
    idx getCurStep() const { return cur_it; }

    //! move to next iteration step
    BatchingIterator & operator ++()
    {
        for (idx ilist=batches.dim() - 1; ilist>=0; ilist--) {
            if (ilist+1 < batches.dim()) {
                batches[ilist+1] = 0;
            }
            if (++batches[ilist] < desc->dimBatches(ilist)) {
                break;
            }
        }

        cur_it++;

        return *this;
    }

    //! number of iteration steps
    idx maxIterations() const { return max_it; }

    //! print submission form for current iteration step
    sVar * makeForm(sVar * pform, const char * svcToSubmit) const;
};

// Sort batch values in same order as batch field lists and their fields
idx BatchingDescriptor::cmpBatchValues(void * param, void * arr, idx i1, idx i2)
{
    BatchingDescriptor * desc = static_cast<BatchingDescriptor*>(param);
    BatchValue * batch_values = static_cast<BatchValue*>(arr);

    BatchFieldIndex * fld_ind1 = desc->name2field.get(batch_values[i1].value_node->name());
    BatchFieldIndex * fld_ind2 = desc->name2field.get(batch_values[i2].value_node->name());

    if( idx d = fld_ind1->ilist - fld_ind2->ilist ) {
        return d;
    } else if( idx d = fld_ind1->ifld - fld_ind2->ifld ) {
        return d;
    } else if( idx d = batch_values[i1].value_node->treeIndex() - batch_values[i2].value_node->treeIndex() ) {
        return d;
    }
    return batch_values[i1].singleton_split_offset - batch_values[i2].singleton_split_offset;
}

void BatchingDescriptor::makeBatchLabel(BatchValue * bv)
{
    bv->label_offset = buf.length();

    sHiveId objId;
    bv->value_node->hiveidvalue(objId);
    sUsrObj obj(user, objId);
    if (!sLen(obj.propGet("name", &buf))) {
        const char * val = bv->singleton_split_offset >= 0 ? buf.ptr(bv->singleton_split_offset) : bv->value_node->value();
        buf.printf("%s %s", bv->value_node->name(), val);
    }
    buf.add0();
}

// Mark descendents of batch branch node as non-zero in batch_descendents, so we can separate out nodes that are not in any batch branch
static sUsrObjPropsNode::FindStatus markDescendentsCb(const sUsrObjPropsNode & node, void * param)
{
    sVec<idx> * batch_descendents = static_cast<sVec<idx>*>(param);
    idx inod = node.treeIndex();
    if (inod >= 0 && *batch_descendents->ptr(inod) == 0) {
        *batch_descendents->ptr(inod) = -1;
    }
    return sUsrObjPropsNode::eFindContinue;
}

bool BatchingDescriptor::init(sVar * pForm, const char * svcToSubmit, const char * svcTitle, sStr * err)
{
    sStr print_buf;

    tree.empty(true);
    buf.cut(0);
    batch_fields.empty();
    batch_values.cut(0);
    name_node = 0;
    skel_form.empty();

    if (!user.propSet(*pForm, *err, tree.getTable())) {
        if (!sLen(err->ptr())) {
            err->addString("Submitted form could not be parsed");
        }
        return false;
    }

    if (!tree.useTable(tree.getTable()) || !tree.initialized()) {
        err->addString("Submitted form is invalid or user is not logged in");
        return false;
    }

    for(const sUsrObjPropsNode * batch_node = tree.find("batch"); batch_node; batch_node = batch_node->nextSibling("batch")) {
        const sUsrObjPropsNode * batch_array_node = batch_node->find("batch_list");
        if( !batch_array_node || !batch_array_node->dim() ) {
            proc->logOut(sQPrideBase::eQPLogType_Warning, "'batch' tree node with path %s: 'batch_list' is empty, skipping", batch_node->path());
            continue;
        }

        idx ilist = batch_lists.dim();
        BatchList * lst = batch_lists.add(1);
        lst->saturated = batch_node->findBoolValue("batch_saturated");

        // Find names of batch parameters
        idx irow = 0;
        for(const sUsrObjPropsNode * batch_row = batch_array_node->firstChild(); batch_row; batch_row = batch_row->nextSibling(), irow++) {
            const sUsrObjPropsNode * batch_name_node = batch_row->find("batch_param");
            const sUsrObjPropsNode * batch_size_node = batch_row->find("batch_value");

            const char * batch_name = batch_name_node ? batch_name_node->value() : 0;
            idx batch_size = batch_size_node ? sMax<idx>(0, batch_size_node->ivalue()) : 0;
            if( !batch_name || !batch_name[0] ) {
                proc->logOut(sQPrideBase::eQPLogType_Warning, "'batch_list' tree node with path %s, row %"DEC": 'batch_param' is empty, skipping", batch_array_node->path(), irow);
                continue;
            }
            if( !batch_size ) {
                proc->logOut(sQPrideBase::eQPLogType_Warning, "'batch_list' tree node with path %s, row %"DEC": 'batch_value' is empty, skipping", batch_array_node->path(), irow);
                continue;
            }

            if( name2field.get(batch_name) ) {
                err->printf("Batch parameter '%s' was listed multiple times", batch_name);
                return false;
            }

            if( lst->fields_cnt == 0 ) {
                lst->fields_start = batch_fields.dim();
            }
            idx ifld = lst->fields_cnt++;
            BatchField * fld = batch_fields.add(1);
            fld->batch_size = batch_size;

            BatchFieldIndex * fld_index = name2field.setString(batch_name, 0, &fld->iname);
            fld_index->ilist = ilist;
            fld_index->ifld = ifld;
        }

        // ignore "batch" nodes that didn't have any valid "batch_param" descendents
        if( lst->fields_cnt < 1 ) {
            proc->logOut(sQPrideBase::eQPLogType_Warning, "'batch' tree node with path %s: 'batch_list' had no valid rows, skipping", batch_node->path());
            batch_lists.cut(ilist);
        }
    }

    if( !batch_lists.dim() || !batch_fields.dim() ) {
        err->addString("No valid 'batch_param' and 'batch_value' pairs found");
        return false;
    }

    print_buf.cut0cut();
    for(idx ilst = 0; ilst < batch_lists.dim(); ilst++) {
        BatchList & lst = batch_lists[ilst];
        print_buf.addString(ilst ? " x [" : "[");
        for(idx ifld = 0; ifld < lst.fields_cnt; ifld++) {
            BatchField & fld = batch_fields[lst.fields_start + ifld];
            if( ifld ) {
                print_buf.addString(", ");
            }
            print_buf.addString(static_cast<const char *>(name2field.id(fld.iname)));
        }
        print_buf.addString("]");
    }
    proc->logOut(sQPrideBase::eQPLogType_Info, "Batch field lists: %s", print_buf.ptr());

    // Perform any tree modifications before creating batch_descendents to ensure node pointers and descendent counts are not invalidated
    if( !tree.find("reqPriority") ) {
        // ensure we can get a proper path for reqPriority node
        tree.push("reqPriority", 0);
    }

    // We want to find prop subtrees controlled by batched nodes: i.e. max subtrees
    // that contain exactly 1 batched prop node.
    // To do this, we count the number of descendent batch prop nodes of each node,
    // and then find where the number of descendents changes from >1 to 1.
    sVec<idx> batch_descendents(sMex::fSetZero); // if > 0, number of descendent batch nodes; if < 0, descendent of a node which has only one descendent batch node.
    batch_descendents.resize(tree.dimTree());
    for(idx inod = 0; inod < tree.dimTree(); inod++) {
        const sUsrObjPropsNode * node = tree.getNodeByIndex(inod);
        if( BatchFieldIndex * fld_index = name2field.get(node->name()) ) {
            BatchList & lst = batch_lists[fld_index->ilist];
            BatchField & fld = batch_fields[lst.fields_start + fld_index->ifld];
            if( fld.values_cnt++ == 0 ) {
                fld.values_start = batch_values.dim();
            }
            BatchValue * bv = batch_values.add(1);
            bv->value_node = node;
            makeBatchLabel(bv);

            for(; node; node = node->parentNode()) {
                idx jnod = node->treeIndex();
                if( jnod >= 0 ) {
                    batch_descendents[jnod]++;
                }
            }
        }
    }

    // special case: singleton batch values get string-splitted for batching
    sStr splitbuf;
    for (idx abs_ifld=0; abs_ifld<batch_fields.dim(); abs_ifld++) {
        BatchField * bf = batch_fields.ptr(abs_ifld);
        if (bf->values_cnt == 1) {
            BatchValue * bv = batch_values.ptr(bf->values_start);
            const sUsrObjPropsNode * node = bv->value_node;
            if (sLen(node->value()) && strchr(node->value(), ';')) {
                splitbuf.cut(0);
                sString::searchAndReplaceSymbols(&splitbuf, node->value(), 0, ";", 0, 0, true, true, false, true);
                const char * subval = splitbuf.ptr();

                while (subval && *subval) {
                    idx split_offset = buf.length();

                    // batch_size > 1 means we concatenated batch_size many subvalues with ";"
                    for (idx iv=0; subval && *subval && iv < bf->batch_size; iv++, subval = sString::next00(subval)) {
                        if (iv) {
                            buf.addString(";");
                        }
                        buf.addString(subval);
                    }

                    if (split_offset < buf.length()) {
                        buf.add0();

                        if (bv->singleton_split_offset >= 0) {
                            bv = batch_values.add(1);
                            bv->value_node = node;
                            bf->values_cnt++;
                        }

                        bv->singleton_split_offset = split_offset;
                        makeBatchLabel(bv); // reset batch label even if it already existed, since we've changed the node's effective value
                    }
                }

                // groups of batch_size many subvalues have been concatenated together, so from this point batch_size is effectively 1
                bf->batch_size = 1;
            }
        }
        // reset values_start, it will become invalid after sorting of batch_values
        bf->values_start = -1;
    }

    for (idx iv=0; iv<batch_values.dim(); iv++) {
        batch_values[iv].branch_node = batch_values[iv].value_node;
        while (batch_values[iv].branch_node->parentNode() && batch_values[iv].branch_node->parentNode()->treeIndex() >= 0 && batch_descendents[batch_values[iv].branch_node->parentNode()->treeIndex()] == 1) {
            batch_values[iv].branch_node = batch_values[iv].branch_node->parentNode();
        }
        // Mark descendents of branch_node as non-zero in batch_descendents, so we can separate out nodes that are not in any batch branch
        batch_values[iv].branch_node->find(0, markDescendentsCb, &batch_descendents);
    }

    // Set up association from batch_fields to batch_values
    sSort::sortSimpleCallback(cmpBatchValues, this, batch_values.dim(), batch_values.ptr());
    for (idx iv=0; iv<batch_values.dim(); iv++) {
        BatchFieldIndex * fld_index = name2field.get(batch_values[iv].value_node->name());
        BatchList & lst = batch_lists[fld_index->ilist];
        BatchField & fld = batch_fields[lst.fields_start + fld_index->ifld];
        if (fld.values_start < 0) {
            fld.values_start = iv;
        }
        fld.batch_cnt = fld.values_cnt / fld.batch_size;
        if (fld.values_cnt % fld.batch_size) {
            fld.batch_cnt++;
        }
    }

    // Find batch_cnt for batch field lists
    // in "saturated" case, each batch contains at least one value from every field - so the list's batch count is the minimum batch count of its fields
    // in "unsaturated" case, each batch contains at most f.batch_size values from any field f - so the list's batch count is the max batch count of its fields
    for(idx ilst=0; ilst<batch_lists.dim(); ilst++) {
        BatchList & lst = batch_lists[ilst];
        lst.batch_cnt = 0;
        for(idx ifld=0; ifld<lst.fields_cnt; ifld++) {
            BatchField & fld = batch_fields[lst.fields_start + ifld];
            if( ifld == 0 ) {
                lst.batch_cnt = fld.batch_cnt;
            } else if( lst.saturated ) {
                lst.batch_cnt = sMin<idx>(lst.batch_cnt, fld.batch_cnt);
            } else {
                lst.batch_cnt = sMax<idx>(lst.batch_cnt, fld.batch_cnt);
            }
        }
    }

    proc->logOut(sQPrideBase::eQPLogType_Debug, "%s", printDump(0));

    // Set up skeleton form
    for (idx ikey=0; ikey<pForm->dim(); ikey++) {
        const char * key = static_cast<const char*>(pForm->id(ikey));
        // add all non-property form entries
        if (strncmp(key, "prop.", 5) != 0) {
            skel_form.inp(key, pForm->value(key));
        }
    }
    skel_form.inp("svc", svcToSubmit);
    for (idx inod=0; inod<batch_descendents.dim(); inod++) {
        // skip batched value branches
        if (batch_descendents[inod] == 1 || batch_descendents[inod] < 0)
            continue;
        const sUsrObjPropsNode * node = tree.getNodeByIndex(inod);
        // skip inner nodes and batching parameters
        if (!node->hasValue() || !node->namecmp("batch_param") || !node->namecmp("batch_value") || !node->namecmp("batch_saturated") || !node->namecmp("batch_children_proc_ids"))
            continue;
        // force set batch_svc to "single"
        if( !node->namecmp("batch_svc") ) {
            skel_form.inp(printFormKey(node->name(), node->path()), "single");
            continue;
        }
        // name node is handled later
        if (!node->namecmp("name")) {
            name_node = node;
            continue;
        }
        // reqPriority is handled specially
        if( !node->namecmp("reqPriority") ) {
            reqPriority_node = node;
            continue;
        }
        // submitter node is handled specially: remove terminal "batch" or "-batch" from "cmdMode" url parameter
        if (!node->namecmp("submitter")) {
            regmatch_t match[4]; // "&cmdMode=foobar-batch", "&cmdMode=", "foobar", "-batch"
            const char * submitter = node->value();
            if( submitter && regexec(&submitter_batch_re, submitter, 4, match, 0) == 0 ) {
                sStr value_buf;
                if( match[2].rm_so < 0 || match[2].rm_so == match[2].rm_eo ) {
                    // we had "&cmdMode=batch" - so skip it entirely
                    value_buf.addString(submitter, match[0].rm_so);
                } else {
                    // we had "&cmdMode=foobar-batch" - want to turn it into "&cmdMode=foobar"
                    value_buf.addString(submitter, match[2].rm_eo);
                }
                value_buf.addString(submitter + match[0].rm_eo);
                skel_form.inp(printFormKey(node->name(), node->path()), value_buf.ptr());
                continue;
            }
        }
        // svc / svcTitle need to be set to the submitted service
        if( !node->namecmp("svc") ) {
            skel_form.inp(printFormKey(node->name(), node->path()), svcToSubmit);
            continue;
        }
        if( !node->namecmp("svcTitle") ) {
            if( svcTitle && *svcTitle ) {
                skel_form.inp(printFormKey(node->name(), node->path()), svcTitle);
            }
            continue;
        }
        // remaining leaf nodes go into skeleton form
        skel_form.inp(printFormKey(node->name(), node->path()), node->value());
    }

    return true;
}

static void printNodeBrief(sJSONPrinter & p, const sUsrObjPropsNode * node)
{
    if( node ) {
        p.startObject();
        p.addKey("name");
        p.addValue(node->name());
        p.addKey("tree_index");
        p.addValue(node->treeIndex());
        p.addKey("path");
        p.addValue(node->path());
        p.addKey("value");
        p.addValue(node->value());
        p.endObject();
    } else {
        p.addNull();
    }
}

const char * BatchingDescriptor::printDump(sStr * s) const
{
    static sStr static_buf;
    if (!s) {
        static_buf.cut(0);
        s = &static_buf;
    }

    idx start = s->length();
    s->addString("batch = ");
    sJSONPrinter p(s);

    p.startArray(true);
    for(idx ilst = 0; ilst < batch_lists.dim(); ilst++) {
        const BatchList & lst = batch_lists[ilst];
        p.startObject();
        p.addKey("params");
        p.startArray(true);
        for(idx ifld = 0; ifld< lst.fields_cnt; ifld++) {
            const BatchField & fld = batch_fields[lst.fields_start + ifld];
            p.startObject();
            p.addKey("name");
            p.addValue(static_cast<const char *>(name2field.id(fld.iname)));
            p.addKey("values");
            p.startArray();
            for(idx ival = 0; ival < fld.values_cnt; ival++) {
                const BatchValue & val = batch_values[fld.values_start + ival];
                p.startObject();
                p.addKey("value_node");
                printNodeBrief(p, val.value_node);
                if( val.value_node != val.branch_node ) {
                    p.addKey("branch_node");
                    printNodeBrief(p, val.branch_node);
                }
                if( val.singleton_split_offset >= 0 ) {
                    p.addKey("singleton_value");
                    p.addValue(buf.ptr(val.singleton_split_offset));
                }
                if( val.label_offset >= 0 ) {
                    p.addKey("label");
                    p.addValue(buf.ptr(val.label_offset));
                }
                p.endObject();
            }
            p.endArray();
            p.addKey("values_cnt");
            p.addValue(fld.values_cnt);
            p.addKey("batch_cnt");
            p.addValue(fld.batch_cnt);
            p.addKey("batch_size");
            p.addValue(fld.batch_size);
            p.endObject();
        }
        p.endArray();
        p.addKey("batch_cnt");
        p.addValue(lst.batch_cnt);
        p.addKey("saturated");
        p.addValue(lst.saturated);
        p.endObject();
    }
    p.endArray();
    p.finish();
    s->addString("\n");

    return s->ptr(start);
}

// static
sUsrObjPropsNode::FindStatus BatchingDescriptor::addToFormWorker(const sUsrObjPropsNode & node, void * param)
{
    AddToFormWorkerParam * p = static_cast<AddToFormWorkerParam*>(param);

    const char * value = 0;
    if (p->bv->singleton_split_offset >= 0 && node.treeIndex() == p->bv->value_node->treeIndex()) {
        value = p->desc->buf.ptr(p->bv->singleton_split_offset);
    } else if (node.hasValue()) {
        value = node.value();
    }

    if (value) {
        p->pForm->inp(p->desc->printFormKey(node.name(), node.path()), value);
    }

    return sUsrObjPropsNode::eFindContinue;
}

bool BatchingDescriptor::addToForm(sVar * pForm, idx ilist, idx ifld, idx ibatch, idx ivalue) const
{
    if (!validIndex(ilist, ifld, ibatch, ivalue))
        return false;

    const BatchValue * bv = batch_values.ptr(getBatchValueIndex(ilist, ifld, ibatch, ivalue));
    AddToFormWorkerParam p(this, bv, pForm);
    bv->branch_node->find(0, addToFormWorker, &p);
    return true;
}

sVar * BatchingIterator::makeForm(sVar * pForm, const char * svcToSubmit) const
{
    const sVar * skel = desc->getSkeletonForm();
    pForm->empty();
    pForm->mex()->empty();
    pForm->mex()->add(skel->mex()->ptr(0), skel->mex()->pos());

    static sStr label;
    label.printf(0, "%s #%"DEC, svcToSubmit, cur_it + 1);

    for (idx ilist=0; ilist<batches.dim(); ilist++) {
        idx dim_fields = desc->dimFields(ilist);
        for(idx ifld=0; ifld<dim_fields; ifld++) {
            idx dim_values = desc->dimValues(ilist, ifld, batches[ilist]);
            for(idx ivalue = 0; ivalue < dim_values; ivalue++) {
                desc->addToForm(pForm, ilist, ifld, batches[ilist], ivalue);
                label.addString(ilist == 0 && ifld == 0 && ivalue == 0 ? " : " : ", ");
                label.addString(desc->getBatchLabel(ilist, ifld, batches[ilist], ivalue));
            }
        }
    }

    static sStr keybuf;
    keybuf.cut(0);
    pForm->inp(desc->printFormLabelKey(), label.ptr(0));

    keybuf.cut(0);
    label.cut(0);
    pForm->inp(desc->printFormReqPriorityKey(), label.printf("%"DEC, desc->getReqPriority(cur_it)));

    return pForm;
}

idx SvcBatcher::OnExecute(idx req)
{
#ifdef _DEBUG
    fprintf(stderr, "qpride form for req %"DEC":\n", req);
    for (idx i=0; i<pForm->dim(); i++) {
        const char * key = static_cast<const char*>(pForm->id(i));
        const char * value = pForm->value(key);
        fprintf(stderr, "  %s = %s\n", key, value);
    }
#endif

    if(!svcToSubmit)
        svcToSubmit = formValue("batch_svc");

    if(!svcToSubmit) {
        reqSetInfo(req, eQPInfoLevel_Error, "Incorrect or missing service name.");
        reqSetStatus(req, eQPReqStatus_ProgError);
        return 0;
    }

    selfService = (strcmp(vars.value("serviceName"), "svc-batcher") == 0) ? true : false;

    bool ignore_errors = formBoolValue("batch_ignore_errors");

    stillRunning=0;
    alreadyDone=0;
    killed=0;

    BatchingDescriptor batch_desc(*user, this);
    BatchingIterator batch_iter;

    if(!reqGetData(req,"submittedGrpIDs",submittedGrpIDs.mex())){
        if( !doCreateProcesses )
            reqSetData(req,"resultFileTemplate","reqself-");

        const char * svc=svcToSubmit ? svcToSubmit : pForm->value("svc",0); // otherwise , use the CGI's service itself
        sQPride::Service Svc;
        serviceGet( &Svc, svc, 0) ; // otherwise retrieve it from the reqID

        sStr errBuf;

        if (!batch_desc.init(pForm, svcToSubmit, Svc.title, &errBuf)) {
            //
            // If there was no batching Descriptors then set as an error
            //
            reqSetInfo(req, eQPInfoLevel_Error, "Incorrect or missing batch parameters: %s", errBuf.ptr());
            reqSetStatus(req, eQPReqStatus_ProgError);
            return 0;
        }

        batch_iter.init(&batch_desc);

        sStr log, strObjList;
        sVar submissionForm;
        submittedGrpIDs.add(batch_iter.maxIterations());
        if( doCreateProcesses ) {
            submittedProcessIDs.add(batch_iter.maxIterations());
            logOut(eQPLogType_Info, "Will create %"DEC" process submissions", batch_iter.maxIterations());
        } else {
            logOut(eQPLogType_Info, "Will create %"DEC" non-process submissions", batch_iter.maxIterations());
        }

        idx howManySubmitted=0;
        idx numProcesses=0;
//        for ( idx iv=0; iv<valueSets.dim() ; ++iv) {
        for(; batch_iter; ++batch_iter) {
            sVec< sUsrProc > procObjs;
            idx reqSub;

            idx err=0;
            strObjList.cut(0);
            logOut(eQPLogType_Trace, "Trying to make form for value set %"DEC, batch_iter.getCurStep());
            batch_iter.makeForm(&submissionForm, svcToSubmit);

#ifdef _DEBUG
            log.printf(0, "Created form for value set %"DEC":\n", batch_iter.getCurStep());
            for (idx i=0; i<submissionForm.dim(); i++) {
                const char * key = static_cast<const char*>(submissionForm.id(i));
                const char * value = submissionForm.value(key);
                log.printf("  %s = %s\n", key, value);
            }
            logOut(eQPLogType_Trace, "%s", log.ptr());
            log.cut0cut();
#endif

            if (doCreateProcesses) {
                logOut(eQPLogType_Trace, "Trying to create process for value set %"DEC, batch_iter.getCurStep());
                err=sUsrProc::createProcesForsubmission(this, &submissionForm, user, procObjs, &Svc, &strObjList, &log);
                if( err ) {
                    logOut(eQPLogType_Error, "Failed to create process for value set %"DEC": %s", batch_iter.getCurStep(), log.ptr());
                    if( ignore_errors ) {
                        reqSetInfo(req, eQPInfoLevel_Warning, "Failed to create process; attempting to continue (ignoring errors).");
                        continue;
                    } else {
                        recordSubmittedProcessIDs(numProcesses);
                        reqSetInfo(req, eQPInfoLevel_Error, "Failed to create process.");
                        reqSetStatus(req, eQPReqStatus_ProgError);
                        return 0;
                    }
                }
                submittedProcessIDs[numProcesses++] = procObjs[0].Id();
                logOut(eQPLogType_Info, "Created process for value set %"DEC": %s", batch_iter.getCurStep(), strObjList.ptr());
            }

            logOut(eQPLogType_Trace, "Trying to customize submission for value set %"DEC, batch_iter.getCurStep());
            idx cntParallel=sHiveTools::customizeSubmission(&submissionForm, user, procObjs.dim() ? procObjs.ptr(0) : 0, &Svc, &log);
            if( !cntParallel ) {
                logOut(eQPLogType_Error, "Failed to customize submission for value set %"DEC": %s", batch_iter.getCurStep(), log.ptr());
                if( ignore_errors ) {
                    sStr readable;
                    reqSetInfo(req, eQPInfoLevel_Warning, "Failed to customize submission; attempting to continue (ignoring errors). %s", prop2readableLog(readable, log.ptr()));
                    continue;
                } else {
                    recordSubmittedProcessIDs(numProcesses);
                    sStr readable;
                    reqSetInfo(req, eQPInfoLevel_Error, "Failed to customize submission. %s", prop2readableLog(readable, log.ptr()));
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
            }
            logOut(eQPLogType_Info, "Customized submission for value set %"DEC": parallel count == %"DEC, batch_iter.getCurStep(), cntParallel);

            logOut(eQPLogType_Trace, "Trying standardized submission for value set %"DEC, batch_iter.getCurStep());
            err=sUsrProc::standardizedSubmission(this, &submissionForm, user, procObjs, cntParallel, &reqSub, &Svc, 0, &strObjList, &log);
            if( err ) {
                logOut(eQPLogType_Error, "Failed to submit process for value set %"DEC": %s", batch_iter.getCurStep(), log.ptr());
                if( ignore_errors ) {
                    reqSetInfo(req, eQPInfoLevel_Warning, "Failed to submit process; attempting to continue (ignoring errors).");
                    continue;
                } else {
                    recordSubmittedProcessIDs(numProcesses);
                    reqSetInfo(req, eQPInfoLevel_Error, "Failed to submit process.");
                    reqSetStatus(req, eQPReqStatus_ProgError);
                    return 0;
                }
            }
            howManySubmitted+=cntParallel;
            logOut(eQPLogType_Info, "Standardized submission for value set %"DEC": parallel count == %"DEC", total submitted == %"DEC, batch_iter.getCurStep(), cntParallel, howManySubmitted);

            sVec < idx > reqIds;
            grp2Req(reqSub, &reqIds);
            if( !doCreateProcesses ) {
                for ( idx ir=0; ir<reqIds.dim(); ++ir ) {
                    grpAssignReqID(reqIds[ir], req, ir);
                }
            }

            log.cut0cut();
            sString::printfIVec(&log, &reqIds, ", ");
            logOut(eQPLogType_Info, "Running requests for value set %"DEC": %s", batch_iter.getCurStep(), log.ptr());
            log.cut0cut();

            reqSetAction(&reqIds, eQPReqAction_Run);

            submittedGrpIDs[batch_iter.getCurStep()]=reqSub;
            ++stillRunning;
        }

        // remember the list of submitted Ids here
        recordSubmittedProcessIDs(numProcesses);

        if( selfService && doCreateProcesses ){
            // Sets svc-batcher process to finished no matter what
            reqProgress(batch_iter.getCurStep(), 100, 100);
            reqSetStatus(req, eQPReqStatus_Done);// change the status
        }
        else
            reqSetData(req,"submittedGrpIDs",submittedGrpIDs.dim()*sizeof(idx),submittedGrpIDs.ptr());

    }
    else {


        // if it has already been submitted
        // now we analyze the status
        for ( idx ig=0; ig<submittedGrpIDs.dim() ; ++ig) {
            sVec < idx > stat;
            //grpGetStatus( submittedGrpIDs[ig], &stat,svcToWaitFor);
            //grp2Req(submittedGrpIDs[ig], &waitedReqs, svcToWaitFor);
            grp2Req(req, &waitedReqs, svcToWaitFor, 0);
            if(waitedReqs.dim())reqGetStatus(&waitedReqs,&stat);

            for ( idx is=0; is<stat.dim() ; ++is ) {
                if(stat[is]<eQPReqStatus_Done)
                    ++stillRunning;
                else if(stat[is]==eQPReqStatus_Done)
                    ++alreadyDone;
                else
                    ++killed;
            }
        }


    }

    if(!selfService) {
        if(alreadyDone+stillRunning+killed) {
            reqProgress(batch_iter.getCurStep(), alreadyDone, alreadyDone + stillRunning + killed);
        }

        if(killed) {
            reqProgress(batch_iter.getCurStep(), 0, 100);
            reqSetStatus(req, eQPReqStatus_ProgError); // change the status
            return 0;
        }
    }
//    else if( stillRunning) {
//        reqReSubmit(req,60);
//        return 0;
//    }

//    if(selfService) {
//        reqSetProgress(req,valueSets.dim(), 100);
//        reqSetStatus(req, eQPReqStatus_Done);// change the status
//    }
    return 0;
}

void SvcBatcher::recordSubmittedProcessIDs(idx num)
{
    num = sMin<idx>(num, submittedProcessIDs.dim());
    if( selfService && doCreateProcesses && objs.dim() ) {
        if( num ) {
            sStr buf;
            for(idx i = 0; i < num; i++) {
                submittedProcessIDs[i].print(buf);
                buf.add0();
            }
            buf.add0(2);

            sVec<const char *> id_values;
            id_values.resize(num);
            const char * id_value = buf.ptr();
            for(idx i = 0; i < num; i++, id_value = sString::next00(id_value, 1)) {
                id_values[i] = id_value;
            }

            objs[0].propSet("batch_children_proc_ids", 0, id_values.ptr(), num, false);
        } else {
            objs[0].propDel("batch_children_proc_ids", 0, 0);
        }
    }
}

const char * SvcBatcher::prop2readableLog(sStr & out_buf, const char * prop_fmt_log)
{
    sVarSet tbl;
    sStr tmp_log; // ignored - if parsing prop format fails, let's simply return the raw string
    if( user->propSet(prop_fmt_log, 0, tmp_log, tbl, true) && tbl.rows && tbl.cols == 4 ) {
        idx pos = out_buf.length();
        idx ids_used = 0;

        for(idx irow = 0; irow < tbl.rows; ) {
            const char * block_id = tbl.val(irow, 0);
            idx block_pos = out_buf.length();
            bool block_used = false;

            out_buf.printf("%sError in processing object %s: ", ids_used ? " " : "", block_id ? block_id : "???");
            if( !block_id ) {
                block_id = sStr::zero;
            }

            for(; irow < tbl.rows; irow++) {
                const char * row_id = tbl.val(irow, 0);
                if( !row_id ) {
                    row_id = sStr::zero;
                }
                if( strcmp(block_id, row_id) != 0 ) {
                    irow++;
                    break;
                }

                const char * fld = tbl.val(irow, 1);
                const char * msg = tbl.val(irow, 3);
                idx msg_len = sLen(msg);

                for(; msg_len > 0 && (msg[msg_len - 1] == '.' || isspace(msg[msg_len - 1])); msg_len--);

                if( msg_len > 0 && (!fld || fld[0] != '_') ) {
                    if( block_used ) {
                        out_buf.addString("; ");
                    } else {
                        block_used = true;
                    }
                    char c = tolower(msg[0]);
                    out_buf.addString(&c, 1);
                    if( msg_len > 1 ) {
                        out_buf.addString(msg + 1, msg_len - 1);
                    }
                }
            }

            if( block_used ) {
                out_buf.addString(".");
                ids_used++;
            } else {
                out_buf.cut0cut(block_pos);
            }
        }

        if( ids_used ) {
            return out_buf.ptr(pos);
        }
    }

    return prop_fmt_log ? prop_fmt_log : sStr::zero;
}

//idx SvcBatcher::getService (sStr & _tmp2) {
//    sStr _tmp;
//    _tmp.printf("%s", formValue("REFERER"));
//    //sStr _tmp2;
//    bool doNotRecord = true;
//
//    // Split out calling service
//    for (idx i = 0; i < _tmp.length();i++) {
//        char _tmpChar[2];
//        _tmpChar[0] = *(_tmp.ptr(i));
//        _tmpChar[1] = '\0';
//        if (strcmp(_tmpChar, "=") != 0 && doNotRecord) continue;
//        else {
//            if (doNotRecord){
//                doNotRecord = false;
//                continue;
//            }
//            // need to read until hit &
//            if (strcmp(_tmpChar, "&") == 0) return 1;
//            else _tmp2.printf("%s", _tmpChar);
//        }
//    }
//    return 0;
//}
