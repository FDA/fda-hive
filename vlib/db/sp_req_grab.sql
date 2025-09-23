
DROP PROCEDURE IF EXISTS `sp_req_grab`;

DELIMITER //

CREATE PROCEDURE `sp_req_grab`(IN l_id int, IN ljob bigint, IN lstat int, IN lact int, IN linParallel int)
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

  declare l_grabRand bigint ;
  declare l_reqID bigint default 0;
  declare l_minpriority int default 0;

  set l_grabRand =  (FLOOR( 1 + RAND( ) *4000000000 ));
  if linParallel = 0 then
    select IFNULL(min(priority), 0) from QPReq where svcID = l_id and stat=lstat and act = lact
    into l_minpriority;
    update QPReq set grabRand=l_grabRand, jobId = ljob, stat = 2, takenCnt = takenCnt+1, takenTm = NOW(), aliveTm=NOW()
        where svcID = l_id and stat=lstat and act = lact and priority=l_minpriority
        limit 1;
  else
    update QPReq set grabRand=l_grabRand, jobId = ljob, stat = 2, takenCnt = takenCnt+1, takenTm = NOW(), aliveTm=NOW()
        where svcID = l_id and stat=lstat and act = lact and inParallel=linParallel
        limit 1;
  end if;

  select reqID into l_reqID from QPReq where grabRand=l_grabRand;
  if l_reqID > 0 then
    update QPJob set aliveTm=NOW() , reqID=l_reqID , cntGrabbed=cntGrabbed + 1 where jobID = ljob ;
  else
    update QPJob set aliveTm=NOW() where jobID = ljob;
  end if;

  select l_reqID ;
end //
DELIMITER ;
