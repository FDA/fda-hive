
DROP PROCEDURE IF EXISTS `sp_grp_get_log`;

DELIMITER //

CREATE PROCEDURE `sp_grp_get_log`(
    IN p_grp_id BIGINT,
    IN p_level BIGINT
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
    DECLARE I INT DEFAULT 0;
    DECLARE R INT DEFAULT 0;

    DROP TEMPORARY TABLE IF EXISTS tmp_reqs;
    CREATE TEMPORARY TABLE tmp_reqs (
        `reqID` BIGINT UNSIGNED NOT NULL,
        UNIQUE KEY `uniq_idx` (`reqID`)
    );
    DROP TEMPORARY TABLE IF EXISTS tmp_reqs2;
    CREATE TEMPORARY TABLE tmp_reqs2 (
        `reqID` BIGINT UNSIGNED NOT NULL,
        UNIQUE KEY `uniq_idx` (`reqID`)
    );
    INSERT IGNORE INTO tmp_reqs (reqID) VALUES(p_grp_id);
    SET R = ROW_COUNT();
    WHILE R > 0 AND I < 100 DO
        SET I = I + 1;
        INSERT IGNORE INTO tmp_reqs2 SELECT reqID FROM QPGrp WHERE grpID IN (SELECT reqID FROM tmp_reqs);
        SET R = ROW_COUNT();
        INSERT IGNORE INTO tmp_reqs SELECT reqID FROM QPGrp WHERE grpID IN (SELECT reqID FROM tmp_reqs2);
        SET R = ROW_COUNT();
    END WHILE;
    DROP TEMPORARY TABLE IF EXISTS tmp_reqs2;
    IF p_level > 0 THEN
        SELECT distinct l.reqID, l.jobID, l.level, UNIX_TIMESTAMP(l.cdate), l.txt
        FROM QPLog l JOIN tmp_reqs USING(reqID) WHERE level >= p_level ORDER BY l.cdate DESC;
    ELSE
        SELECT distinct l.reqID, l.jobID, l.level, UNIX_TIMESTAMP(l.cdate), l.txt
        FROM QPLog l JOIN tmp_reqs USING(reqID) ORDER BY l.cdate DESC;
    END IF;
    DROP TEMPORARY TABLE IF EXISTS tmp_reqs;
END //
DELIMITER ;
