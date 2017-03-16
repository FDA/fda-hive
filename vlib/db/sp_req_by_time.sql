
DROP PROCEDURE IF EXISTS `sp_req_by_time`;

DELIMITER //

CREATE PROCEDURE `sp_req_by_time`(
    IN p_user_id BIGINT UNSIGNED,
    IN p_since_time BIGINT, -- : unix timestamp; <= 0 means since beginning of time
    IN p_to_time BIGINT, -- : unix timestamp
    IN p_field CHAR(16) -- : 'created' or 'completed'
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
    IF p_field != 'created' AND p_field != 'completed' THEN
        SET p_field = 'created';
    END IF;
    IF p_field = 'created' THEN
        SELECT reqID, UNIX_TIMESTAMP(cdate) AS 'created'
            FROM QPReq
            WHERE userID = p_user_id AND cdate > FROM_UNIXTIME(p_since_time) AND cdate <= FROM_UNIXTIME(p_to_time);
    ELSE
        SELECT reqID, IF(stat >= 5, UNIX_TIMESTAMP(GREATEST(cdate, takenTm, aliveTm, doneTm)), UNIX_TIMESTAMP(purgeTm)) AS 'completed', UNIX_TIMESTAMP(cdate) AS 'created', UNIX_TIMESTAMP(takenTm) AS takenTm, UNIX_TIMESTAMP(aliveTm) AS 'aliveTm', UNIX_TIMESTAMP(doneTm) AS 'doneTm'
            FROM QPReq
            WHERE userID = p_user_id AND
                ((stat >= 5 AND UNIX_TIMESTAMP(GREATEST(cdate, takenTm, aliveTm, doneTm)) > p_since_time AND UNIX_TIMESTAMP(GREATEST(cdate, takenTm, aliveTm, doneTm)) <= p_to_time) OR
                 (stat < 5 AND UNIX_TIMESTAMP(purgeTm) > p_since_time AND UNIX_TIMESTAMP(purgeTm) <= p_to_time));
    END IF;
END //
DELIMITER ;
