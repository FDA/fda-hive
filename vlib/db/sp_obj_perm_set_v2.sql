
DROP PROCEDURE IF EXISTS `sp_obj_perm_set_v2`;

DELIMITER //

CREATE PROCEDURE `sp_obj_perm_set_v2`(
    IN p_user_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_group_id BIGINT UNSIGNED,
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_obj_id BIGINT UNSIGNED,
    IN p_view_domainID BIGINT UNSIGNED,
    IN p_view_id BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED,
    IN p_flags BIGINT UNSIGNED,
    IN p_doSelectRowCount BOOLEAN
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
    IF p_obj_domainID IS NULL THEN
        SET p_obj_domainID = 0;
    END IF;
    IF p_view_domainID IS NULL THEN
        SET p_view_domainID = 0;
    END IF;
    IF p_view_id IS NULL THEN
        SET p_view_id = 0;
    END IF;
    IF p_obj_id > 0 THEN
        IF p_permissions != 0 THEN
            REPLACE INTO UPPerm
                (domainID, objID, groupID, viewDomainID, view_id, flags, bits, createTm)
            VALUES
                (p_obj_domainID, p_obj_id, p_group_id, p_view_domainID, p_view_id, p_flags, p_permissions, CURRENT_TIMESTAMP);
            IF p_doSelectRowCount THEN
                SELECT ROW_COUNT();
            END IF;
        ELSE
            DELETE FROM UPPerm
                WHERE domainID = p_obj_domainID AND objID = p_obj_id AND groupID = p_group_id AND viewDomainID = p_view_domainID AND view_id = p_view_id;
            IF p_doSelectRowCount THEN
                SELECT ROW_COUNT();
            END IF;
        END IF;
    ELSE
        IF p_doSelectRowCount THEN
            SELECT 0;
        END IF;
    END IF;
END //
DELIMITER ;
