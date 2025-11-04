
DROP PROCEDURE IF EXISTS `sp_grp_submit_v2`;

DELIMITER //

CREATE PROCEDURE `sp_grp_submit_v2`(
    IN p_svc_id INT,
    IN p_cleanup_days INT,
    IN p_start INT, -- first request created will have inParallel = p_start + 1
    IN p_priority_cnt_csv LONGTEXT, -- comma-separated list of priority,count, e.g. "1,10,2,90,3,900"; or single negative value to use for all requests; or single non-negative value to increment logarithmically
    IN p_subip BIGINT,
    IN p_cnt INT, -- number of requests to create
    IN p_user_id BIGINT,
    IN p_grp_id BIGINT -- 0 to create a new group
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

   DECLARE l_req BIGINT;
   DECLARE l_grp BIGINT DEFAULT 0;
   DECLARE l_currentbase INT DEFAULT 10;
   DECLARE il INT DEFAULT IFNULL(p_start, 0);
   DECLARE l_priority INT DEFAULT 0;
   DECLARE l_priority_cnt BIGINT DEFAULT 0; -- number of requests to launch with l_priority (or 0 to follow default logic)
   DECLARE l_when DATETIME;

   SET l_when = NOW() + INTERVAL p_cleanup_days DAY;

   IF p_cnt IS NULL OR p_cnt < 1 THEN
       SET p_cnt = 1;
   END IF;

   WHILE il < p_cnt DO
        SET il = il + 1;
        IF l_priority_cnt > 0 THEN
            SET l_priority_cnt = l_priority_cnt - 1;
        END IF;

        WHILE l_priority_cnt <= 0 AND p_priority_cnt_csv IS NOT NULL AND LENGTH(p_priority_cnt_csv) DO
            SET @pos = POSITION(',' IN p_priority_cnt_csv);
            IF @pos > 0 THEN
                SET l_priority = TRIM(SUBSTRING(p_priority_cnt_csv FROM 1 FOR @pos - 1));
                SET p_priority_cnt_csv = SUBSTRING(p_priority_cnt_csv FROM @pos + 1);
                SET @pos2 = POSITION(',' IN p_priority_cnt_csv);
                IF @pos2 > 0 THEN
                    SET l_priority_cnt = TRIM(SUBSTRING(p_priority_cnt_csv FROM 1 FOR @pos2 - 1));
                    SET p_priority_cnt_csv = SUBSTRING(p_priority_cnt_csv FROM @pos2 + 1);
                ELSE
                    SET l_priority_cnt = p_priority_cnt_csv;
                    SET p_priority_cnt_csv = '';
                END IF;
            ELSE
                SET l_priority = p_priority_cnt_csv;
                SET p_priority_cnt_csv = '';
            END IF;
        END WHILE;

        -- SELECT CONCAT('INSERT INTO QPReq(svcID, userID, inParallel, priority, subIp, purgeTm) VALUES (', p_svc_id, ', ', p_user_id, ', ', il, ', ',  ABS(l_priority), ', ', p_subip, ', ', l_when, ');') AS sql;
        INSERT INTO QPReq(svcID, userID, inParallel, priority, subIp, purgeTm)
            VALUES (p_svc_id, p_user_id, il, ABS(l_priority), p_subip, l_when);

        SELECT LAST_INSERT_ID() INTO l_req;
        IF l_grp = 0 THEN
            IF p_grp_id IS NOT NULL AND p_grp_id != 0 THEN
                SET l_grp = p_grp_id;
            ELSE
                SET l_grp = l_req;
            END IF;
        END IF;
        INSERT INTO QPGrp(grpID, reqID, jobIDCollect, masterGrpID)
            VALUES (l_grp, l_req, il, l_grp);

        /* If we are launching more requests than priorities were provided, increment priorities logarithmically */
        IF il >= l_currentbase THEN
            SET l_currentbase = l_currentbase * 10;
            IF l_priority_cnt <= 0 AND (p_priority_cnt_csv IS NULL OR LENGTH(p_priority_cnt_csv) = 0) AND l_priority >= 0 THEN
                SET l_priority = l_priority + 1;
            END IF;
         END IF;
   END WHILE;

   SELECT l_grp;
END //
DELIMITER ;
