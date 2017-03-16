
DROP PROCEDURE IF EXISTS `sp_type_get_v5`;

DELIMITER //

CREATE PROCEDURE `sp_type_get_v5`(
    IN p_where_sql VARCHAR(65535),
    IN p_minimal BOOLEAN,
    IN p_type_type_domain BIGINT UNSIGNED,
    IN p_type_type_id BIGINT UNSIGNED
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
    IF IFNULL(p_type_type_id, 0) = 0 OR p_type_type_domain IS NULL THEN
        SELECT objTypeDomainID, objTypeID
            FROM UPObj
            WHERE objTypeDomainID = domainID AND objTypeID = objID
            INTO p_type_type_domain, p_type_type_id;
    END IF;
    SELECT p_type_type_domain AS domainID, p_type_type_id AS objID;

    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v5_ids;
    CREATE TEMPORARY TABLE tmp_type_get_v5_ids
    (
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        UNIQUE KEY `uniq_idx` (`domainID`,`objID`)
    );
    SET @q = CONCAT('INSERT IGNORE INTO tmp_type_get_v5_ids (domainID, objID) ',
        'SELECT o.domainID, o.objID ',
        'FROM UPObj o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID ',
        'WHERE o.objTypeDomainID = ', p_type_type_domain, ' AND o.objTypeID = ', p_type_type_id, ' AND (', p_where_sql, ')');
    PREPARE x FROM @q;
    EXECUTE x;

    IF p_minimal THEN
        SELECT o.domainID AS domainID, o.objID AS objID, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
            FROM tmp_type_get_v5_ids o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE f.`name` IN ('name', 'title', 'description', 'parent', 'field_include_type');
    ELSE
        SELECT o.domainID AS domainID, o.objID AS objID, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
            FROM tmp_type_get_v5_ids o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID;
    END IF;
    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v5_ids;
END //
DELIMITER ;
