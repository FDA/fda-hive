
DROP PROCEDURE IF EXISTS `sp_user_usage_objs`;

DELIMITER //

CREATE PROCEDURE `sp_user_usage_objs`(
    IN p_path_csv VARCHAR(16384)
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
    SET @path_cond = '';
    IF p_path_csv IS NOT NULL THEN
        WHILE LENGTH(p_path_csv) != 0 DO
            SET @pos_comma = POSITION(',' IN p_path_csv);
            IF @pos_comma > 0 THEN
                SET @p = TRIM(SUBSTRING(p_path_csv FROM 1 FOR @pos_comma - 1));
                SET p_path_csv = SUBSTRING(p_path_csv FROM @pos_comma + 1);
            ELSE
                SET @p = TRIM(p_path_csv);
                SET p_path_csv = '';
            END IF;
            IF @p IS NOT NULL AND LENGTH(@p) > 0 AND @p != '*' THEN
                SET @is_prefix = IF(SUBSTRING(@p FROM -1) = '/', TRUE, FALSE);
                SET @path_cond = CONCAT(@path_cond, IF(LENGTH(@path_cond) = 0, ' WHERE ', ' OR '), 'g.groupPath', IF(@is_prefix, ' LIKE ', ' = '), '\'', @p, IF(@is_prefix, '%\'', '\''));
            END IF;
        END WHILE;
    END IF;

    -- find type type id
    SELECT domainID, objID FROM UPObj WHERE objTypeDomainID = domainID AND objTypeID = objID
    INTO @tdid, @toid;

    -- find type named 'u-usage'
    SELECT o.domainID, o.objID FROM UPObj o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
    WHERE o.objTypeDomainID = @tdid AND o.objTypeID = @toid AND f.name = 'name' AND f.value = 'u-usage'
    INTO @tdid_uu, @toid_uu;

    SET @q = CONCAT('SELECT o.domainID, o.objID, NULL AS ionID, g.userID FROM ',
                    ' UPObj o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainaD IS NULL AND o.domainID = 0)) AND f.objID = o.objID AND f.`name` = \'user\'',
                    ' JOIN (SELECT DISTINCT userID FROM UPGroup', @path_cond, ') g ON f.value = g.userID',
                    ' WHERE o.objTypeDomainID = ', @tdid_uu, ' AND o.objTypeID = ', @toid_uu,
                    ' ORDER BY g.userID');
    PREPARE x from @q;
    EXECUTE x;
END //
DELIMITER ;
