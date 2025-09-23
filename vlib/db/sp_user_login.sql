
DROP PROCEDURE IF EXISTS `sp_user_login`;

DELIMITER //

CREATE PROCEDURE `sp_user_login`(
    IN p_user_id BIGINT UNSIGNED,
    IN p_source VARCHAR(128),
    IN p_time BIGINT UNSIGNED,
    IN p_persistent TINYINT(1)
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


    DECLARE ses_used BIGINT DEFAULT 0;
    DECLARE ses_max BIGINT DEFAULT 0;
    DECLARE ses_close BIGINT DEFAULT 0;
    DECLARE v_oid BIGINT UNSIGNED DEFAULT 0;
    DECLARE v_logCount BIGINT UNSIGNED DEFAULT 0;

    SET ses_used = 0;
    SELECT COUNT(*) FROM UPSession
        WHERE user_id = p_user_id AND ended IS NULL AND persistent = FALSE ORDER BY started ASC
    INTO ses_used;

    SET ses_max = 0;
    SELECT max_sessions FROM UPUser WHERE userID = p_user_id
    INTO ses_max;

    SET @tm = FROM_UNIXTIME(p_time);

    SET ses_close = ses_used - (ses_max - 1);
    IF ses_close > 0 THEN
        UPDATE UPSession SET ended = @tm, origin = CONCAT('expired: ', IFNULL(origin,'')), rnd = NULL
            WHERE user_id = p_user_id AND ended IS NULL AND persistent = FALSE
            ORDER BY accessed ASC LIMIT ses_close;
    END IF;

    SET @r = FLOOR(-32768 + (RAND() * 65535));
    INSERT INTO UPSession (user_id, rnd, origin, started, accessed, persistent)
        VALUES (p_user_id, @r, p_source, @tm, @tm, p_persistent);

    SET v_oid = LAST_INSERT_ID();
    IF v_oid > 0 THEN
        UPDATE UPUser SET loginTm = NOW(), logCount = IF(logCount > 0, logCount + 1, 0), login_failed_count = 0
            WHERE userID = p_user_id;
        SELECT IF(logCount > 0, logCount - 1, 0) FROM UPUser WHERE userID = p_user_id
        INTO v_logCount;
    END IF;

    SELECT v_oid AS id, @r AS rnd, v_logCount as logCount;
END //
DELIMITER ;
