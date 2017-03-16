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
-- default menus
-- ----------------------
SET @type_menuitem = 'menuitem';
SELECT type_id FROM UPType WHERE name = @type_menuitem INTO @type_menuitem_id;

DELETE FROM UPPerm WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_menuitem_id);
DELETE FROM UPObjField WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_menuitem_id);
DELETE FROM UPObj WHERE objTypeID = @type_menuitem_id;

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Main\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=main\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'1\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/home\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Home\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=home\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);


CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'HIVE-Portal\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/help\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Help\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=main-about#training\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'6\'', '', FALSE);
CALL sp_obj_perm_set(@everyone_group_id, @everyone_membership, @everyone_group_id, @oid, NULL, @everyone_permission, @everyone_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/login\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Login\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=login&follow=home\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'7\'', '', FALSE);
CALL sp_obj_perm_set(@public_group_id, @public_membership, @public_group_id, @oid, NULL, @public_permission, @public_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/register\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Register\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=userReg\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'8\'', '', FALSE);
CALL sp_obj_perm_set(@public_group_id, @public_membership, @public_group_id, @oid, NULL, @public_permission, @public_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/logout\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Logout\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=logout&follow=login\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'10\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/profile\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'eval:gUserGetInfo("name")\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'11\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/profile/info\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Account Info\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=user\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'1\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/profile/settings\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Settings\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'right\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'javascript:vjObjEvent("winop", "vjVisual","dvUser","toggle");\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/general\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'General DNA Filters and Tools\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=General\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/alignment\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Sequence Alignment on Genome\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=Alignment\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/Assemblers\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Sequence Assemblers\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=Assembler\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);


CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/datasets\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Taxonomy Dataset\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=Datasets\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'5\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);


CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/Annotations\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Annotations\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=Annotations\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'5\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);


CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/tools/classifications\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Classifications\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'?cmd=menu&selected=Classifications\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'6\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

--
-- ALPHABETIC ORDER!
--

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Links\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'align', 'NULL,\'left\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links/DDBJ\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'DNA Databank of Japan\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'http://www.ddbj.nig.ac.jp\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'1\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links/EBI\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'European Bioinformatics Institute\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'http://www.ebi.ac.uk\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'2\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links/NCBI\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'NCBI\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'http://ncbi.nlm.nih.gov\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'3\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links/PIR\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Protein Information Resource\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'http://pir.georgetown.edu\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'4\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_menuitem, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'path', 'NULL,\'/links/Uniprot\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'UniProt\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'url', 'NULL,\'http://www.uniprot.org\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'order', 'NULL,\'5\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

-- ----------------------
-- ----------------------
-- ----------------------

COMMIT;
