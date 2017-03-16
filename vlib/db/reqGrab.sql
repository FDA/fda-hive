
DROP PROCEDURE IF EXISTS `reqGrab`;

DELIMITER //

CREATE PROCEDURE `reqGrab`(
    IN lsvcnm VARCHAR(128),
    IN ljob BIGINT,
    IN lstat INT,
    IN lact INT,
    IN linParallel INT
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

    DECLARE l_id INT DEFAULT 1;
    DECLARE l_grabRand BIGINT DEFAULT 0;
    DECLARE l_reqID BIGINT DEFAULT 0;
    DECLARE l_minpriority INT DEFAULT 0;
    SELECT svcID INTO l_id FROM QPSvc WHERE `name` = lsvcnm;

    SET l_grabRand = FLOOR(1 + RAND() * 4000000000);

    IF linParallel = 0 THEN
        SELECT min(priority) FROM QPReq WHERE svcID = l_id AND stat = lstat AND act = lact AND scheduleGrab < NOW()
        INTO l_minpriority;

        UPDATE QPReq SET grabRand = l_grabRand, jobId = ljob, stat = 2, takenCnt = takenCnt + 1, takenTm = NOW(), aliveTm = NOW()
            WHERE svcID = l_id AND stat = lstat AND act = lact AND scheduleGrab < NOW() AND priority = l_minpriority LIMIT 1;
    ELSE
        UPDATE QPReq SET grabRand = l_grabRand, jobId = ljob, stat = 2, takenCnt = takenCnt + 1, takenTm = NOW(), aliveTm = NOW()
            WHERE svcID = l_id AND stat = lstat AND act = lact AND scheduleGrab < NOW() AND inParallel = linParallel LIMIT 1;
    END IF;

    SELECT reqID FROM QPReq
        WHERE grabRand = l_grabRand AND svcID = l_id AND act = lact AND stat = 2
    INTO l_reqID;

    IF l_reqID > 0 THEN
        UPDATE QPJob SET aliveTm = NOW(), reqID = l_reqID, cntGrabbed = cntGrabbed + 1 WHERE jobID = ljob;
    ELSE
        UPDATE QPJob SET aliveTm = NOW() WHERE jobID = ljob;
    END IF;

  SELECT l_reqID;

END //
DELIMITER ;
