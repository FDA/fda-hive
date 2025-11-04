
DROP PROCEDURE IF EXISTS `sp_obj_get_v5`;

DELIMITER //

CREATE PROCEDURE `sp_obj_get_v5`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(21844),
    IN p_type_filter MEDIUMTEXT,
    IN p_obj_filter MEDIUMTEXT,
    IN p_prop_filter MEDIUMTEXT,
    IN p_is_expired BIGINT SIGNED,
    IN p_start BIGINT UNSIGNED,
    IN p_count BIGINT UNSIGNED,
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
    DECLARE JJ VARCHAR(1024) DEFAULT NULL;

    DROP TEMPORARY TABLE IF EXISTS st1;
    CREATE TEMPORARY TABLE st1  (
      `domainID` BIGINT UNSIGNED NOT NULL,
      `objID` BIGINT UNSIGNED NOT NULL,
      UNIQUE KEY `uniq_idx` (`domainID`,`objID`)
    );
    DROP TEMPORARY TABLE IF EXISTS st2;
    CREATE TEMPORARY TABLE st2 LIKE st1;

    CREATE TEMPORARY TABLE IF NOT EXISTS tmp_last_ids
    (
        `sid` INT DEFAULT 0,
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        `created` TIMESTAMP NOT NULL,
        UNIQUE (`sid`, `domainID`, `objID`),
        INDEX  (`sid`),
        INDEX  (`created`)
    );
    -- connection variable to provide nested call to this stored proc and sp_obj_prop
    SET @OBJSID = IFNULL(@OBJSID, 0) + 1;

    IF LENGTH(p_prop_filter) > 0 THEN
        SET @q1 = CONCAT('INSERT IGNORE INTO st1 SELECT IFNULL(f.domainID, 0), f.objID FROM UPObjField f WHERE ', p_prop_filter);
        PREPARE x1 FROM @q1;
        EXECUTE x1;
        SET JJ = ' JOIN st1 s ON o.domainID = s.domainID AND o.objID = s.objID';
    END IF;

    SET @q = CONCAT('INSERT IGNORE INTO st2 SELECT o.domainID, o.objID FROM UPObj o', IFNULL(JJ, ''), ' WHERE TRUE');
    IF p_type_filter IS NOT NULL AND LENGTH(p_type_filter) > 0 THEN
        SET @q = CONCAT(@q, ' AND (', p_type_filter, ')');
    END IF;
    IF LENGTH(p_obj_filter) > 0 THEN
        SET @q = CONCAT(@q, ' AND (', p_obj_filter, ')');
    END IF;
    CASE p_is_expired
        -- only trashed
        WHEN 1 THEN SET @q = CONCAT(@q, ' AND ((softExpiration IS NOT NULL) AND (softExpiration <= CURRENT_TIMESTAMP)) AND ((hardExpiration IS NULL) OR (hardExpiration > CURRENT_TIMESTAMP))');
        -- normal and trashed
        WHEN 2 THEN SET @q = CONCAT(@q, ' AND ((hardExpiration IS NULL) OR (hardExpiration <= CURRENT_TIMESTAMP))');
        -- all: normal, trashed and deleted (like a sysadmin call?)
        WHEN -1 THEN SET @q = @q; -- no-op
        -- normal
        ELSE SET @q = CONCAT(@q, ' AND ((softExpiration IS NULL) OR (softExpiration > CURRENT_TIMESTAMP)) AND ((hardExpiration IS NULL) OR (hardExpiration > CURRENT_TIMESTAMP))');
    END CASE;
    PREPARE x FROM @q;
    EXECUTE x;
    TRUNCATE st1;

    -- not denied, on hold or temp revoked, see UPerm.hpp
    SET @q = CONCAT('INSERT IGNORE INTO st1 SELECT o.domainID, o.objID FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN st2 o ON o.domainID = p.domainID AND o.objID = p.objID WHERE ((p.flags & 0x30000001) = 0) AND (', p_member_sql, ')');
    PREPARE x FROM @q;
    EXECUTE x;
    DROP TEMPORARY TABLE IF EXISTS st2;

    INSERT IGNORE INTO tmp_last_ids SELECT @OBJSID, t.*, FROM_UNIXTIME(fc.value)
        FROM st1 t JOIN UPObjField fc ON (fc.domainID = t.domainID OR (fc.domainID IS NULL AND t.domainID = 0)) AND t.objID = fc.objID
    WHERE (fc.name = 'created');
    SET @TOTAL_ROWS = 0;
    SELECT COUNT(*) FROM tmp_last_ids WHERE sid = @OBJSID INTO @TOTAL_ROWS;
    -- first row in result is total count
    SELECT @TOTAL_ROWS, @OBJSID;
    DROP TEMPORARY TABLE IF EXISTS st1;

    -- trim result to page and size
    IF p_start > 0 THEN
        DELETE FROM tmp_last_ids WHERE sid = @OBJSID ORDER BY created DESC LIMIT p_start;
        SELECT COUNT(*) FROM tmp_last_ids WHERE sid = @OBJSID INTO @TOTAL_ROWS;
    END IF;
    IF p_count > 0 AND @TOTAL_ROWS > p_count THEN
        SET p_count = @TOTAL_ROWS - p_count;
        DELETE FROM tmp_last_ids WHERE sid = @OBJSID ORDER BY created ASC LIMIT p_count;
    END IF;
-- select * FROM tmp_last_ids WHERE sid = @OBJSID;

    SELECT t.domainID, t.objID, NULL AS ionID, '_type' AS `name`, o.objTypeDomainID, o.objTypeID
        FROM tmp_last_ids t JOIN UPObj o ON o.domainID = t.domainID AND o.objID = t.objID
        WHERE t.sid = @OBJSID
        ORDER BY t.created DESC, o.objID DESC;

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
