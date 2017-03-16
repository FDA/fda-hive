
DROP PROCEDURE IF EXISTS `sp_svc_purge_v2`;

DELIMITER //

CREATE PROCEDURE `sp_svc_purge_v2`(
    IN p_servicename VARCHAR(128), -- : NULL or '*' to purge all services
    IN p_limit BIGINT UNSIGNED,
    IN p_mode VARCHAR(16), -- : 'find' or 'delete' or 'cleanup'
    IN p_qpsvc BOOL -- : are services defined in QPSvc or as objects?
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
    DECLARE l_cleanup_days INT DEFAULT 0;
    DECLARE l_svcid BIGINT DEFAULT 0;

    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpSvcPurge_reqs (reqID BIGINT, PRIMARY KEY (reqID));
    CREATE TEMPORARY TABLE IF NOT EXISTS tempSpSvcPurge_jobs (jobID BIGINT, PRIMARY KEY (jobID));
    IF p_qpsvc IS NULL OR NOT p_qpsvc THEN
        SET p_qpsvc    = FALSE;
        CREATE TEMPORARY TABLE IF NOT EXISTS tempSpSvcPurge_qpsvc (
            svcID BIGINT,
            `name` VARCHAR(128),
            cleanUpDays INT,
            PRIMARY KEY (svcID),
            INDEX `name` (`name`)
        );
        TRUNCATE TABLE tempSpSvcPurge_qpsvc;
        INSERT IGNORE INTO tempSpSvcPurge_qpsvc
        SELECT svcID, MAX(`name`) AS `name`, MAX(cleanUpDays) AS cleanUpDays FROM
        (
            SELECT o.objID AS svcID,
                   IF(f.`name` = 'name', f.`value`, NULL) AS `name`,
                   IF(f.`name` = 'cleanUpDays', f.`value`, NULL) AS `cleanUpDays`
                FROM UPObj o JOIN UPType t ON (o.objTypeDomainID IS NOT NULL AND o.objTypeID = t.type_id)
                    JOIN UPObjField f ON (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL)) AND o.objID = f.objID
                WHERE t.`name` = 'qpsvc' AND f.`name` IN ('name', 'cleanUpDays')
        ) x GROUP BY svcID;
    END IF;

    IF p_mode = 'find' THEN
        -- Only purge requests whose entire group doesn't contain other living requests
        CREATE TEMPORARY TABLE IF NOT EXISTS tempSpSvcPurge_alive_grp (
            grpID BIGINT,
            PRIMARY KEY (grpID)
        );
        TRUNCATE TABLE tempSpSvcPurge_alive_grp;
        INSERT IGNORE INTO tempSpSvcPurge_alive_grp
            SELECT g.grpID
            FROM QPReq r INNER JOIN QPGrp g ON r.reqID = g.reqID OR r.reqID = g.masterGrpID
            WHERE r.stat < 5 AND NOW() <= (r.cdate + INTERVAL days_discard DAY);

        -- Calculating tempSpSvcPurge_req_alive_grp via tempSpSvcPurge_alive_grp is much faster
        -- than using a QPreq - GPGrp - QPGrp 3-level join
        CREATE TEMPORARY TABLE IF NOT EXISTS tempSpSvcPurge_req_alive_grp (
            reqID BIGINT,
            PRIMARY KEY (reqID)
        );
        TRUNCATE TABLE tempSpSvcPurge_req_alive_grp;
        INSERT IGNORE INTO tempSpSvcPurge_req_alive_grp
            SELECT g.reqID
            FROM tempSpSvcPurge_alive_grp ag INNER JOIN QPGrp g USING (grpID);

        TRUNCATE TABLE tempSpSvcPurge_reqs;
        TRUNCATE TABLE tempSpSvcPurge_jobs;
        IF p_servicename IS NULL OR p_servicename = '*' THEN
            INSERT IGNORE INTO tempSpSvcPurge_reqs
                SELECT r.reqID
                FROM QPReq r LEFT JOIN tempSpSvcPurge_req_alive_grp a ON r.reqID = a.reqID
                WHERE a.reqID IS NULL AND ((r.stat >= 5 AND NOW() > r.purgeTm) OR NOW() > r.cdate + INTERVAL days_discard DAY)
                LIMIT p_limit;

            IF p_qpsvc THEN
                INSERT IGNORE INTO tempSpSvcPurge_jobs
                    SELECT j.jobID
                        FROM QPJob j LEFT JOIN QPSvc s USING (svcID) LEFT JOIN tempSpSvcPurge_reqs t USING (reqID)
                        WHERE s.svcID IS NULL
                            OR NOW() > j.aliveTm + INTERVAL s.cleanUpDays DAY
                            OR NOW() > j.cdate + INTERVAL days_discard DAY
                            OR t.reqID IS NOT NULL
                        LIMIT p_limit;
            ELSE
                INSERT IGNORE INTO tempSpSvcPurge_jobs
                    SELECT j.jobID
                        FROM QPJob j LEFT JOIN tempSpSvcPurge_qpsvc s USING (svcID) LEFT JOIN tempSpSvcPurge_reqs t USING (reqID)
                        WHERE s.svcID IS NULL
                            OR NOW() > j.aliveTm + INTERVAL s.cleanUpDays DAY
                            OR NOW() > j.cdate + INTERVAL days_discard DAY
                            OR t.reqID IS NOT NULL
                        LIMIT p_limit;
            END IF;
        ELSE
            IF p_qpsvc THEN
                SELECT svcID, cleanUpDays FROM QPSvc WHERE name = p_servicename
                    INTO l_svcid, l_cleanup_days;
            ELSE
                SELECT svcID, cleanUpDays FROM tempSpSvcPurge_qpsvc WHERE name = p_servicename
                    INTO l_svcid, l_cleanup_days;
            END IF;

            INSERT IGNORE INTO tempSpSvcPurge_reqs
                SELECT r.reqID
                FROM QPReq r LEFT JOIN tempSpSvcPurge_req_alive_grp a ON r.reqID = a.reqID
                WHERE r.svcID = l_svcid AND a.reqID IS NULL AND ((r.stat >= 5 AND NOW() > r.purgeTm) OR NOW() > r.cdate + INTERVAL days_discard DAY)
                LIMIT p_limit;

            INSERT IGNORE INTO tempSpSvcPurge_jobs
                SELECT j.jobID
                    FROM QPJob j LEFT JOIN tempSpSvcPurge_reqs t USING (reqID)
                    WHERE j.svcID = l_svcid AND (NOW() > j.aliveTm + INTERVAL sl_cleanup_days DAY
                        OR NOW() > j.cdate + INTERVAL days_discard DAY
                        OR t.reqID IS NOT NULL)
                    LIMIT p_limit;
        END IF;
        SELECT reqID, userID FROM tempSpSvcPurge_reqs t LEFT JOIN QPReq r USING (reqID);
    ELSEIF p_mode = 'delete' THEN
        -- we need only top objID to have auto_incerement working for local object IDs
        SET @idMax = NULL;
        SELECT MAX(objID) FROM UPObjMax INTO @idMax;
        IF @idMax IS NOT NULL AND @idMax > 0 THEN
            DELETE FROM UPObjMax WHERE objID < @idMax AND cdate < NOW() - INTERVAL 1 DAY;
        END IF;

        DELETE QPData FROM QPData INNER JOIN tempSpSvcPurge_reqs USING (reqID);
        DELETE g FROM QPGrp g INNER JOIN tempSpSvcPurge_reqs t ON g.reqID = t.reqID OR g.grpID = t.reqID OR g.masterGrpID = t.reqID;
        DELETE QPJob FROM QPJob INNER JOIN tempSpSvcPurge_jobs USING (jobID);
        DELETE QPReqPar FROM QPReqPar INNER JOIN tempSpSvcPurge_reqs USING (reqID);
        DELETE QPReq FROM QPReq INNER JOIN tempSpSvcPurge_reqs USING (reqID);
        DELETE QPLog FROM QPLog INNER JOIN tempSpSvcPurge_reqs USING (reqID);
        DELETE QPLog FROM QPLog INNER JOIN tempSpSvcPurge_jobs USING (jobID);
        DELETE QPLock FROM QPLock INNER JOIN tempSpSvcPurge_reqs USING(reqID);
    END IF;

    IF p_mode = 'delete' OR p_mode = 'cleanup' THEN
        DROP TEMPORARY TABLE IF EXISTS tempSpSvcPurge_alive_grp;
        DROP TEMPORARY TABLE IF EXISTS tempSpSvcPurge_req_alive_grp;
        DROP TEMPORARY TABLE IF EXISTS tempSpSvcPurge_reqs;
        DROP TEMPORARY TABLE IF EXISTS tempSpSvcPurge_jobs;
        DROP TEMPORARY TABLE IF EXISTS tempSpSvcPurge_qpsvc;
    END IF;
END //
DELIMITER ;
