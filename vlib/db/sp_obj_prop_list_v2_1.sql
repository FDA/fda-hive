
DROP PROCEDURE IF EXISTS `sp_obj_prop_list_v2_1`;

DELIMITER //

CREATE PROCEDURE `sp_obj_prop_list_v2_1`(
    IN p_user_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_objIDs MEDIUMTEXT,
    IN p_prop_list MEDIUMTEXT
)
    READS SQL DATA
BEGIN
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


     IF p_objIDs IS NOT NULL AND LENGTH(p_objIDs) > 0 THEN

        DROP TEMPORARY TABLE IF EXISTS tmp_obj_prop_list_v2;
        CREATE TEMPORARY TABLE tmp_obj_prop_list_v2
        (
            `domainID` BIGINT UNSIGNED NOT NULL,
            `objID` BIGINT UNSIGNED NOT NULL,
            UNIQUE KEY `uniq_idx` (`domainID`,`objID`)
        );

        SET @q = CONCAT('INSERT IGNORE INTO tmp_obj_prop_list_v2 SELECT domainID, objID FROM UPObj WHERE ', p_objIDs);
        PREPARE x FROM @q;
        EXECUTE x;

        SELECT o.domainID, o.objID, f.`name`, f.`group`, f.`value`, f.`encoding`, f.`blob_value`
            FROM UPObjField f JOIN tmp_obj_prop_list_v2 o
                ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND f.objID = o.objID
            WHERE IF(LENGTH(p_prop_list) > 0, FIND_IN_SET(f.`name`, p_prop_list) > 0, TRUE);

        DROP TEMPORARY TABLE IF EXISTS tmp_obj_prop_list_v2;

    END IF;
END //
DELIMITER ;
