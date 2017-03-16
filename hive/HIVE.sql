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

-- this script cannot be executed standalone, only as a part of setup!!
DROP TEMPORARY TABLE IF EXISTS tmp_QPCfg_vars; 
CREATE TEMPORARY TABLE tmp_QPCfg_vars (nm VARCHAR(255), emptyok BOOLEAN, val MEDIUMTEXT);

SET @user_expires_days=IFNULL(@user_expires_days, '');
SET @password_expires_days=IFNULL(@password_expires_days, '');
SET @password_keep_old_qty=IFNULL(@password_keep_old_qty, '');
SET @smtp_server=IFNULL(@smtp_server, '');
SET @email_address=IFNULL(@email_address, '');

INSERT INTO tmp_QPCfg_vars VALUES
     ('@head_hosts', FALSE, @head_hosts)
    ,('@site_hosts', FALSE, @site_hosts)
    ,('@bin_dir', FALSE, @bin_dir)
    ,('@local_scratch_dir', FALSE, @local_scratch_dir)
    ,('@global_scratch_dir', FALSE, @global_scratch_dir)
    ,('@obj_storage_dirs', FALSE, @obj_storage_dirs)
    ,('@internalWWW', FALSE, @internalWWW)
    ,('@user_expires_days', TRUE, @user_expires_days)
    ,('@password_expires_days', TRUE, @password_expires_days)
    ,('@password_keep_old_qty', TRUE, @password_keep_old_qty)
    ,('@smtp_server', TRUE, @smtp_server)
    ,('@email_address', TRUE, @email_address)
    ,('@encoder_hex', FALSE, @encoder_hex)
    ,('@session_bin', FALSE, @session_bin)
;

SELECT nm AS VARIABLE, IFNULL(val, '<== ERROR: missing value!!!') AS `VALUE` FROM tmp_QPCfg_vars
WHERE val IS NULL OR (LENGTH(val) = 0 AND NOT emptyok);

SELECT CONCAT('KILL CONNECTION ', connection_id())
FROM tmp_QPCfg_vars WHERE (val IS NULL OR LENGTH(val) = 0) AND NOT emptyok LIMIT 1
INTO @stmt;
SET @stmt = IFNULL(@stmt, 'SELECT \'Configuration setup...\' AS Message');
PREPARE stmt FROM @stmt;
EXECUTE stmt;

DROP PROCEDURE IF EXISTS `tmp_QPCfg_set`;

DELIMITER //

CREATE PROCEDURE `tmp_QPCfg_set`(
    IN p_par VARCHAR(256),
    IN p_val VARCHAR(4096),
    IN p_cleanUpDays INT,
    IN p_nameMasks VARCHAR(4096),
    IN p_overwrite BOOLEAN
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
    IF p_par IS NOT NULL AND LENGTH(p_par) > 0 THEN
        SET @p_exist = NULL;
        SELECT par FROM QPCfg where par = p_par
        INTO @p_exist;
        IF @p_exist IS NULL OR p_overwrite THEN
            DELETE FROM QPCfg where par = TRIM(p_par);
            INSERT INTO QPCfg (par, val, cleanUpDays, nameMasks) VALUES (TRIM(p_par), TRIM(p_val), p_cleanUpDays, p_nameMasks);
        ELSE
            SELECT CONCAT('Parameter ', p_par, ' already exists, skipped') AS message;
        END IF;
    ELSE
        SET @mt = CONCAT('Parameter name (p_par) is NULL or empty');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
END //

DELIMITER ;

CALL tmp_QPCfg_set('qm.genericLauncherFolder', CONCAT(@global_scratch_dir, 'launcher/'), 30, NULL, TRUE); -- TRUE here to update everywhere
CALL tmp_QPCfg_set('qm.tempDirectory', @local_scratch_dir, 90, NULL, FALSE);
CALL tmp_QPCfg_set('qm.resourceRoot', @bin_dir, NULL, NULL, TRUE); -- TRUE here to update everywhere
CALL tmp_QPCfg_set('qm.largeDataRepository', CONCAT(@global_scratch_dir, 'data/'), 180, '/@reqID@-/', TRUE); -- TRUE here to update everywhere
CALL tmp_QPCfg_set('qm.subNetworkHeadList', @head_hosts, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.domains', '/Pride1/Pride2/', NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.domains_Pride1', @site_hosts, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.domains_Pride2', '', NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.largeDataReposSize', 32 * 1024, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.statisticsTimeFrequencySecs', 15 * 60, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.statisticsTimeUnit', 3600, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.baseSvcUDPPort', IF(IFNULL(@baseSvcUDPPort, 0) <= 0, 13667, @baseSvcUDPPort), NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.ObjectExpireDays', 30, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.purgeObjectLimit', 1000, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.PurgeFrequency', 300, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.purgeReqLimit', 1000, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.logStdout', 'off', NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.dbLogMinLevel', 4, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.encoder', @encoder_hex, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('qm.session', @session_bin, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('user.rootStoreManager', @obj_storage_dirs, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('user.storageMinKeepFree', 200 * 1024 * 1024 * 1024, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('user.download', CONCAT(@global_scratch_dir, 'download/'), 30, '/@reqID@_/', FALSE);
CALL tmp_QPCfg_set('user.accountExpireDays', @user_expires_days, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('user.pswdExpireDays', @password_expires_days, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('user.pswdKeepOldQty', @password_keep_old_qty, NULL, NULL, FALSE);
-- user.LoginTmpltList
CALL tmp_QPCfg_set('smtpSrv', @smtp_server, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('emailAddr', @email_address, NULL, NULL, FALSE);
CALL tmp_QPCfg_set('internalWWW', @internalWWW, NULL, NULL, FALSE);
    
-- all request are dropped by that value
update QPSvc set cleanUpDays = 7;

DROP PROCEDURE IF EXISTS `tmp_QPCfg_set`;
