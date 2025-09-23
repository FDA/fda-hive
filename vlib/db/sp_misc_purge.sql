
DROP PROCEDURE IF EXISTS `sp_misc_purge`;

DELIMITER //

CREATE PROCEDURE `sp_misc_purge`(
    IN p_qpsvc BOOL, -- : are services defined in QPSvc (not as objects)?
    IN p_limit BIGINT UNSIGNED -- : number of rows in each table to modify
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
    DECLARE days_discard BIGINT DEFAULT 122; -- 4 months
    DECLARE p_limit_big BIGINT UNSIGNED;

    IF p_qpsvc IS NULL OR NOT p_qpsvc THEN
        -- find type type id
        SELECT domainID, objID FROM UPObj WHERE objTypeDomainID = domainID AND objTypeID = objID
        INTO @tdid, @toid;

        -- find type named 'qpsvc'
        SELECT o.domainID, o.objID FROM UPObj o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
        WHERE o.objTypeDomainID = @tdid AND o.objTypeID = @toid AND f.name = 'name' AND f.value = 'qpsvc'
        INTO @tdid_qpsvc, @toid_qpsvc;


        SET p_qpsvc = FALSE;
        CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_qpsvc (svcID BIGINT, `name` VARCHAR(128), PRIMARY KEY (svcID), INDEX `name` (`name`));
        TRUNCATE TABLE tempSpMiscPurge_qpsvc;
        INSERT IGNORE INTO tempSpMiscPurge_qpsvc SELECT o.objID AS svcID, f.`value` AS `name`
            FROM UPObj o JOIN UPObjField f ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
            WHERE o.objTypeDomainID = @tdid_qpsvc AND o.objTypeID = @toid_qpsvc AND f.`name` = "name";
    END IF;

    IF p_limit IS NULL OR p_limit <= 0 THEN
        SET p_limit = 1000;
    END IF;

    SET p_limit_big = p_limit * 10;

    -- orphaned data
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_QPReq (reqID BIGINT, PRIMARY KEY (reqID));
    INSERT IGNORE INTO tempSpMiscPurge_QPReq SELECT d.reqID FROM QPData d LEFT JOIN QPReq r USING (reqID) WHERE r.reqID IS NULL LIMIT p_limit;
    SELECT "QPData reqID", reqID FROM tempSpMiscPurge_QPReq;
    DELETE QPData FROM QPData INNER JOIN tempSpMiscPurge_QPReq USING (reqID);
    DROP TEMPORARY TABLE tempSpMiscPurge_QPReq;

    -- orphaned groups
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_QPReq2 (reqID BIGINT, PRIMARY KEY (reqID));
    INSERT IGNORE INTO tempSpMiscPurge_QPReq2
    SELECT r.reqID FROM QPGrp g LEFT JOIN QPReq r ON g.reqID = r.reqID OR g.grpID = r.reqID OR g.masterGrpId = r.reqID
        WHERE r.reqID IS NULL LIMIT p_limit;
    SELECT "QPGrp reqID", reqID FROM tempSpMiscPurge_QPReq2;
    DELETE g FROM QPGrp g INNER JOIN tempSpMiscPurge_QPReq2 r ON g.reqID = r.reqID OR g.grpID = r.reqID OR g.masterGrpId = r.reqID;
    DROP TEMPORARY TABLE tempSpMiscPurge_QPReq2;

    -- jobs of unknown services
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_QPJob (jobID BIGINT, PRIMARY KEY (jobID));
    IF p_qpsvc THEN
        INSERT IGNORE INTO tempSpMiscPurge_QPJob SELECT j.jobID FROM QPJob j LEFT JOIN QPSvc s USING (svcID) WHERE s.svcID IS NULL LIMIT p_limit;
    ELSE
        INSERT IGNORE INTO tempSpMiscPurge_QPJob SELECT j.jobID FROM QPJob j LEFT JOIN tempSpMiscPurge_qpsvc s USING (svcID) WHERE s.svcID IS NULL LIMIT p_limit;
    END IF;
    SELECT "jobID", jobID FROM tempSpMiscPurge_QPJob;
    DELETE QPJob FROM QPJob INNER JOIN tempSpMiscPurge_QPJob USING (jobID);
    DROP TEMPORARY TABLE tempSpMiscPurge_QPJob;

    -- old log entries; larger limit because QPLog grows fast
    SELECT "QPLog old entries", COUNT(*) FROM QPLog WHERE NOW() > (cdate + INTERVAL days_discard DAY) LIMIT p_limit_big;
    DELETE FROM QPLog WHERE NOW() > (cdate + INTERVAL days_discard DAY) LIMIT p_limit_big;

    -- old locks
    SELECT "QPLock old entries", COUNT(*) FROM QPLock WHERE NOW() > purgeTm LIMIT p_limit;
    DELETE FROM QPLock WHERE NOW() > purgeTm LIMIT p_limit;

    -- orphaned parameters
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_QPReq3 (reqID BIGINT, PRIMARY KEY (reqID));
    INSERT IGNORE INTO tempSpMiscPurge_QPReq3 SELECT p.reqID FROM QPReqPar p LEFT JOIN QPReq r USING (reqID) WHERE r.reqID IS NULL LIMIT p_limit;
    SELECT "QPReqPar reqID", reqID FROM tempSpMiscPurge_QPReq3;
    DELETE p FROM QPReqPar p INNER JOIN tempSpMiscPurge_QPReq3 r USING (reqID);
    DROP TEMPORARY TABLE tempSpMiscPurge_QPReq3;

    -- resources of unknown services
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_QPSvc (svcName varchar(256), PRIMARY KEY (svcName));
    IF p_qpsvc THEN
        INSERT IGNORE INTO tempSpMiscPurge_QPSvc SELECT r.svcName FROM QPResource r LEFT JOIN QPSvc s ON r.svcName = s.name WHERE s.name IS NULL LIMIT p_limit;
    ELSE
        INSERT IGNORE INTO tempSpMiscPurge_QPSvc SELECT r.svcName FROM QPResource r LEFT JOIN tempSpMiscPurge_qpsvc s ON r.svcName = s.name WHERE s.name IS NULL LIMIT p_limit;
    END IF;
    SELECT svcName FROM tempSpMiscPurge_QPSvc;
    DELETE QPResource FROM QPResource INNER JOIN tempSpMiscPurge_QPSvc USING (svcName);
    DROP TEMPORARY TABLE tempSpMiscPurge_QPSvc;

    -- empty object fields
    SELECT "UPObjField empty entries", COUNT(*) FROM UPObjField
        WHERE (`value` IS NULL OR LENGTH(`value`) = 0) AND `blob_value` IS NULL LIMIT p_limit;
    DELETE FROM UPObjField
        WHERE (`value` IS NULL OR LENGTH(`value`) = 0) AND `blob_value` IS NULL LIMIT p_limit;

    -- orphaned object fields
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpMiscPurge_UPObj (domainID BIGINT UNSIGNED, objID BIGINT UNSIGNED, UNIQUE KEY uniq_idx (domainID, objID));
    INSERT IGNORE INTO tempSpMiscPurge_UPObj SELECT f.domainID, f.objID FROM UPObjField f LEFT JOIN UPObj o
        ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID
        WHERE o.objID IS NULL AND o.domainID IS NULL LIMIT p_limit;
    SELECT "UPObjField objID", objID FROM tempSpMiscPurge_UPObj;
    DELETE f FROM UPObjField f INNER JOIN tempSpMiscPurge_UPObj o
        ON (f.domainID = o.domainID OR (f.domainID IS NULL AND o.domainID = 0)) AND o.objID = f.objID;
    DROP TEMPORARY TABLE tempSpMiscPurge_UPObj;

    IF NOT p_qpsvc THEN
        DROP TEMPORARY TABLE IF EXISTS tempSpMiscPurge_qpsvc;
    END IF;
END //
DELIMITER ;
