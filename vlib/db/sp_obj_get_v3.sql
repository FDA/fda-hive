
DROP PROCEDURE IF EXISTS `sp_obj_get_v3`;

DELIMITER //

CREATE PROCEDURE `sp_obj_get_v3`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(65535),
    IN p_type_name_csv VARCHAR(65535),
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
    -- expand type list using hierachy into flat list
    IF p_type_name_csv IS NULL OR LENGTH(p_type_name_csv) = 0 THEN
        SET p_type_name_csv = '^base_user_type$+';
    END IF;
    SET @ts = '';
    SET @negater_ts = '';
    IF p_type_name_csv != '*' THEN
        SET @only_negate = TRUE;
        WHILE @only_negate OR LENGTH(p_type_name_csv) != 0 DO
            SET @pos = POSITION(',' IN p_type_name_csv);
            IF @pos > 0 THEN
                SET @t = TRIM(SUBSTRING(p_type_name_csv FROM 1 FOR @pos - 1));
            ELSE
                SET @pos = LENGTH(p_type_name_csv);
                SET @t = TRIM(p_type_name_csv);
            END IF;
            SET p_type_name_csv = SUBSTRING(p_type_name_csv FROM @pos + 1);

            IF @only_negate AND (@t IS NULL OR LENGTH(@t) = 0) THEN
                SET p_type_name_csv = '^base_user_type$+';
                SET @only_negate = FALSE;
            END IF;

            IF @t IS NOT NULL AND LENGTH(@t) > 0 THEN
                IF LEFT(@t, 1) = '!' THEN
                    -- negate
                    SET @negate = TRUE;
                    SET @t = SUBSTRING(@t FROM 2);
                ELSE
                    SET @negate = FALSE;
                    SET @only_negate = FALSE;
                END IF;
                IF RIGHT(@t, 1) = '+' THEN
                    -- expand type to include its children
                    SET @t = SUBSTRING(@t FROM 1 FOR LENGTH(@t) - 1);
                    SELECT GROUP_CONCAT(type_id) FROM UPType WHERE name REGEXP @t INTO @tids;
                    WHILE @tids IS NOT NULL AND LENGTH(@tids) != 0 DO
                        SET @tn = '';
                        SELECT GROUP_CONCAT(t2.type_id), GROUP_CONCAT(CONCAT('^', t2.name, '$')) FROM UPType t2 JOIN UPType t1 ON FIND_IN_SET(t1.name, t2.parent) > 0 WHERE FIND_IN_SET(t1.type_id, @tids) > 0
                        INTO @tids, @tn;
                        IF LENGTH(@tn) > 0 THEN
                            SET p_type_name_csv = CONCAT(p_type_name_csv, ',', @tn);
                        END IF;
                    END WHILE;
                END IF;
                SELECT GROUP_CONCAT(type_id) FROM UPType WHERE `name` REGEXP @t INTO @t;
                IF @negate THEN
                    SET @negater_ts = CONCAT(@negater_ts, ',', @t);
                ELSE
                    SET @ts = CONCAT(@ts, ',', @t);
                END IF;
            END IF;
        END WHILE;
        IF (@ts IS NULL OR LENGTH(@ts) = 0) AND (@negater_ts IS NULL OR LENGTH(@negater_ts) = 0) THEN
            SET @ts = -1000;
        END IF;
    END IF;

    CREATE TEMPORARY TABLE IF NOT EXISTS tmp_last_ids
    (
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        UNIQUE KEY `uniq_idx` (`domainID`,`objID`)
    );
    DELETE FROM tmp_last_ids;

    -- find objects
    SET @q = CONCAT('INSERT IGNORE INTO tmp_last_ids SELECT DISTINCT ', IF(p_total, 'SQL_CALC_FOUND_ROWS', ''), ' o.domainID, o.objID
        FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN UPObj o ON o.domainID = p.domainID AND o.objID = p.objID',
        IF(LENGTH(p_prop_filter) > 0, ' JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID ', ''),
        ' WHERE (', p_member_sql, ')');
    IF LENGTH(@ts) > 0 THEN
        SET @q = CONCAT(@q, ' AND objTypeID IN (', SUBSTRING(@ts, 2), ')');
    END IF;
    IF LENGTH(@negater_ts) > 0 THEN
        SET @q = CONCAT(@q, ' AND objTypeID NOT IN (', SUBSTRING(@negater_ts, 2), ')');
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
    SELECT IF(p_total, FOUND_ROWS(), 0) AS TOTAL_ROWS;

    SELECT t.domainID, t.objID, NULL AS ionID, '_type' AS `name`, o.*
        FROM tmp_last_ids t JOIN UPObj o ON o.domainID = t.domainID AND o.objID = t.objID
        ORDER BY o.objID DESC;

    SET @q = CONCAT('SELECT p.domainID, p.objID, NULL AS ionID, \'_acl\' as `name`, p.flags, p.bits
        FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN tmp_last_ids t ON
        p.domainID = t.domainID AND p.objID = t.objID
        WHERE ', p_member_sql);
    PREPARE x FROM @q;
    EXECUTE x;

    -- return requested properties
    IF p_show_perm THEN
        -- full permisisons
        SELECT t.domainID, t.objID, NULL AS ionID, '_perm' AS `name`, groupId, view_id, flags, bits
            FROM tmp_last_ids t JOIN UPPerm p ON
            p.domainID = t.domainID AND p.objID = t.objID;
    END IF;

END //
DELIMITER ;
