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

START TRANSACTION;

TRUNCATE TABLE UPUser;
TRUNCATE TABLE UPGroup;

-- special system group
SET @system_group = '/system/';
SET @system_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = TRUE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'system',
    email = '', pswd = '--not an account--', first_name = @system_group, last_name = 'SYSTEM', createTm = CURRENT_TIMESTAMP;
SET @system_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @system_group_id, flags = -1, is_active_fg = TRUE, groupPath = @system_group, createTm = CURRENT_TIMESTAMP;
SET @system_group_id = LAST_INSERT_ID();
SET @system_membership = CONCAT('( ((g.groupPath in (\'', @system_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @system_group, '%\') AND (p.flags & 4)) )');
SET @system_permission = 4294967295; -- all
SET @system_flags = 2; -- down

SET @admins_group = '/system/admins/';
SET @admins_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'system',
    email = '', pswd = '--not an account--', first_name = @admins_group, last_name = 'Administrators', createTm = CURRENT_TIMESTAMP;
SET @admins_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @admins_group_id, flags = -1, is_active_fg = TRUE, groupPath = @admins_group, createTm = CURRENT_TIMESTAMP;
SET @admins_group_id = LAST_INSERT_ID();
SET @admins_membership = CONCAT('( ((g.groupPath in (\'', @system_group, '\',\'', @admins_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @admins_group, '%\') AND (p.flags & 4)) )');
SET @admins_permission = 3; -- browse, read
SET @admins_flags = 2; -- down

SET @type_editors_group = '/system/type-editors/';
SET @type_editors_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'system',
    email = '', pswd = '--not an account--', first_name = @type_editors_group, last_name = 'Type editors', createTm = CURRENT_TIMESTAMP;
SET @type_editors_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @type_editors_group_id, flags = -1, is_active_fg = TRUE, groupPath = @type_editors_group, createTm = CURRENT_TIMESTAMP;
SET @type_editors_group_id = LAST_INSERT_ID();
SET @type_editors_membership = CONCAT('( ((g.groupPath in (\'', @system_group, '\',\'', @type_editors_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @type_editors_group, '%\') AND (p.flags & 4)) )');
SET @type_editors_permission = 3; -- browse, read
SET @type_editors_flags = 2; -- down

-- anything accessing the system is automatic member of this group
SET @everyone_group = '/everyone/';
SET @everyone_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'group',
    email = '', pswd = '--not an account--', first_name = @everyone_group, last_name = 'Everyone (world)', createTm = CURRENT_TIMESTAMP;
SET @everyone_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @everyone_group_id, flags = -1, is_active_fg = TRUE, groupPath = @everyone_group, createTm = CURRENT_TIMESTAMP;
SET @everyone_group_id = LAST_INSERT_ID();
SET @everyone_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @everyone_group, '%\') AND (p.flags & 4)) )');
SET @everyone_permission = 3; -- browse, read
SET @everyone_flags = 2; -- down

-- all guest users are automatic members of this group
SET @public_group = '/everyone/public/';
SET @public_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'group',
    email = '', pswd = '--not an account--', first_name = @public_group, last_name = 'Guest users (public)', createTm = CURRENT_TIMESTAMP;
SET @public_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @public_group_id, flags = -1, is_active_fg = TRUE, groupPath = @public_group, createTm = CURRENT_TIMESTAMP;
SET @public_group_id = LAST_INSERT_ID();
SET @public_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\',\'', @public_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @public_group, '%\') AND (p.flags & 4)) )');
SET @public_permission = 3; -- browse, read
SET @public_flags = 2; -- down

-- all registred users are automatic members of this group
-- this also provides users with their own individual private group
SET @users_group = '/everyone/users/';
SET @users_group_id = NULL;
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'group',
    email = '', pswd = '--not an account--', first_name = @users_group, last_name = 'All registered users', createTm = CURRENT_TIMESTAMP;
SET @users_group_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @users_group_id, flags = -1, is_active_fg = TRUE, groupPath = @users_group, createTm = CURRENT_TIMESTAMP;
SET @users_group_id = LAST_INSERT_ID();
SET @users_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\',\'', @users_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @users_group, '%\') AND (p.flags & 4)) )');
SET @users_permission = 3; -- read, browse
SET @users_flags = 2; -- down

SET @guest_user = 'guest';
INSERT INTO UPUser SET
    is_admin_fg = FALSE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'system', logCount = 1,
    email = @guest_user, pswd = 'insert you password here, its an optional account', first_name = 'Guest', last_name = '', createTm = CURRENT_TIMESTAMP;
SET @guest_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @guest_id, flags = -1, is_active_fg = TRUE, groupPath = CONCAT(@public_group, @guest_user), createTm = CURRENT_TIMESTAMP;

SET @qpride_user = 'qpride';
INSERT INTO UPUser SET
    is_admin_fg = TRUE, is_active_fg = TRUE, is_email_valid_fg = FALSE, `type` = 'service', logCount = 1,
    email = @qpride_user, pswd = '--service account--', first_name = 'QPride', last_name = '',
    createTm = CURRENT_TIMESTAMP;
SET @qpride_id = LAST_INSERT_ID();
INSERT INTO UPGroup SET userID = @qpride_id, flags = -1, is_active_fg = TRUE, groupPath = CONCAT(@users_group, @qpride_user), createTm = CURRENT_TIMESTAMP;
INSERT INTO UPGroup SET userID = @qpride_id, flags = 0, is_active_fg = TRUE, groupPath = CONCAT(@system_group, @qpride_user), createTm = CURRENT_TIMESTAMP;

-- ----------------------
-- ----------------------
-- ----------------------

COMMIT;
