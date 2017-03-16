
DROP PROCEDURE IF EXISTS `sp_obj_create`;

DELIMITER //

CREATE PROCEDURE `sp_obj_create`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(4096),
    IN p_type_name VARCHAR(255),
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

    SELECT type_id FROM UPType where name = p_type_name
    INTO @tid;

    -- new object
    SET @id = 0;
    INSERT INTO UPObjMax (`objID`) VALUES (NULL);
    SET @id = LAST_INSERT_ID();
    SET l_err = 0;
    INSERT INTO UPObj (`domainID`, `objID`, `objTypeDomainID`, `objTypeID`, `creatorID`)
    VALUES (0, @id, 1954115685, @tid, p_group_id);
    IF l_err != 0 THEN
        SET @id = 0; -- error should occur here only on duplicate key
        SET @mt = CONCAT('Failed to create object: ', l_err);
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF @id > 0 THEN
        CALL sp_obj_perm_set(p_group_id, p_member_sql, p_group_id, @id, NULL, p_permissions, p_flags, false);
        INSERT INTO UPObjField (objID, name, `group`, value) VALUES (@id, 'created', NULL, UNIX_TIMESTAMP(CURRENT_TIMESTAMP));
    END IF;

    SELECT @id AS id;
END //
DELIMITER ;
