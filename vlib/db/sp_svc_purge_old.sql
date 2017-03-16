
DROP PROCEDURE IF EXISTS `sp_svc_purge_old`;

DELIMITER //

CREATE PROCEDURE `sp_svc_purge_old`(
    IN p_svc_id BIGINT UNSIGNED,
    IN p_svc_cleanUpDays INT,
    IN p_qm_cleanUpDays INT,
    IN p_limit BIGINT UNSIGNED,
    IN p_no_delete BOOLEAN
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
        DECLARE cleanUpDays INT DEFAULT 0;
        DECLARE daysDiscard BIGINT DEFAULT 122; -- 4 months


        SET p_qm_cleanUpDays = IF(p_qm_cleanUpDays > 0, p_qm_cleanUpDays, 90);
        SET cleanUpDays = IF(p_svc_cleanUpDays > 0, p_svc_cleanUpDays, p_qm_cleanUpDays);

        DROP TEMPORARY TABLE IF EXISTS tempOldReq;
        DROP TEMPORARY TABLE IF EXISTS tempOldJob;
        DROP TEMPORARY TABLE IF EXISTS tempOldObj;
        DROP TEMPORARY TABLE IF EXISTS tempSvcObj;
        CREATE TEMPORARY TABLE tempOldReq (reqID BIGINT, PRIMARY KEY (reqID));
        CREATE TEMPORARY TABLE tempOldJob (jobID BIGINT, PRIMARY KEY (jobID));
        CREATE TEMPORARY TABLE tempOldObj (domainID BIGINT UNSIGNED, objID BIGINT UNSIGNED, UNIQUE (domainID, objID));
        CREATE TEMPORARY TABLE tempSvcObj (svcID BIGINT UNSIGNED);

        CREATE TEMPORARY TABLE tempOldReqHold (reqID BIGINT, PRIMARY KEY (reqID));
        CREATE TEMPORARY TABLE tempOldGrpHold (grpID BIGINT, PRIMARY KEY (grpID));

        -- exclude groups where grpID is done (>= 5) but children reqIDs are not yet
        INSERT IGNORE INTO tempOldReqHold
        SELECT reqid FROM QPReq WHERE stat <= 4;

        INSERT IGNORE INTO tempOldGrpHold
        SELECT DISTINCT grpID FROM QPGrp JOIN tempOldReqHold USING (reqID);
        INSERT IGNORE INTO tempOldGrpHold
        SELECT DISTINCT masterGrpID FROM QPGrp JOIN tempOldReqHold USING (reqID);

        INSERT IGNORE INTO tempOldReqHold
        SELECT r.reqID FROM QPReq r JOIN QPGrp g USING (reqID) JOIN tempOldGrpHold t USING (grpID);

        INSERT INTO tempSvcObj SELECT objID FROM UPObj WHERE objTypeID = (SELECT type_id FROM UPType WHERE `name` = 'qpsvc');

        IF p_svc_id != 0 THEN

            INSERT IGNORE INTO tempOldReq
            SELECT r.reqID FROM QPReq r LEFT JOIN tempOldReqHold t USING(reqID) WHERE t.reqID IS NULL AND NOW() > r.purgeTm AND r.stat >= 5 AND r.svcID = p_svc_id LIMIT p_limit;
            SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
            INSERT IGNORE INTO tempOldJob SELECT jobID FROM QPJob WHERE NOW() > actTm + INTERVAL cleanUpDays DAY AND svcID = p_svc_id LIMIT p_limit;

        ELSE

            INSERT IGNORE INTO tempOldReq
            SELECT r.reqID FROM QPReq r LEFT JOIN tempOldReqHold t USING(reqID) WHERE t.reqID IS NULL AND NOW() > r.purgeTm AND r.stat >= 5 limit p_limit;
            SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
            INSERT IGNORE INTO tempOldJob SELECT QPJob.jobID FROM QPJob JOIN (SELECT DISTINCT objID AS svcID, `value` AS cleanUpDays FROM UPObjField F
                            WHERE `name` = 'cleanUpDays' AND objID IN (SELECT svcID FROM tempSvcObj)) S
                USING(svcID) WHERE NOW() > QPJob.aliveTm + interval IF(S.cleanUpDays > 0, S.cleanUpDays, p_qm_cleanUpDays) DAY LIMIT p_limit;
        END IF;

        DROP TEMPORARY TABLE IF EXISTS tempOldReqHold;
        DROP TEMPORARY TABLE IF EXISTS tempOldGrpHold;

        -- delete any very old request even if it is running
        INSERT IGNORE INTO tempOldReq SELECT reqID FROM QPReq WHERE NOW() > (cdate + INTERVAL daysDiscard DAY) LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        INSERT IGNORE INTO tempOldJob SELECT jobID FROM QPJob WHERE NOW() > (cdate + INTERVAL daysDiscard DAY) LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        -- overall deletion of trash/orphan records
        INSERT IGNORE INTO tempOldReq SELECT DISTINCT d.reqID FROM QPData d LEFT JOIN QPReq r USING(reqID) WHERE r.reqID IS NULL LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        INSERT IGNORE INTO tempOldReq SELECT DISTINCT g.grpID FROM QPGrp g LEFT JOIN QPReq r ON r.reqID = g.grpID WHERE r.reqID IS NULL LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        INSERT IGNORE INTO tempOldReq SELECT DISTINCT g.reqID FROM QPGrp g LEFT JOIN QPReq r USING(reqID) WHERE r.reqID IS NULL LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        INSERT IGNORE INTO tempOldReq SELECT DISTINCT p.reqID FROM QPReqPar p LEFT JOIN QPReq r USING(reqID) WHERE r.reqID IS NULL LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);
        INSERT IGNORE INTO tempOldJob SELECT DISTINCT j.jobID FROM QPJob j LEFT JOIN tempSvcObj s USING(svcID) WHERE s.svcID IS NULL LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);

        -- orphaned object fields
        INSERT IGNORE INTO tempOldObj
        SELECT IFNULL(f.domainID, 0), f.objID
            FROM UPObjField f LEFT JOIN UPObj o ON (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL)) AND o.objID = f.objID
            WHERE o.objID IS NULL
            LIMIT p_limit;
        SET p_limit = p_limit - IF(ROW_COUNT() > 0, ROW_COUNT(), 0);

        IF p_no_delete = FALSE THEN
            -- we need only top objID to have auto_incerement working for local object IDs
            SET @idMax = NULL;
            SELECT MAX(objID) FROM UPObjMax INTO @idMax;
            IF @idMax IS NOT NULL AND @idMax > 0 THEN
                DELETE FROM UPObjMax WHERE objID < @idMax AND cdate < NOW() - INTERVAL 1 DAY;
            END IF;

            -- drop empty records
            DELETE FROM UPObjField WHERE (`value` IS NULL OR LENGTH(`value`) = 0) AND `blob_value` IS NULL LIMIT p_limit;
            -- here we are missing deletion of props with invalid names
            -- it is difficult to do it using SQL because of type inheritance

            -- tmp: drop orphan fields from types
            DELETE FROM UPTypeField WHERE type_id NOT IN (SELECT type_id FROM UPType);
            -- clean resource of unknown services
            DELETE FROM QPResource
                WHERE svcName NOT IN (
                    SELECT `value` FROM UPObjField WHERE /* domainID IS NULL AND  */ objID IN (SELECT svcID FROM tempSvcObj) AND `name` = 'name'
                );

            SET @qr = 0;
            SELECT COUNT(*) FROM tempOldReq INTO @qr;
            IF @qr > 0 THEN
                DELETE QPData FROM QPData LEFT JOIN tempOldReq t USING(reqID) WHERE t.reqID IS NOT NULL;
                DELETE QPGrp FROM QPGrp LEFT JOIN tempOldReq t USING(reqID) WHERE t.reqID IS NOT NULL;
                DELETE QPGrp FROM QPGrp LEFT JOIN tempOldReq t ON grpID = t.reqID WHERE t.reqID IS NOT NULL;
                DELETE QPReqPar FROM QPReqPar LEFT JOIN tempOldReq t USING(reqID) WHERE t.reqID IS NOT NULL;
                DELETE QPReq FROM QPReq LEFT JOIN tempOldReq t USING(reqID) WHERE t.reqID IS NOT NULL;
                DELETE QPLog FROM QPLog LEFT JOIN tempOldReq r USING(reqID) WHERE QPLog.reqID IS NOT NULL AND r.reqID IS NOT NULL;
                DELETE QPLock FROM QPLock LEFT JOIN tempOldReq t USING(reqID) WHERE t.reqID IS NOT NULL;
            END IF;

            SET @qj = 0;
            SELECT COUNT(*) FROM tempOldJob INTO @qj;
            IF @qj > 0 THEN
                DELETE QPJob FROM QPJob LEFT JOIN tempOldJob t USING(jobID) WHERE t.jobID IS NOT NULL;
                DELETE QPLog FROM QPLog LEFT JOIN tempOldJob j USING(jobID) WHERE QPLog.jobID IS NOT NULL AND j.jobID IS NOT NULL;
            END IF;

            SET @qo = 0;
            SELECT COUNT(*) FROM tempOldObj INTO @qo;
            IF @qo > 0 THEN
                DELETE UPObjField FROM UPObjField LEFT JOIN tempOldObj t ON
                    (t.domainID = UPObjField.domainID OR (t.domainID = 0 AND UPObjField.domainID IS NULL)) AND t.objID = UPObjField.objID
                WHERE t.objID IS NOT NULL;
            END IF;

            DELETE FROM QPLog WHERE NOW() > (cdate + INTERVAL daysDiscard DAY);
            DELETE FROM QPLock WHERE NOW() > purgeTm LIMIT p_limit;
        END IF;

        DROP TEMPORARY TABLE IF EXISTS tempOldJob;
        DROP TEMPORARY TABLE IF EXISTS tempOldObj;
        DROP TEMPORARY TABLE IF EXISTS tempSvcObj;

        SELECT reqID FROM tempOldReq;

        DROP TEMPORARY TABLE IF EXISTS tempOldReq;
END //
DELIMITER ;
