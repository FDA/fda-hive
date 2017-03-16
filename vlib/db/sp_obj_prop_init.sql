
DROP PROCEDURE IF EXISTS `sp_obj_prop_init`;

DELIMITER //

CREATE PROCEDURE `sp_obj_prop_init`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_obj_id BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED
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
    DECLARE del_qty BIGINT UNSIGNED DEFAULT 0;

    DECLARE has_created BIGINT UNSIGNED DEFAULT 0;
    DECLARE cnt_deleted BIGINT UNSIGNED DEFAULT 0;
    DECLARE cnt_changed BIGINT UNSIGNED DEFAULT 0;

    CALL sp_permission_check_v2(p_group_id, p_member_sql, p_obj_domainID, p_obj_id, p_permissions, @allowed);

    IF @allowed != 0 THEN
        -- normalize domainID to UPObjField default: NULL
        SET p_obj_domainID = IF(p_obj_domainID = 0, NULL, p_obj_domainID);
        SELECT COUNT(*) FROM UPObjField WHERE (domainID = p_obj_domainID OR (domainID IS NULL AND p_obj_domainID IS NULL)) AND objID = p_obj_id AND name = 'created' INTO has_created;
        DELETE FROM UPObjField WHERE (domainID = p_obj_domainID OR (domainID IS NULL AND p_obj_domainID IS NULL)) AND objID = p_obj_id AND name != 'created';
        SELECT ROW_COUNT() INTO cnt_deleted;
        SET cnt_changed = cnt_deleted;
        IF has_created = 0 THEN
            INSERT INTO UPObjField (domainID, objID, name, value) VALUES (p_obj_domainID, p_obj_id, 'created', UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
            SET cnt_changed = cnt_changed + 1;
        END IF;
        IF has_created = 0 OR cnt_deleted > 0 THEN
            DELETE FROM UPObjField WHERE (domainID = p_obj_domainID OR (domainID IS NULL AND p_obj_domainID IS NULL)) AND objID = p_obj_id AND name = 'modified';
            INSERT INTO UPObjField (domainID, objID, name, value) VALUES (p_obj_domainID, p_obj_id, 'modified', UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
            SET cnt_changed = cnt_changed + 1;
        END IF;
    END IF;

    SELECT IF(@allowed != 0, 1, 0) AS objs, cnt_changed AS changes;
END //
DELIMITER ;
