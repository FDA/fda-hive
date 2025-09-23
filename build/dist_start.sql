
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
    DECLARE new_par MEDIUMTEXT;
    DECLARE new_val MEDIUMTEXT;
    DECLARE curs_done INT DEFAULT 0;
    DECLARE curs CURSOR FOR SELECT par, val FROM QPCfg_TEMP;
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET curs_done = 1;

    IF @local_scratch_dir IS NULL OR LENGTH(@local_scratch_dir) < 1 THEN
        SET @mt = CONCAT('Missing @local_scratch_dir');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF @global_scratch_dirs IS NULL OR LENGTH(@global_scratch_dirs) < 1 THEN
        SET @mt = CONCAT('Missing @global_scratch_dirs');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF @bin_dir IS NULL OR LENGTH(@bin_dir) < 1 THEN
        SET @mt = CONCAT('Missing @bin_dir');
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;
    END IF;
    IF p_name IS NOT NULL AND LENGTH(p_name) > 0 THEN
        SET @q = TRIM(',' FROM p_params_csv);
        SET @q = TRIM(@q);
        IF @q IS NOT NULL AND LENGTH(@q) > 0 THEN
            CREATE TEMPORARY TABLE QPCfg_TEMP AS SELECT par,val FROM QPCfg LIMIT 1;
            TRUNCATE QPCfg_TEMP;
            -- convert "p1='a',p2='b',p3='c'" => "nm.p1', 'a'),('nm.p2', 'b'),('nm.p3', 'c"
            -- escape doulbe ,
            SET @q = REPLACE(@q, ',,', '~~~~~~~~~~~HIVE~~~~~~~~~~~~~~~~~');
            SET @q = REPLACE(@q, ',', CONCAT('),(''', p_name, '.'));
            SET @q = REPLACE(@q, '=', ''',');
            SET @q = REPLACE(@q, '@local_scratch_dir@', @local_scratch_dir);
            SET @q = REPLACE(@q, '@global_scratch_dir@', @global_scratch_dirs);
            SET @q = REPLACE(@q, '@bin_dir@', @bin_dir);
            SET @q = REPLACE(@q, '~~~~~~~~~~~HIVE~~~~~~~~~~~~~~~~~', ',');
            SET @q = CONCAT('INSERT INTO QPCfg_TEMP (par, val) VALUES (''', p_name, '.', @q, ')');
            PREPARE x FROM @q;
            EXECUTE x;
            OPEN curs;
            SET curs_done = 0;
            REPEAT
                FETCH curs INTO new_par, new_val;
                IF new_par IS NOT NULL AND LENGTH(new_par) > 0 AND new_val IS NOT NULL AND LENGTH(new_val) > 0 THEN
                    SELECT COUNT(par) FROM QPCfg WHERE par = new_par INTO @qp;
                    IF @qp < 1 THEN
                        INSERT INTO QPCfg (par, val) VALUES (new_par, new_val);
                        IF ROW_COUNT() > 0 THEN
                            SELECT CONCAT('Parameter ''', new_par, ''' is set to ''', new_val, '''') AS message;
                        END IF;
                    END IF;
                END IF;
            UNTIL curs_done END REPEAT;
            CLOSE curs;
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
        SET @nm = CONCAT(TRIM(p_name), '.', TRIM(p_param));
        SET p_cleanUpDays = TRIM(IFNULL(p_cleanUpDays, ''));
        IF p_cleanUpDays REGEXP '^[0-9]+$' THEN
            SELECT cleanUpDays FROM QPCfg WHERE par = @nm INTO @cd;
            IF @cd IS NULL OR LENGTH(TRIM(@cd)) < 1 THEN
                UPDATE QPCfg SET cleanUpDays = p_cleanUpDays WHERE par = @nm;
                SELECT CONCAT('Parameter "', @nm, '" flag cleanUpDays is set to ', p_cleanUpDays) AS message;
            END IF;
        END IF;
        SET p_nameMasks = TRIM(IFNULL(p_nameMasks, ''));
        IF p_nameMasks REGEXP '^[0-9]+$' THEN
            SELECT nameMasks FROM QPCfg WHERE par = @nm INTO @ns;
            IF @ns IS NULL OR LENGTH(TRIM(@ns)) < 1 THEN
                UPDATE QPCfg SET nameMasks = p_nameMasks WHERE par = @nm;
                SELECT CONCAT('Parameter "', @nm, '" flag nameMasks is set to ', p_nameMasks) AS message;
            END IF;
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
    
        SET p_in = SUBSTRING_INDEX(p_in,',',1);
    
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
SELECT val FROM QPCfg WHERE par = 'qm.largeDataRepository' INTO @global_scratch_dirs;
SELECT val FROM QPCfg WHERE par = 'qm.resourceRoot' INTO @bin_dir;

CALL tmp_path_cut(@global_scratch_dirs, @global_scratch_dirs);

-- SELECT CONCAT('@global_scratch_dirs=''', @global_scratch_dirs, ''', @local_scratch_dir=''', @local_scratch_dir, ''', @bin_dir=''', @bin_dir, '''') AS debug;

DROP PROCEDURE IF EXISTS `tmp_path_cut`;
