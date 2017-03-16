
DROP PROCEDURE IF EXISTS `sp_req_recover`;

DELIMITER //

CREATE PROCEDURE `sp_req_recover`(
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
    DROP TEMPORARY TABLE IF EXISTS svc_maxTrials_restartSec;
    CREATE TEMPORARY TABLE svc_maxTrials_restartSec AS
        SELECT o.objID AS svcID, f1.`value` AS maxTrials, f2.`value` AS restartSec
            FROM UPObjField f2 JOIN UPObjField f1
                    ON ((f1.domainID = f2.domainID OR (f1.domainID IS NULL AND f2.domainID IS NULL)) AND f1.objID = f2.objID)
                JOIN UPObj o ON ((f1.domainID = o.domainID OR (f1.domainID IS NULL AND o.domainID = 0)) AND f1.objID = o.objID)
            WHERE f1.`name` = 'maxTrials' AND f2.`name` = 'restartSec' AND
                /* o.objTypeDomainID = 0 AND */ o.objTypeID = (SELECT type_id FROM UPType WHERE `name` ='qpsvc');

    DROP TEMPORARY TABLE IF EXISTS QP_recover_reqs;
    CREATE TEMPORARY TABLE QP_recover_reqs AS
        SELECT r.reqID FROM QPReq r JOIN svc_maxTrials_restartSec s USING(svcID)
            WHERE r.takenCnt < s.maxTrials AND s.restartSec != 0 AND r.aliveTm != 0 AND
                (UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(r.aliveTm)) > s.restartSec AND
                (r.stat = 3 OR r.stat = 2) AND r.act = 2;

    UPDATE QPReq SET stat = 1
        WHERE reqID IN (SELECT reqID FROM QP_recover_reqs);

    DROP TEMPORARY TABLE IF EXISTS QP_kill_reqs;
    CREATE TEMPORARY TABLE QP_kill_reqs AS
        SELECT r.reqID FROM QPReq r JOIN svc_maxTrials_restartSec s USING(svcID)
            WHERE r.takenCnt >= s.maxTrials AND s.restartSec != 0 AND r.aliveTm != 0 AND
                (UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(r.aliveTm)) > s.restartSec AND
                (r.stat = 3 or r.stat = 2) AND r.act = 2;

    UPDATE QPReq SET stat = 6
        WHERE reqID IN ( SELECT reqID FROM QP_kill_reqs );

    DROP TEMPORARY TABLE QP_recover_reqs;
    DROP TEMPORARY TABLE QP_kill_reqs;
    DROP TEMPORARY TABLE svc_maxTrials_restartSec;
END //
DELIMITER ;
