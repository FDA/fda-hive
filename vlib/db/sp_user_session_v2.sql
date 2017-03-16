
DROP PROCEDURE IF EXISTS `sp_user_session_v2`;

DELIMITER //

CREATE PROCEDURE `sp_user_session_v2`(
    IN p_session_id BIGINT UNSIGNED,
    IN p_user_id BIGINT UNSIGNED,
    IN p_time BIGINT UNSIGNED,
    IN p_rnd BIGINT
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

    DECLARE l_user_id BIGINT UNSIGNED DEFAULT 0;

    SELECT u.userID FROM UPUser u JOIN UPSession s ON u.userID = s.user_id
        WHERE s.session_id = p_session_id AND s.rnd IS NOT NULL AND p_rnd IS NOT NULL AND s.rnd = p_rnd AND
              s.ended IS NULL AND u.is_active_fg = TRUE AND u.`type` = 'user' AND u.userId = p_user_id
    INTO l_user_id;

    IF l_user_id > 0 AND l_user_id IS NOT NULL THEN
        UPDATE UPSession SET accessed = FROM_UNIXTIME(p_time) WHERE user_id = l_user_id AND session_id = p_session_id;
        SELECT l_user_id;
    END IF;
END //
DELIMITER ;
