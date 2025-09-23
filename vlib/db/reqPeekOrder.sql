
DROP PROCEDURE IF EXISTS `reqPeekOrder`;

DELIMITER //

CREATE PROCEDURE `reqPeekOrder`(IN lsvcnm char (64), IN lreq bigint, IN lstat int, IN lact int)
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


  declare l_prior int default 1;
  declare l_id int default 0;
  declare l_queuenum int default 0;
  declare l_running int default 0;

  if lsvcnm != '' then
    select svcID into l_id from QPSvc where name = lsvcnm;
  elseif lsvcnm != 'any' then
    select svcID into l_id from QPReq where reqID = lreq;
  end if;

  select priority into l_prior from QPReq where reqID =lreq;

  if lsvcnm = 'any' then
    select count(reqID) into l_queuenum from QPReq where stat=lstat and act = lact  and priority <= l_prior and reqID<lreq and scheduleGrab <= NOW();
    select count(reqID) into l_running from QPReq where stat=3 and act = lact ;
  else
    select count(reqID) into l_queuenum from QPReq where svcID=l_id and stat=lstat and act = lact  and priority <= l_prior and reqID<lreq and scheduleGrab <= NOW();
    select count(reqID) into l_running from QPReq where svcID=l_id and stat=3 and act = lact  ;
  end if;

  select l_queuenum, l_running;

end //
DELIMITER ;
