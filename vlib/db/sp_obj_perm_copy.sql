
DROP PROCEDURE IF EXISTS `sp_obj_perm_copy`;

DELIMITER //

CREATE PROCEDURE `sp_obj_perm_copy`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_obj_id_from BIGINT UNSIGNED,
    IN p_obj_id_to BIGINT UNSIGNED
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

    IF p_obj_id_from > 0 AND p_obj_id_to > 0 AND p_obj_id_from != p_obj_id_to THEN
        REPLACE INTO UPPerm
            (domainID, objID, groupID, viewDomainID, view_id, flags, bits, createTm)
        SELECT 0, p_obj_id_to, groupID, 0, view_id, flags, bits, CURRENT_TIMESTAMP
            FROM UPPerm WHERE domainID = 0 AND objID = p_obj_id_from;
        SELECT ROW_COUNT();
    ELSE
        SELECT 0;
    END IF;
END //
DELIMITER ;
