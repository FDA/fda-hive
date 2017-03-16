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

START TRANSACTION;

source db_init_data_include.sql;

-- ----------------------
-- default actions
-- ----------------------
SET @type_action = 'action';
SELECT type_id FROM UPType WHERE name = @type_action INTO @type_action_id;

DELETE FROM UPPerm WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_action_id);
DELETE FROM UPObjField WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_action_id);
DELETE FROM UPObj WHERE objTypeID = @type_action_id;

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'create\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Create\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=act&act=create&type=$(type)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'plus\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Create an object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'search\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Search\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=objList&type=$(type)&prop_val=$(search)&start=$(start)&cnt=$(cnt)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'create\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'detail\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Details\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=record&ids=$(ids)&types=$(types)&readonly=1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'10\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'eye\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'See objects metadata\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'edit\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Edit\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=record&ids=$(ids)&types=$(types)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'9\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img/edit.png\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/detail/edit\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Examine, review, edit details of the object\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'export\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Export\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=propget&mode=json&ids=$(ids)&raw=1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'9.5\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
-- CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'download\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/export\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Export the object metadata\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'copy\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Copy\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/cut/copy\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'javascript:gClip.copy($(srcFolder),$(ids))\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'copy\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cut\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Cut\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/cut\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'javascript:gClip.cut($(srcFolder),$(ids))\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'cut\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'delete\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Delete\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'javascript:gClip._delete( $(s:srcFolder),$(s:dcls),$(s:ids) )\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'delete\'', '', FALSE);
-- CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'confirmation', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Delete object\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'admin\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Admin\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'32\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=admin&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'admin\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Administer object\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'share\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'base+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Share\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'96\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=sharing&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'8\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'share\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'share the object with other users of the system; esablish permissions.\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

--
-- renameable object types
--
CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'rename\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'file+,folder+,ionDB+,process+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Rename\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=propset&prop.$(ids).name.1=$(prompt_res)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'10\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img/edit.png\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/detail/rename\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Quickly rename an object.\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

--
-- directory
--
CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'subfolder\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'directory+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Create\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=folderCreate&ids=$(ids)&name=$(prompt_res)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'7\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'folder-new\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Create a folder\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'paste\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'directory+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Paste\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'javascript:gClip.paste($(ids),$(s:dcls))\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'6\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'paste\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

