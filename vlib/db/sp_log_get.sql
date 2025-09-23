
DROP PROCEDURE IF EXISTS `sp_log_get`;

DELIMITER //

CREATE PROCEDURE `sp_log_get`(
    p_req BIGINT UNSIGNED,
    p_isGrp BOOLEAN,
    p_job BIGINT UNSIGNED,
    p_level SMALLINT
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
    IF p_req IS NOT NULL AND p_req != 0 THEN
        IF p_isGrp THEN
            SELECT l.reqID, l.jobID, l.level, UNIX_TIMESTAMP(l.cdate), l.txt
                FROM QPLog l JOIN QPGrp g USING (reqID) WHERE (g.grpID = p_req OR g.masterGrpId = p_req OR g.reqID = p_req) AND level >= p_level ORDER BY l.cdate DESC;
            IF FOUND_ROWS() = 0 THEN
                SELECT reqId, jobID, level, UNIX_TIMESTAMP(cdate), txt FROM QPLog WHERE reqID = p_req AND level >= p_level ORDER BY cdate DESC;
            END IF;
        ELSE
            IF p_job IS NOT NULL AND p_job != 0 THEN
                SELECT reqID, jobID, level, UNIX_TIMESTAMP(cdate), txt FROM QPLog WHERE reqID = p_req AND jobID = p_job AND level >= p_level ORDER BY cdate DESC;
            ELSE
                SELECT reqID, jobID, level, UNIX_TIMESTAMP(cdate), txt FROM QPLog WHERE reqID = p_req AND level >= p_level ORDER BY cdate DESC;
            END IF;
        END IF;
    ELSEIF p_job IS NOT NULL AND p_job != 0 THEN
        SELECT reqID, jobID, level, UNIX_TIMESTAMP(cdate), txt FROM QPLog WHERE jobID = p_job AND level >= p_level ORDER BY cdate DESC;
    END IF;

END //
DELIMITER ;
