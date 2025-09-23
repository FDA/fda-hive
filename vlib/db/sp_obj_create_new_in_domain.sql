
DROP PROCEDURE IF EXISTS `sp_obj_create_new_in_domain`;

DELIMITER //

CREATE PROCEDURE `sp_obj_create_new_in_domain`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_type_domainID BIGINT UNSIGNED,
    IN p_typeID BIGINT UNSIGNED,
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED,
    IN p_flags BIGINT UNSIGNED,
    -- authorization object for creating (not importing) a new object in the given domain
    IN p_auth_obj_domainID BIGINT UNSIGNED,
    IN p_auth_obj_id BIGINT UNSIGNED,
    IN p_auth_permissions BIGINT UNSIGNED
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
    DECLARE l_err INT;
    -- 1062 Duplicate entry '0-10' for key 'uniq'
    DECLARE CONTINUE HANDLER FOR 1062 SET l_err = 1062;

    SET p_typeID = IFNULL(p_typeID, 0);
    SET p_obj_domainID = IFNULL(p_obj_domainID, 0);
    SET p_auth_obj_domainID = IFNULL(p_auth_obj_domainID, 0);
    SET p_auth_obj_id = IFNULL(p_auth_obj_id, 0);
    SET @obj_id = 0;

    CALL sp_permission_check_v2(p_group_id, p_member_sql, p_auth_obj_domainID, p_auth_obj_id, p_auth_permissions, @allowed);

    IF IFNULL(@allowed, 0) != 0 AND p_obj_domainID > 0 THEN
        SET l_err = 0;
        INSERT INTO UPObj (`domainID`, `objID`, `objTypeDomainID`, `objTypeID`, `creatorID`)
            SELECT p_obj_domainID, MAX(objID) + 1, p_type_domainID, p_typeID, p_group_id
                FROM UPObj WHERE domainID = p_obj_domainID;

        IF l_err != 0 OR ROW_COUNT() != 1 THEN
            SET @obj_id = 0;
            SET @mt = CONCAT('Failed to create object in non-local domain: ', l_err);
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
        END IF;

        SELECT MAX(objID) FROM UPObj
            WHERE domainID = p_obj_domainID AND creatorID = p_group_id AND
                objTypeDomainID = p_type_domainID AND objTypeID = p_typeID
            INTO @obj_id;

        IF @obj_id > 0 THEN
            CALL sp_obj_perm_set_v2(p_group_id, p_member_sql, p_group_id, p_obj_domainID, @obj_id, NULL, NULL, p_permissions, p_flags, FALSE);
            INSERT INTO UPObjField (domainID, objID, `name`, `group`, `value`)
                VALUES (IF(p_obj_domainID = 0, NULL, p_obj_domainID), @obj_id, 'created', NULL, UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
        END IF;
    END IF;

    SELECT p_obj_domainID AS domainID, @obj_id AS id;
END //
DELIMITER ;
