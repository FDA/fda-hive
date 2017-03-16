
DROP PROCEDURE IF EXISTS `tmp_QPSvc_add`;

DELIMITER //

CREATE PROCEDURE `tmp_QPSvc_add`(
    IN p_name VARCHAR(128),
    IN p_title VARCHAR(128),
    IN p_categories VARCHAR(128),
    IN p_params_csv MEDIUMTEXT
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
    IF p_name IS NOT NULL AND LENGTH(p_name) > 0 THEN
        SET @sxd = NULL;
        SELECT svcID FROM QPSvc where name = p_name
        INTO @sxd;
        IF @sxd IS NULL THEN
            -- choose biggest domain by default
            SELECT val FROM QPCfg where par LIKE 'qm.domains_%' ORDER BY LENGTH(val) DESC LIMIT 1
            INTO @hosts;
            INSERT INTO QPSvc (name, title, categories, hosts, isUp) VALUES (TRIM(p_name), TRIM(p_title), TRIM(p_categories), @hosts, TRUE);
            SET @id = LAST_INSERT_ID();
            IF @id IS NOT NULL AND @id > 0 THEN
                SELECT CONCAT('Service #', @id, ' ', p_name, ' registered ok') AS message;
                SET @q = TRIM(',' FROM p_params_csv);
                SET @q = TRIM(@q);
                IF @q IS NOT NULL AND LENGTH(@q) > 0 THEN
                    SET @q = CONCAT('UPDATE QPSvc SET ', @q, ' WHERE svcID = ', @id);
                    PREPARE x FROM @q;
                    EXECUTE x;
                END IF;
            ELSE
                SET @mt = CONCAT('Failed to register service "', p_name, '"');
                SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
            END IF;
        ELSE
            UPDATE QPSvc SET title = TRIM(p_title), categories = TRIM(p_categories) WHERE svcID = @sxd;
            SELECT CONCAT('Service ', p_name, ' already exists') AS message;
        END IF;
    END IF;
END //
DELIMITER ;

DROP PROCEDURE IF EXISTS `tmp_QPSvc_set_config`;

DELIMITER //

CREATE PROCEDURE `tmp_QPSvc_set_config`(
    IN p_name VARCHAR(128),
    IN p_params_csv MEDIUMTEXT
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
    IF @local_scratch_dir IS NULL OR LENGTH(@local_scratch_dir) < 1 THEN
        SET @mt = CONCAT('Missing @local_scratch_dir');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF @global_scratch_dir IS NULL OR LENGTH(@global_scratch_dir) < 1 THEN
        SET @mt = CONCAT('Missing @global_scratch_dir');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF @bin_dir IS NULL OR LENGTH(@bin_dir) < 1 THEN
        SET @mt = CONCAT('Missing @bin_dir');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF p_name IS NOT NULL AND LENGTH(p_name) > 0 THEN
        DELETE FROM QPCfg WHERE par LIKE CONCAT(p_name, '.%');
        SET @rq = ROW_COUNT();
        IF @rq != 0 THEN
            SELECT CONCAT(@rq, ' parameters deleted') AS message;
        END IF;
        SET @q = TRIM(',' FROM p_params_csv);
        SET @q = TRIM(@q);
        IF @q IS NOT NULL AND LENGTH(@q) > 0 THEN
            SET @q = REPLACE(@q, ',', CONCAT('),(''', p_name, '.'));
            SET @q = REPLACE(@q, '=', ''',');
            SET @q = REPLACE(@q, '@local_scratch_dir@', @local_scratch_dir);
            SET @q = REPLACE(@q, '@global_scratch_dir@', @global_scratch_dir);
            SET @q = REPLACE(@q, '@bin_dir@', @bin_dir);
            SET @q = CONCAT('INSERT INTO QPCfg (par, val) VALUES (''', p_name, '.', @q, ')');
            PREPARE x FROM @q;
            EXECUTE x;
            SELECT CONCAT(ROW_COUNT(), ' parameters set') AS message;
        END IF;
    END IF;
END //
DELIMITER ;

DROP PROCEDURE IF EXISTS `tmp_QPSvc_set_config_path`;

DELIMITER //

CREATE PROCEDURE `tmp_QPSvc_set_config_path`(
    IN p_name VARCHAR(128),
    IN p_param MEDIUMTEXT,
    IN p_cleanUpDays VARCHAR(128),
    IN p_nameMasks VARCHAR(128)
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
    IF p_name IS NOT NULL AND LENGTH(p_name) > 0 AND p_param IS NOT NULL AND LENGTH(p_param) > 0 THEN
        SET p_cleanUpDays = TRIM(p_cleanUpDays);
        SET p_nameMasks = TRIM(p_nameMasks);
        IF p_cleanUpDays IS NULL OR LENGTH(p_cleanUpDays) < 1 THEN
            SET p_cleanUpDays = NULL;
        END IF;
        IF p_nameMasks IS NULL OR LENGTH(p_nameMasks) < 1 THEN
            SET p_nameMasks = NULL;
        END IF;
        SET @nm = CONCAT(TRIM(p_name), '.', TRIM(p_param));
        UPDATE QPCfg SET cleanUpDays = p_cleanUpDays, nameMasks = p_nameMasks WHERE par = @nm;
        SET @rq = ROW_COUNT();
        IF @rq != 1 THEN
            SET @mt = CONCAT('Parameter "', @nm, '" not found');
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
        ELSE
            SELECT CONCAT('Parameter "', @nm, '" flags set') AS message;
        END IF;
    END IF;
END //
DELIMITER ;

DROP PROCEDURE IF EXISTS `tmp_path_cut`;

DELIMITER //

CREATE PROCEDURE `tmp_path_cut`(
    IN p_in MEDIUMTEXT,
    OUT p_out MEDIUMTEXT
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
    SET @g = p_in;
    IF p_in IS NOT NULL AND LENGTH(p_in) > 0 THEN
        SET @g = TRIM(TRAILING '/' FROM p_in);
        -- remove last component of the path letter by letter
        REPEAT
            SET @g = SUBSTRING(@g, 1, LENGTH(@g) - 1);
        UNTIL SUBSTRING(@g, LENGTH(@g)) = '/' END REPEAT;
    END IF;
    SET p_out = @g;
END //
DELIMITER ;


SELECT val FROM QPCfg WHERE par = 'qm.tempDirectory' INTO @local_scratch_dir;
SELECT val FROM QPCfg WHERE par = 'qm.largeDataRepository' INTO @global_scratch_dir;
SELECT val FROM QPCfg WHERE par = 'qm.resourceRoot' INTO @bin_dir;

CALL tmp_path_cut(@global_scratch_dir, @global_scratch_dir);

DROP PROCEDURE IF EXISTS `tmp_path_cut`;
