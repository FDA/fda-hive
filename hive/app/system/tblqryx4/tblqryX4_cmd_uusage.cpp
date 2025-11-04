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

#include <ulib/uusage.hpp>
#include "tblqryX4_cmd.hpp"

using namespace slib;
using namespace slib::tblqryx4;

namespace slib {
    namespace tblqryx4 {
        class UUsageCommand : public Command
        {
            private:
                sVec<sUsrUsage2::GroupSpec> group_specs;
                time_t since;
                time_t to;
                sUsrUsage2::EUsageType type;
                bool expand_groups;

            public:
                UUsageCommand(ExecContext & ctx) : Command(ctx)
                {
                    since = 0;
                    to = sIdxMax;
                    type = sUsrUsage2::eDiskUsage;
                    expand_groups = false;
                }

                const char * getName() { return "u-usage"; }
                bool computesOutTable() { return true; }
                bool needsInTableReinterpret() { return false; }

                bool init(const char * op_name, sVariant * arg);
                bool initGroupSpec(sVec<sUsrUsage2::GroupSpec> & specs, sVariant * arg);
                bool compute(sTabular * tbl);
        };
        Command * cmdUUsageFactory(ExecContext & ctx) { return new UUsageCommand(ctx); }
    };
};


bool UUsageCommand::initGroupSpec(sVec<sUsrUsage2::GroupSpec> & specs, sVariant * arg)
{
    if( arg->isList() ) {
        for(idx i = 0; i < arg->dim(); i++) {
            if( !initGroupSpec(specs, arg->getListElt(i)) ) {
                return false;
            }
        }
        return true;
    } else if( arg->isString() ) {
        idx ispec = specs.dim();
        specs.add(1);
        specs[ispec].kind = sUsrUsage2::GroupSpec::eGroupPath;
        specs[ispec].value.setString(arg->asString());
        return true;
    } else if( sVariant * val_arg = arg->getDicElt("userID") ) {
        idx ispec = specs.dim();
        specs.add(1);
        specs[ispec].kind = sUsrUsage2::GroupSpec::eUserID;
        specs[ispec].value.setUInt(val_arg->asUInt());
        if( sVariant * except_arg = arg->getDicElt("except") ) {
            initGroupSpec(specs[ispec].except, except_arg);
        }
        return true;
    } else if( sVariant * val_arg = arg->getDicElt("billable_group_obj") ) {
        idx ispec = specs.dim();
        specs.add(1);
        specs[ispec].kind = sUsrUsage2::GroupSpec::eBillableGroupObj;
        specs[ispec].value.setHiveId(*val_arg);
        if( sVariant * except_arg = arg->getDicElt("except") ) {
            initGroupSpec(specs[ispec].except, except_arg);
        }
        return true;
    } else if( sVariant * val_arg = arg->getDicElt("billable_group_name") ) {
        idx ispec = specs.dim();
        specs.add(1);
        specs[ispec].kind = sUsrUsage2::GroupSpec::eBillableGroupName;
        specs[ispec].value.setString(val_arg->asString());
        if( sVariant * except_arg = arg->getDicElt("except") ) {
            initGroupSpec(specs[ispec].except, except_arg);
        }
        return true;
    }
    return false;
}

bool UUsageCommand::init(const char * op_name, sVariant * arg)
{
    if( arg ) {
        if( sVariant * groups_arg = arg->getDicElt("group") ) {
            if( !initGroupSpec(group_specs, groups_arg) ) {
                _ctx.logError("u-usage command: invalid \"group\" argument");
                return false;
            }
        }

        if( sVariant * since_arg = arg->getDicElt("since") ) {
            since = since_arg->asDateTime();
        }

        if( sVariant * to_arg = arg->getDicElt("to") ) {
            to = to_arg->asDateTime();
        }

        if( sVariant * type_arg = arg->getDicElt("type") ) {
            type = sUsrUsage2::parseTypeName(type_arg->asString());
        }

        if( sVariant * expand_groups_arg = arg->getDicElt("expand_groups") ) {
            expand_groups = expand_groups_arg->asBool();
        }
    }

    if( type == sUsrUsage2::eUsageTypeInvalid ) {
        _ctx.logError("u-usage command: missing or invalid \"type\" argument");
        return false;
    }

    return true;
}

bool UUsageCommand::compute(sTabular * tbl)
{
    sTxtTbl * out_tbl = new sTxtTbl;

    sRC rc;
    const sUsrUsage2 * uusage_obj = sUsrUsage2::getObj(*_ctx.qproc().user, &rc);
    if( !uusage_obj ) {
        _ctx.logError("u-usage command: failed to open user usage: %s", rc.print());
        delete out_tbl;
        return 0;
    }

    if( group_specs.dim() ) {
        rc = uusage_obj->exportGroupsTable(*out_tbl, type, since, to, group_specs.ptr(), group_specs.dim(), group_specs.dim() > 1, expand_groups);
    } else {
        rc = uusage_obj->exportTable(*out_tbl, type, since, to);
    }
    if( rc ) {
        _ctx.logError("u-usage command failed: %s", rc.print());
        delete uusage_obj;
        delete out_tbl;
        return false;
    }
    delete uusage_obj;
    setOutTable(out_tbl);
    return true;
}
