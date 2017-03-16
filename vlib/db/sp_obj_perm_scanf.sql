
DROP PROCEDURE IF EXISTS `sp_obj_perm_scanf`;

DELIMITER //

CREATE PROCEDURE `sp_obj_perm_scanf`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_group_path VARCHAR(1024),
    IN p_type_name MEDIUMTEXT,
    IN p_view_name MEDIUMTEXT
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

    SET @grpId = NULL;
    IF LENGTH(p_group_path) > 0 THEN
        SELECT groupID FROM UPGroup WHERE groupPath = p_group_path
        INTO @grpId;
    END IF;

    SET @viewDomainID = NULL;
    SET @viewID = NULL;
    IF p_type_name IS NOT NULL AND LENGTH(p_type_name) > 0 AND p_view_name IS NOT NULL AND LENGTH(p_view_name) > 0 THEN
        -- TODO: test on real views
        SELECT f1.domainID, f1.objID FROM UPObjField f1 JOIN UPObjField f2
            ON (f1.domainID = f2.domainID OR (f1.domainID IS NULL AND f2.domainID IS NULL)) AND f1.objID = f2.objID
            WHERE f1.`name` = 'name' AND f1.`value` = p_view_name AND
                  f2.`name` = 'type_name' AND f2.`value` = p_type_name
        INTO @viewDomainID, @viewID;
    END IF;

    SELECT @grpId, @viewID, @viewDomainID;
END //
DELIMITER ;
