
DROP PROCEDURE IF EXISTS `grpSubmit_vBES`;

DELIMITER //

CREATE PROCEDURE `grpSubmit_vBES`(
    IN lsvcnm VARCHAR(128),
    IN l_grp INT,
    IN lpriority INT,
    IN lsubip BIGINT,
    IN lNumReqs BIGINT,
    IN lusrid BIGINT
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

 /*
       !!!!!OBSOLETE STORED PROC!!!!!
               USE sp_grp_submit
 */

    DECLARE l_inPar INT DEFAULT 0;
    DECLARE l_id INT;
    DECLARE l_cd INT;
    DECLARE l_req BIGINT;
    DECLARE l_currentbase INT DEFAULT 10;
    DECLARE il INT DEFAULT 0;
    DECLARE l_when DATETIME;

    SELECT svcID FROM QPSvc WHERE `name` = lsvcnm
    INTO l_id;
    SELECT parallelJobs FROM QPSvc WHERE svcID = l_id
    INTO l_inPar;
    SELECT cleanUpDays FROM QPSvc WHERE svcID = l_id
    INTO l_cd;
    SET l_when = NOW() + INTERVAL l_cd DAY;

    IF lNumReqs != 0 THEN
        SET l_inPar = lNumReqs;
    END IF;

    IF l_grp != 0 AND NOT EXISTS(SELECT 1 FROM QPReq where `reqID`=l_grp) THEN
        SET l_grp=0;
    END IF;

    IF l_grp != 0 THEN
        IF EXISTS( SELECT 1 FROM QPGrp WHERE `reqID`=l_grp AND `grpId`=l_grp AND `masterGrpID` is NULL) THEN
            UPDATE QPGrp SET `masterGrpID`=l_grp WHERE `reqID`=l_grp AND `grpId`=l_grp;
        END IF;
        IF NOT EXISTS(SELECT 1 FROM QPGrp WHERE `reqID`= l_grp AND `grpId`= l_grp AND `masterGrpID`= l_grp) THEN
            INSERT QPGrp(grpID, reqID, jobIDCollect, masterGrpID) VALUES (l_grp, l_grp, 1, l_grp);
        END IF;
        SELECT max(jobIDCollect) FROM QPGrp where masterGrpID=l_grp
        INTO il;
        SET l_inPar = l_inPar + il;
    END IF;

    grp_loop: LOOP
        SET il = il + 1;

        INSERT QPReq(svcID, userID, inParallel, priority, subIp, purgeTm)
        VALUES (l_id, lusrid, il, ABS(lpriority), lsubip, l_when);

        SELECT LAST_INSERT_ID() INTO l_req;

        IF l_grp = 0 THEN
           SET l_grp = l_req;
        END IF;

        INSERT QPGrp(grpID, reqID, jobIDCollect, masterGrpID)
        VALUES (l_grp, l_req, il, l_grp);

        IF il >= l_inPar THEN
            LEAVE grp_loop;
        END IF;
        IF lpriority >= 0 THEN
            IF il >= l_currentbase THEN
                SET lpriority = lpriority + 1;
                SET l_currentbase = l_currentbase * 10;
            END IF;
        END IF;
   END LOOP grp_loop;

   SELECT l_grp;
END //
DELIMITER ;
