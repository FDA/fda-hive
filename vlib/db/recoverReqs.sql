
DROP PROCEDURE IF EXISTS `recoverReqs`;

DELIMITER //

CREATE PROCEDURE `recoverReqs`( )
    MODIFIES SQL DATA
begin
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

    drop temporary table if exists QP_recover_reqs;
    create temporary table QP_recover_reqs as
        select QPReq.reqID from QPReq, QPSvc
            where QPReq.svcID = QPSvc.svcID and
                  QPReq.takenCnt < QPSvc.maxTrials and
                  QPSvc.restartSec != 0 and
                  QPReq.aliveTm != 0 and
                  ( UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(QPReq.aliveTm) ) > QPSvc.restartSec and
                  ( QPReq.stat = 3 or QPReq.stat = 2 ) and
                  QPReq.act = 2;
    update QPReq set stat = 1
        where reqID in (select reqID from QP_recover_reqs);

    drop temporary table if exists QP_kill_reqs;
    create temporary table QP_kill_reqs as
        select QPReq.reqID from QPReq, QPSvc
            where QPReq.svcID = QPSvc.svcID and
                  QPReq.takenCnt >= QPSvc.maxTrials and
                  QPSvc.restartSec != 0 and
                  QPReq.aliveTm != 0 and
                  ( UNIX_TIMESTAMP(NOW()) - UNIX_TIMESTAMP(QPReq.aliveTm) ) > QPSvc.restartSec and
                  (QPReq.stat = 3 or QPReq.stat = 2 ) and
                  QPReq.act = 2;
    update QPReq set stat = 6
        where reqID in ( select reqID from QP_kill_reqs );

    drop temporary table QP_recover_reqs;
    drop temporary table QP_kill_reqs;

end //
DELIMITER ;
