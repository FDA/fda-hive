
DROP PROCEDURE IF EXISTS `sp_type_get_latest_mtime`;

DELIMITER //

CREATE PROCEDURE `sp_type_get_latest_mtime`(
    IN p_group_id BIGINT UNSIGNED,
    IN p_member_sql VARCHAR(21844),
    IN p_type_type_domain BIGINT UNSIGNED,
    IN p_type_type_id BIGINT UNSIGNED
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

    IF IFNULL(p_type_type_id, 0) = 0 OR p_type_type_domain IS NULL THEN
        SELECT objTypeDomainID, objTypeID
            FROM UPObj
            WHERE objTypeDomainID = domainID AND objTypeID = objID
            INTO p_type_type_domain, p_type_type_id;
    END IF;
    SELECT p_type_type_domain AS domainID, p_type_type_id AS objID;

    SET @type_action_domain = 0;
    SET @type_action_id = 0;
    SET @type_view_domain = 0;
    SET @type_view_id = 0;
    SELECT o.domainID, o.objID
        FROM UPObj o JOIN UPObjField f ON o.objID = f.objID AND (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL))
        WHERE o.objTypeDomainID = p_type_type_domain AND o.objTypeID = p_type_type_id AND f.`name` = 'name' AND f.`value` = 'action' LIMIT 1
        INTO @type_action_domain, @type_action_id;
    SELECT o.domainID, o.objID
        FROM UPObj o JOIN UPObjField f ON o.objID = f.objID AND (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL))
        WHERE o.objTypeDomainID = p_type_type_domain AND o.objTypeID = p_type_type_id AND f.`name` = 'name' AND f.`value` = 'view' LIMIT 1
        INTO @type_view_domain, @type_view_id;

    SELECT o.domainID as domainID, o.objID AS objID, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
        FROM UPObj o JOIN UPObjField f ON o.objID = f.objID AND (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL))
        WHERE o.objTypeDomainID = p_type_type_domain AND o.objTypeID = p_type_type_id AND f.`name` = 'modified' ORDER BY f.`value` DESC LIMIT 1;
    SELECT o.domainID as domainID, o.objID AS objID, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
        FROM UPObj o JOIN UPObjField f ON o.objID = f.objID AND (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL))
        WHERE o.objTypeDomainID = @type_action_domain AND o.objTypeID = @type_action_id AND f.`name` = 'modified' ORDER BY f.`value` DESC LIMIT 1;
    SELECT o.domainID as domainID, o.objID AS objID, f.`name` AS `name`, f.`group` AS `group`, f.`value` AS `value`
        FROM UPObj o JOIN UPObjField f ON o.objID = f.objID AND (o.domainID = f.domainID OR (o.domainID = 0 AND f.domainID IS NULL))
        WHERE o.objTypeDomainID = @type_view_domain AND o.objTypeID = @type_view_id AND f.`name` = 'modified' ORDER BY f.`value` DESC LIMIT 1;
END //
DELIMITER ;
