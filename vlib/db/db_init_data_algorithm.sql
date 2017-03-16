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

SET @type_algorithm_x = 'algorithm-alignment-dnaseq';
SELECT type_id FROM UPType WHERE name = @type_algorithm_x INTO @type_algorithm_x_id;

DELETE FROM UPPerm WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObjField WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObj WHERE objTypeID = @type_algorithm_x_id;

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'HIVE-Hexagon\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-hexagon.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Native HIVE-hexagon algorithm optimized for High Performance Cloud computing environments\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-blast\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'NCBI-blast\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'2.2.30\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-blast.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'NCBI blast algorithm developped by National Center for Biotechnology information\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-tblastx\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'NCBI-TblastX\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'2.2.30\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-blast.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'NCBI tblastx algorithm developped by National Center for Biotechnology information\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-blastx\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'NCBI-BlastX\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'2.2.30\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-blast.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'NCBI Blastx algorithm developped by National Center for Biotechnology information\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-blat\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Blat\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'35x1\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-blat.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Standalone UCSC BLAT-35x1 fast sequence search command line tool\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-bowtie\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Bow-Tie\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'4.1.2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-bowtie.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Bow Tie Algorithm developed for Nex-gen Analysis\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-bowtie2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Bow-Tie2\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'2.1.0\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-bowtie.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Bow Tie 2 Algorithm developed for Nex-gen Analysis\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-bwa\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'BWA\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'version', 'NULL,\'0.7.3a\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-bwa.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Burrow Wheeler Algorithm\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);



SET @type_algorithm_x = 'algorithm-alignment-rnaseq';
SELECT type_id FROM UPType WHERE name = @type_algorithm_x INTO @type_algorithm_x_id;

DELETE FROM UPPerm WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObjField WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObj WHERE objTypeID = @type_algorithm_x_id;

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-tophat\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'Tophat\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-tophat.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'Tophat based RNA-seq workflow\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);


SET @type_algorithm_x = 'algorithm-alignment-multiple';
SELECT type_id FROM UPType WHERE name = @type_algorithm_x INTO @type_algorithm_x_id;

DELETE FROM UPPerm WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObjField WHERE objID IN (SELECT objID FROM UPObj WHERE objTypeID = @type_algorithm_x_id);
DELETE FROM UPObj WHERE objTypeID = @type_algorithm_x_id;

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-clustal\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'clustal\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-clustal.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'clustal multiple alignment algorithm\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'qpsvc', 'NULL,\'\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

CALL sp_obj_create(@system_group_id, @system_membership, @type_algorithm_x, @system_permission, @system_flags);
SET @oid = @id; -- comes from create sp
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'name', 'NULL,\'svc-align-mafft\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'title', 'NULL,\'MAFFT\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'icon', 'NULL,\'img-algo/svc-align-mafft.gif\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'description', 'NULL,\'MAFFT multiple alignment algorithm\'', '', FALSE);
CALL sp_obj_prop_set(@system_group_id, @system_membership, @oid, @system_permission, 'qpsvc', 'NULL,\'\'', '', FALSE);
CALL sp_obj_perm_set(@users_group_id, @users_membership, @users_group_id, @oid, NULL, @users_permission, @users_flags, TRUE);

-- ----------------------
-- ----------------------
-- ----------------------

COMMIT;
