
DROP PROCEDURE IF EXISTS `sp_sys_log`;

DELIMITER //

CREATE PROCEDURE `sp_sys_log`(
    IN p_hours BIGINT UNSIGNED,
    IN p_msg_test VARCHAR(2048)
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

    IF p_hours <= 0 OR p_hours IS NULL THEN
        SET p_hours = 5;
    END IF;
    IF LENGTH(p_msg_test) <= 0 OR p_msg_test IS NULL THEN
        SET p_msg_test = '.*';
    END IF;

    SELECT
        (SELECT GROUP_CONCAT(DISTINCT IFNULL(g.masterGrpID, g.grpID))
            FROM QPGrp g WHERE g.reqid = l.reqid) AS grpID,
        j.hostname, COUNT(l.txt) AS repeated, l.level, REPLACE(l.txt, '\n', ' ') AS message
        FROM QPLog l JOIN QPJob j USING (jobid)
        WHERE ((l.level > 4 AND l.level < 100) OR (l.level > 300)) AND (l.cdate >= (NOW() - INTERVAL p_hours HOUR))
        GROUP BY j.hostname, l.txt
        HAVING l.txt REGEXP p_msg_test
        ORDER BY j.hostname;
END //
DELIMITER ;
