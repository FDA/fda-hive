
DROP PROCEDURE IF EXISTS `dataIns`;

DELIMITER //

CREATE PROCEDURE `dataIns`(IN lreq bigint, IN lname varchar(63), IN lblob longblob  )
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
    IF lreq IS NOT NULL AND lreq != 0 AND lname IS NOT NULL AND LENGTH(lname) > 0 THEN
        SET @reqId = NULL;
        SELECT reqID FROM QPData WHERE reqID = lreq AND dataName = lname
        INTO @reqId;

        IF @reqId IS NOT NULL THEN
            DELETE FROM QPData WHERE reqID = lreq AND dataName = lname;
        END IF;
        INSERT INTO QPData (reqID, dataName, dataBlob) VALUES (lreq, lname, lblob);
        SET @rc = ROW_COUNT();
        IF @rc > 0 THEN
            SELECT 1;
        END IF;
    END IF;
END //
DELIMITER ;