--
-- u-file
--
CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'upload\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Upload a new file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'13\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'upload\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Upload a file\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'download\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Download file to local computer\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=objFile&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'download\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Download file\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'new\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Spectra/cast2spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to a Spectra object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- jump from tqs
-- 
CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'jumpToTblqry\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-tqs\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Go to TableQuery\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=tblqry-new&tqsId=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'new\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/jumpToTblqry\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Go to TableQuery with current TQS\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2tqs\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Convert to TQS\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=u-tqs&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/cast2tqs\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert current object to TQS type\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);
--
--
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2spectra-lib\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Spectra Library\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=spectra-lib&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Spectra/cast2spectra-lib\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to Spectra Library object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2spectra-MS\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Spectra MS\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=spectraPeakDetection&SpectraFile=$(ids)&selfDir=1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'new\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Spectra/cast2spectra-MS\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to Spectra-MS object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAsAuto\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Auto detect\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAsAuto\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Reprocess recognizing file type by its extension\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAsbz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as bzip2 file (.bz2)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=bz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'5\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAsbz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as bzip2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAstbz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as tar bzip2 file (.tbz2)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=tbz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'6\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAstbz2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as tar bzip2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAsgz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as gzip file (.gz)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=gz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAsgz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as gzip\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAstgz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as tar gzip file (.tgz)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=tgz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAstgz\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as tar gzip\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAstar\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as tar file (.tar)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=tar\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAstar\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as tar\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAssra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as sra file (.sra)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=sra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'7\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAssra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as sra\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAsbam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as bam file (.bam)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=bam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'8\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAsbam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as bam\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'TreatAszip\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as zip file (.zip)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&ext=zip\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'9\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/Reprocess/TreatAszip\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Process file compressed as zip\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2fasta\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as FastA file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&objtype=nuc-read&ids=$(ids)&datatype=fasta\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Reads/convert2fasta\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as fastA to Reads object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2fastq\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as FastQ file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&objtype=nuc-read&ids=$(ids)&datatype=fastq\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Reads/convert2fastq\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as fastQ to Reads object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To Genome as FastA file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&category=genomic&objtype=genome&ids=$(ids)&datatype=fasta\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/convert2genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as fastA to Genome object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2protein\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To Protein Sequence as FastA file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&objtype=prot-seq&ids=$(ids)&datatype=fasta\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/convert2protein\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as fastA to Protein Sequence object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2sam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Treat as SAM File\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&objtype=nuc-read&ids=$(ids)&datatype=sam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Reads/convert2sam\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as SAM to Reads object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2annot\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To Annotation\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=annotationConverter&objID=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'annotationConverter\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/convert2annot\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to Annotation object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- u-ionExpress and u-idList
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2geneExpressOmics\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To Omics data \'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=omics&invitro=0&experiment=$(?:Please provide experiment)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Gene Expression/convert2geneExpressOmics\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to Omics gene expression object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2geneExpressInVitro\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To In Vitro data\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=archive&ids=$(ids)&objtype=u-ionExpress&isExpr=1&hasdata=measurements&invitro=1&experiment=$(?:Please provide experiment)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Gene Expression/convert2geneExpressInVitro\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to gene expression object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'convert2geneExpressGeneList\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'To Gene List \'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=ingestGeneList&objToConvert=$(ids)&convert=1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/To Gene Expression/convert2geneExpressGeneList\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'single_obj_only', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert file formatted as CSV to gene list for Gene Expression Analysis\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- u-hiveseq
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'hiveseq\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-hiveseq+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Hiveseq editor\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&tab=General&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'hiveseq\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Create a new composite sequence file: HIVESEQ\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- nuc-read type
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'nuc-read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Convert to Genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=genome&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/cast2genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert from Reads to Genome\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'hexagon_nuc_read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'nuc-read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Hive-hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&tab=Alignment&query=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'hive-hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Align using hive-hexagon\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'censuscope_nuc_read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'nuc-read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'censuScope\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=dna-screening&query=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img/scope.png\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Analyze using CensuScope\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- genome
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2nuc-read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Convert to Reads\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=nuc-read&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/cast2nuc-read\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert from Genome to Reads\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'hexagon_genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Hive-hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&tab=Alignment&subject=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'hive-hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Align using hive-hexagon\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- spectra
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Spectral Analyzer\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=spectraPeakDetection&&SpectraFile=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Spectral Data Analyzer\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'spectra\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'spectra-MS\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Spectral Analyzer\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=spectraPeakDetection&&SpectraFile=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'-3\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Spectral Data Analyzer\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- image
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2systemImage\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'image\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Convert to system image\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=system-image&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/cast2systemImage\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert from Image to System Image\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- multimedia
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'cast2multimedia\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'u-file\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Convert to multimedia object\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=scast&type=multimedia&ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/Convert/cast2multimedia\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Convert from File to Multimedia Object\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- process
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'procClone\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'process+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Resubmit\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=-qpProcClone&src_ids=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'12\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'ajax\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'response', 'NULL,\'json\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Resubmit the process with same parameters (without confirmation)\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- table
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_action, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'tblQry\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'type_name', 'NULL,\'table+\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Analyze\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'is_obj_action', 'NULL,\'true\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'required_permission', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=tblqry-new&objs=$(ids)\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'13\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'target', 'NULL,\'new\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'rec\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Analyze table with Table Query\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

-- ----------------------
-- ----------------------
-- ----------------------

COMMIT;
