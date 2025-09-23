
DROP PROCEDURE IF EXISTS `sp_obj_erase_v2`;

DELIMITER //

CREATE PROCEDURE `sp_obj_erase_v2`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_obj_id BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED,
    IN p_perm_read_browse BIGINT UNSIGNED,
    IN p_perm_flag_deny BIGINT UNSIGNED,
    IN p_days_delay BIGINT UNSIGNED
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

    CALL sp_permission_check_v2(p_group_id, p_member_sql, p_obj_domainID, p_obj_id, p_permissions, @allowed);

    IF @allowed != 0 THEN
        if p_days_delay >= 0 THEN
            UPDATE UPObj SET softExpiration = NOW() + INTERVAL p_days_delay DAY
                WHERE domainID = IFNULL(p_obj_domainID, 0) AND objID = p_obj_id;
        ELSE
            UPDATE UPObj SET softExpiration = NULL
                WHERE domainID = IFNULL(p_obj_domainID, 0) AND objID = p_obj_id;
        END IF;
        SELECT ROW_COUNT();
    ELSE
        DELETE FROM UPPerm WHERE domainID = IFNULL(p_obj_domainID, 0) AND objID = p_obj_id AND groupID = p_group_id;
        -- still can browse/read?
        CALL sp_permission_check_v2(p_group_id, p_member_sql, p_obj_domainID, p_obj_id, p_perm_read_browse, @can_browse_read);
        IF @can_browse_read != 0 THEN
            SELECT 0;
        ELSE
            SELECT 1;
        END IF;
    END IF;

END //
DELIMITER ;
