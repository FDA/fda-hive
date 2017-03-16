
DROP PROCEDURE IF EXISTS `sp_obj_by_time`;

DELIMITER //

CREATE PROCEDURE `sp_obj_by_time`(
    IN p_user_id BIGINT UNSIGNED, -- : request user id
    IN p_primary_group_id BIGINT UNSIGNED, -- : corresponding to p_user_id
    IN p_since_time BIGINT, -- : unix timestamp; <= 0 means since beginning of time
    IN p_to_time BIGINT, -- : unix timestamp
    IN p_field CHAR(16) -- : 'created' or 'modified' or 'completed'
)
    READS SQL DATA
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
    IF p_field != 'created' AND p_field != 'completed' AND p_field != 'modified' THEN
        SET p_field = 'created';
    END IF;
    IF p_field = 'created' THEN
        SELECT o.domainID, o.objID, NULL AS ionID, f.`value` AS created
            FROM UPObj o JOIN UPObjField f
            ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE creatorID = p_primary_group_id AND f.name = 'created' AND f.`value` > p_since_time AND f.`value` <= p_to_time
            GROUP BY o.domainID, o.objID ORDER BY o.domainID, o.objID;
    ELSEIF p_field = 'completed' THEN
        CREATE TEMPORARY TABLE IF NOT EXISTS tempSpObjByTime_Completed (
            domainID BIGINT UNSIGNED,
            objID BIGINT UNSIGNED,
            completed BIGINT,
            UNIQUE(domainID, objID)
        );
        INSERT IGNORE INTO tempSpObjByTime_Completed
            SELECT o.domainID, o.objID, f.`value` AS completed
            FROM UPObj o JOIN UPObjField f
            ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE creatorID = p_primary_group_id AND f.name = 'completed' AND f.`value` > p_since_time AND f.`value` <= p_to_time
            GROUP BY o.domainID, o.objID ORDER BY o.domainID, o.objID;
        SELECT o.domainID, o.objID, NULL AS ionID, f.`value` AS 'started', o.completed
            FROM tempSpObjByTime_Completed o JOIN UPObjField f
            ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE f.name = 'started' AND f.`value` > 0;
        DROP TEMPORARY TABLE tempSpObjByTime_Completed;
    ELSE
        SELECT o.domainID, o.objID, NULL AS ionID, f.`value` AS modified
            FROM UPObj o JOIN UPObjField f
            ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE creatorID = p_primary_group_id AND (f.name = 'created' or f.name = 'modified') AND f.`value` > p_since_time AND f.`value` <= p_to_time
            GROUP BY o.domainID, o.objID
        UNION
        SELECT NULL, p.val AS objID, NULL AS ionID, UNIX_TIMESTAMP(GREATEST(r.cdate, r.takenTm, r.aliveTm, r.doneTm)) AS modified
            FROM QPReq r JOIN QPReqPar p USING (reqID)
            WHERE r.userID = p_user_id AND p.type = 1 /* eQPReqPar_Objects */ and p.val > 0 /* objID */
                AND UNIX_TIMESTAMP(GREATEST(r.cdate, r.takenTm, r.aliveTm, r.doneTm)) > p_since_time AND UNIX_TIMESTAMP(GREATEST(r.cdate, r.takenTm, r.aliveTm, r.doneTm)) <= p_to_time
            GROUP BY p.val /* objID */ ;
    END IF;
END //
DELIMITER ;
