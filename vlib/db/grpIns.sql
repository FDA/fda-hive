
DROP PROCEDURE IF EXISTS `grpIns`;

DELIMITER //

CREATE PROCEDURE `grpIns`(IN lgrpID bigint, IN lreqID bigint, IN ljobIDCollect int )
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

   declare lPairIn int;
   declare lGrpIn int;

   if lgrpID = 0 then
     set lgrpID = lreqID;
   end if;

   select reqID from QPGrp where grpID = lgrpID and reqID = lreqID into lPairIn;
   select grpID from QPGrp where grpID=lgrpID limit 1 into lGrpIn;

   if lPairIn is null then

      if lGrpIn is null then
        insert QPGrp(grpID,reqID, jobIDCollect) values (lgrpID,lgrpID, 0);
      end if ;

      if lgrpID != lreqID then
        insert QPGrp(grpID,reqID, jobIDCollect) values (lgrpID,lreqID, ljobIDCollect);
      end if ;



   end if;
   select lgrpID;
end //
DELIMITER ;
