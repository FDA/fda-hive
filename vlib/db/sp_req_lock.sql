
DROP PROCEDURE IF EXISTS `sp_req_lock`;

DELIMITER //

CREATE PROCEDURE `sp_req_lock`(
    IN p_req_id BIGINT, -- : request id (if positive) or other handle (if negative)
    IN p_key VARCHAR(767), -- : file path or other key; 767 is max length of mysql varchar key
    IN p_purge_secs BIGINT, -- : max interval for which the lock is valid
    IN p_force INT, -- : 0 for normal behavior; > 0 to force-update lock no matter what; < 0 to never update (test-only)
    IN p_done_stat INT -- : sQPrideBase::eQPReqStatus_Done, i.e. min value of QPReq.stat indicating request is done
)
    MODIFIES SQL DATA
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

    IF p_done_stat IS NULL OR p_done_stat < 1 THEN
        SET p_done_stat = 5;
    END IF;

    IF p_purge_secs IS NULL OR p_purge_secs < 1 THEN
        SET p_purge_secs = 10;
    END IF;

    IF p_force >= 0 THEN
        INSERT IGNORE INTO QPLock(`reqID`, `key`, `purgeTm`) VALUES(p_req_id, p_key, NOW() + INTERVAL p_purge_secs SECOND);
        IF ROW_COUNT() = 0 THEN
            UPDATE IGNORE QPLock l LEFT JOIN QPReq r USING (reqID)
                SET l.reqID = p_req_id, l.purgeTm = NOW() + INTERVAL p_purge_secs SECOND
                WHERE l.`key` = p_key AND (p_force OR l.reqID = p_req_id OR l.purgeTm < NOW() OR r.stat >= p_done_stat);
        END IF;

        SELECT `reqID`, `key`, `purgeTm` FROM QPLock WHERE `key` = p_key;
    ELSE
        /* find locks which are not expired and either correspond to alive requests
         * or have negative reqID (meaning special handle, not request) */
        SELECT l.`reqID`, l.`key`, l.`purgeTm` FROM QPLock l LEFT JOIN QPReq r USING (reqID)
            WHERE l.`key` = p_key AND l.purgeTm >= NOW() AND (l.reqID < 0 OR r.stat < p_done_stat);
    END IF;

END //
DELIMITER ;
