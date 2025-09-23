
DROP PROCEDURE IF EXISTS `sp_type_get_v6`;

DELIMITER //

CREATE PROCEDURE `sp_type_get_v6`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(21844),
    IN p_where_fetch_sql VARCHAR(21844),
    IN p_where_full_fetch_sql VARCHAR(21844),
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

    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v6_ids;
    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v6_ids2;
    CREATE TEMPORARY TABLE tmp_type_get_v6_ids
    (
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        `is_full_fetch_fg` BOOL NOT NULL,
        UNIQUE KEY `uniq_idx` (`domainID`,`objID`,`is_full_fetch_fg`),
        INDEX (`domainID`,`objID`)
    );
    CREATE TEMPORARY TABLE tmp_type_get_v6_ids2
    (
        `domainID` BIGINT UNSIGNED NOT NULL,
        `objID` BIGINT UNSIGNED NOT NULL,
        `is_full_fetch_fg` BOOL NOT NULL,
        INDEX (`domainID`,`objID`)
    );
    SET @q = CONCAT('INSERT IGNORE INTO tmp_type_get_v6_ids (domainID, objID, is_full_fetch_fg) ',
        'SELECT o.domainID, o.objID, (', p_where_full_fetch_sql, ') ',
        'FROM UPGroup g JOIN UPPerm p USING (groupID) JOIN UPObj o ON o.domainID = p.domainID AND o.objID = p.objID JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID ',
        'WHERE o.objTypeDomainID = ', p_type_type_domain, ' AND o.objTypeID = ', p_type_type_id, ' AND (', p_member_sql, ') AND (', p_where_fetch_sql, ') ',
        'AND ((softExpiration IS NULL) OR (softExpiration > CURRENT_TIMESTAMP)) AND ((hardExpiration IS NULL) OR (hardExpiration > CURRENT_TIMESTAMP))');
--  SELECT @q;
    PREPARE x FROM @q;
    EXECUTE x;

--  SELECT * FROM tmp_type_get_v6_ids;
    /* If a type was selected as both full-fetch and non-full-fetch, treat it as full-fetch, to allow e.g.
       p_where_full_fetch_sql = '(f.`name` = \'prefetch\' AND f.`value` > 0)';
       which in @q would select multiple rows, some with is_full_fetch_fg = 1 and others (corresponding
       to cases where f.`name` != 'prefetch) with spurious is_full_fetch_fg = 0. */
    /* Unfortunately, MySQL doesn't allow self-join on a temporary table - so we need two temp tables */
    INSERT IGNORE INTO tmp_type_get_v6_ids2 (domainID, objID, is_full_fetch_fg)
        SELECT domainID, objID, is_full_fetch_fg FROM tmp_type_get_v6_ids;
    /* MySQL safe mode doesn't allow delete from this join with any sane syntax :( */
    SET @bck_SQL_SAFE_UPDATES = @@SQL_SAFE_UPDATES;
    SET SQL_SAFE_UPDATES = 0;
    DELETE o FROM tmp_type_get_v6_ids o LEFT JOIN tmp_type_get_v6_ids2 o2 USING (domainID, objID)
        WHERE o2.is_full_fetch_fg AND NOT o.is_full_fetch_fg;
    SET SQL_SAFE_UPDATES = @bck_SQL_SAFE_UPDATES;
--  SELECT * FROM tmp_type_get_v6_ids;

    SELECT o.domainID AS domainID, o.objID AS objID, o.is_full_fetch_fg AS is_full_fetch_fg, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
        FROM tmp_type_get_v6_ids o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
        WHERE (o.is_full_fetch_fg OR f.`name` IN ('name', 'title', 'description', 'parent', 'field_include_type'));

    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v6_ids;
    DROP TEMPORARY TABLE IF EXISTS tmp_type_get_v6_ids2;
END //
DELIMITER ;
