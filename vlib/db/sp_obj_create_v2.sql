
DROP PROCEDURE IF EXISTS `sp_obj_create_v2`;

DELIMITER //

CREATE PROCEDURE `sp_obj_create_v2`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_type_domainID BIGINT UNSIGNED,
    IN p_typeID BIGINT UNSIGNED,
    IN p_obj_domainID BIGINT UNSIGNED,
    IN p_objID BIGINT UNSIGNED,
    IN p_permissions BIGINT UNSIGNED,
    IN p_flags BIGINT UNSIGNED
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
    SET p_objID = IFNULL(p_objID, 0);

    IF p_typeID = 0 THEN
        -- missing type id
        SET @id = 0;
    ELSEIF p_objID != 0 THEN
        IF p_obj_domainID = 0 THEN
            -- import is allowed for NOT local domains only
            SET @id = 0;
        ELSE
            -- valid import
            SET p_type_domainID = IFNULL(p_type_domainID, 0);
            SET l_err = 0;
            INSERT INTO UPObj (`domainID`, `objID`, `objTypeDomainID`, `objTypeID`, `creatorID`)
            VALUES (p_obj_domainID, p_objID, p_type_domainID, p_typeID, p_group_id);
            IF l_err != 0 THEN
                SET @id = 0; -- error should occur here only on duplicate key
                SET @mt = CONCAT('Failed to import object: ', l_err);
                SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
            ELSE
                SET @id = p_objID;
            END IF;
        END IF;
    ELSEIF p_obj_domainID != 0 THEN
        -- creation is allowed only for local objects
        SET @id = 0;
    ELSE
        -- new object in local domain
        SET p_type_domainID = IFNULL(p_type_domainID, 0);
        INSERT INTO UPObjMax (`objID`) VALUES (NULL);
        SET @id = LAST_INSERT_ID();
        SET l_err = 0;
        INSERT INTO UPObj (`domainID`, `objID`, `objTypeDomainID`, `objTypeID`, `creatorID`)
        VALUES (0, @id, p_type_domainID, p_typeID, p_group_id);
        IF l_err != 0 THEN
            SET @id = 0; -- error should occur here only on duplicate key
            SET @mt = 'Failed to create object in local domain';
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
        END IF;
    END IF;

    IF @id > 0 THEN
        CALL sp_obj_perm_set_v2(p_group_id, p_member_sql, p_group_id, p_obj_domainID, @id, NULL, NULL, p_permissions, p_flags, false);
        INSERT INTO UPObjField (domainID, objID, `name`, `group`, `value`)
            VALUES (IF(p_obj_domainID = 0, NULL, p_obj_domainID), @id, 'created', NULL, UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
    END IF;

    SELECT p_obj_domainID AS domainID, @id AS id;
END //
DELIMITER ;
