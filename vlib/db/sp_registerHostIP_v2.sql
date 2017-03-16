
DROP PROCEDURE IF EXISTS `sp_registerHostIP_v2`;

DELIMITER //

CREATE PROCEDURE `sp_registerHostIP_v2`(
    IN p_hostname VARCHAR(128),
    IN p_ipaddr VARCHAR(16),
    IN p_sys VARCHAR(16),
    IN p_capacity FLOAT,
    IN ncores BIGINT UNSIGNED,
    IN `memory` BIGINT UNSIGNED
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
    DECLARE l_hostExists BIGINT UNSIGNED DEFAULT 0;

    IF p_hostname IS NOT NULL AND LENGTH(p_hostname) > 0 THEN
        IF p_ipaddr IS NOT NULL THEN
            UPDATE QPHosts SET ip4 = '' WHERE ip4 = p_ipaddr AND `name` != p_hostname;
        END IF;

        SELECT COUNT(`name`) FROM QPHosts WHERE `name` = p_hostname
        INTO l_hostExists;

        CASE
            WHEN l_hostExists = 1 THEN
                -- there is single record we just change the ip
                -- capacity, cores, memory here might be wrong though...
                UPDATE QPHosts SET ip4 = p_ipaddr, mdate = NOW()
                WHERE `name` = p_hostname;
            WHEN l_hostExists = 0 THEN
                INSERT INTO QPHosts (`name`, ip4, htype, enabled, capacity, cores, `memory`)
                    VALUES (p_hostname, p_ipaddr, p_sys, true, p_capacity, ncores, `memory`);
            WHEN l_hostExists > 1 THEN
                -- do not know which one to keep - wipe all and add new
                DELETE FROM QPHosts WHERE `name` = p_hostname;
                INSERT INTO QPHosts (`name`, ip4, htype, enabled, capacity, cores, `memory`)
                    VALUES (p_hostname, p_ipaddr, p_sys, true, p_capacity, ncores, `memory`);
        END CASE;
    END IF;
END //
DELIMITER ;
