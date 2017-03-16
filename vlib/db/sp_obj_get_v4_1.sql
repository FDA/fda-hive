
DROP PROCEDURE IF EXISTS `sp_obj_get_v4_1`;

DELIMITER //

CREATE PROCEDURE `sp_obj_get_v4_1`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(65535),
    IN p_type_filter MEDIUMTEXT,
    IN p_obj_filter MEDIUMTEXT,
    IN p_prop_filter MEDIUMTEXT,
    IN p_is_expired BIGINT SIGNED, -- : 1 - soft, 2 - hard, -1 - any (expired or not)
    IN p_start BIGINT UNSIGNED,
    IN p_count BIGINT UNSIGNED,
    IN p_total BOOLEAN,
    IN p_show_perm BOOLEAN
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
    CREATE TEMPORARY TABLE IF NOT EXISTS tmp_last_ids
    (
        `sid` INT DEFAULT 0,
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        UNIQUE (`sid`, `domainID`, `objID`),
        INDEX  (`sid`)
    );
    -- connection variable to provide nested call to this stored proc and sp_obj_prop
    SET @OBJSID = IFNULL(@OBJSID, 0) + 1;

    -- find objects
    SET @q = CONCAT('INSERT IGNORE INTO tmp_last_ids SELECT DISTINCT ', IF(p_total, 'SQL_CALC_FOUND_ROWS ', ''), @OBJSID, ', o.domainID, o.objID
        FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN UPObj o ON o.domainID = p.domainID AND o.objID = p.objID',
    IF(LENGTH(p_prop_filter) > 0, ' JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID', ''),
        ' WHERE (', p_member_sql, ')');
    IF p_type_filter IS NOT NULL AND LENGTH(p_type_filter) > 0 THEN
        SET @q = CONCAT(@q, ' AND (', p_type_filter, ')');
    END IF;
    IF LENGTH(p_obj_filter) > 0 THEN
        SET @q = CONCAT(@q, ' AND (', p_obj_filter, ')');
    END IF;
    IF LENGTH(p_prop_filter) > 0 THEN
        SET @q = CONCAT(@q, ' AND (', p_prop_filter, ')');
    END IF;
    CASE p_is_expired
        WHEN 1 THEN SET @q = CONCAT(@q, ' AND ((softExpiration IS NOT NULL) AND (softExpiration <= CURRENT_TIMESTAMP)) AND ((hardExpiration IS NULL) OR (hardExpiration > CURRENT_TIMESTAMP))');
        WHEN 2 THEN SET @q = CONCAT(@q, ' AND ((hardExpiration IS NULL) AND (hardExpiration <= CURRENT_TIMESTAMP))');
        WHEN -1 THEN SET @q = @q; -- no-op
        ELSE SET @q = CONCAT(@q, ' AND ((softExpiration IS NULL) OR (softExpiration > CURRENT_TIMESTAMP)) AND ((hardExpiration IS NULL) OR (hardExpiration > CURRENT_TIMESTAMP))');
    END CASE;
    SET @q = CONCAT(@q, ' ORDER BY o.objID DESC');
    IF p_count > 0  THEN
        SET @q = CONCAT(@q, ' LIMIT ', p_count);
    END IF;
    IF p_start > 0  THEN
        SET @q = CONCAT(@q, ' OFFSET ', p_start);
    END IF;

--  select @q;

    PREPARE x FROM @q;
    EXECUTE x;
    -- first row is total count
    SELECT IF(p_total, FOUND_ROWS(), 0) AS TOTAL_ROWS, @OBJSID AS SID;

    SELECT t.domainID, t.objID, NULL AS ionID, '_type' AS `name`, o.objTypeDomainID, o.objTypeID
        FROM tmp_last_ids t JOIN UPObj o ON o.domainID = t.domainID AND o.objID = t.objID
        WHERE t.sid = @OBJSID
        ORDER BY o.objID DESC;

    SET @q = CONCAT('SELECT p.domainID, p.objID, NULL AS ionID, ''_acl'' as `name`, p.flags, p.bits, p.viewDomainID, p.view_id
        FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN tmp_last_ids t ON p.domainID = t.domainID AND p.objID = t.objID
        WHERE t.sid = ', @OBJSID, ' AND ', p_member_sql);
    PREPARE x FROM @q;
    EXECUTE x;

    IF p_show_perm THEN
        -- full permisisons
        SELECT t.domainID, t.objID, NULL AS ionID, '_perm' AS `name`, groupId, flags, bits, viewDomainID, view_id
            FROM tmp_last_ids t JOIN UPPerm p ON p.domainID = t.domainID AND p.objID = t.objID
            WHERE t.sid = @OBJSID;
    END IF;

END //
DELIMITER ;
