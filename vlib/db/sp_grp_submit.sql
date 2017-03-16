
DROP PROCEDURE IF EXISTS `sp_grp_submit`;

DELIMITER //

CREATE PROCEDURE `sp_grp_submit`(
    IN l_id INT,
    IN l_cd INT,
    IN l_inPar INT,
    IN linB INT,
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
   DECLARE l_req BIGINT;
   DECLARE l_grp BIGINT DEFAULT 0;
   DECLARE l_currentbase INT DEFAULT 10;
   DECLARE il INT DEFAULT linB;
   DECLARE l_when DATETIME;

   IF lNumReqs != 0 THEN
         SET l_inPar = lNumReqs;
   END IF;
   SET l_when = NOW() + INTERVAL l_cd DAY;

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

        IF il >= l_inPar OR linB != 0 THEN
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
