
DROP PROCEDURE IF EXISTS `sp_obj_prop_set_v3`;

DELIMITER //

CREATE PROCEDURE `sp_obj_prop_set_v3`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_obj_id BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED,
    IN p_data_tuples LONGTEXT CHARACTER SET utf8, -- 24MB, a little over 'value' column size of UPObjField
    IN p_overwrite_names MEDIUMTEXT -- CSV of field names to wipe
)
    MODIFIES SQL DATA
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
    DECLARE ins_qty BIGINT UNSIGNED DEFAULT 0;
    DECLARE del_qty BIGINT UNSIGNED DEFAULT 0;

    CALL sp_permission_check_v2(p_group_id, p_member_sql, p_obj_domainID, p_obj_id, p_permissions, @allowed);

    IF @allowed != 0 THEN
        IF p_obj_domainID = 0 THEN
            SET p_obj_domainID = NULL;
        END IF;
        IF p_overwrite_names IS NOT NULL AND LENGTH(p_overwrite_names) > 0 THEN
            SET @s = CONCAT('DELETE FROM UPObjField WHERE domainID', IF(p_obj_domainID IS NULL, ' IS NULL', CONCAT(' = ', p_obj_domainID)), ' AND objID = ', p_obj_id, ' AND FIND_IN_SET(`name`, \'', p_overwrite_names, '\') > 0');
 -- select @s;
            PREPARE x FROM @s;
            EXECUTE x;
            SELECT ROW_COUNT() INTO del_qty;
        END IF;
        IF p_data_tuples IS NOT NULL AND LENGTH(p_data_tuples) > 0 THEN
            SET @s = CONCAT('INSERT INTO UPObjField (domainID, objID, `name`, `group`, `value`, `encoding`, `blob_value`) VALUES ', p_data_tuples);
 -- select @s;
            PREPARE x FROM @s;
            EXECUTE x;
            SELECT ROW_COUNT() INTO ins_qty;
        END IF;
        IF ins_qty > 0 OR del_qty > 0 THEN
            DELETE FROM UPObjField
                WHERE (domainID = p_obj_domainID OR (domainID IS NULL AND p_obj_domainID IS NULL)) AND objID = p_obj_id AND `name` = 'modified';
            INSERT INTO UPObjField (domainID, objID, `name`, `value`)
                VALUES (p_obj_domainID, p_obj_id, 'modified', UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
        END IF;
    END IF;

    SELECT ins_qty;

END //
DELIMITER ;
