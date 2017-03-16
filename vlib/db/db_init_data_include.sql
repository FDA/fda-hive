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

SET @system_group = '/system/';
SET @system_group_id = NULL;
SELECT groupID FROM UPGroup WHERE groupPath = @system_group INTO @system_group_id;
SET @system_membership = CONCAT('( ((g.groupPath in (\'', @system_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @system_group, '%\') AND (p.flags & 4)) )');
SET @system_permission = 4294967295; -- all
SET @system_flags = 2; -- down

SET @everyone_group = '/everyone/';
SELECT groupID FROM UPGroup WHERE groupPath = @everyone_group INTO @everyone_group_id;
SET @everyone_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @everyone_group, '%\') AND (p.flags & 4)) )');
SET @everyone_permission = 3; -- browse, read
SET @everyone_flags = 2; -- down

SET @users_group = '/everyone/users/';
SET @users_group_id = NULL;
SELECT groupID FROM UPGroup WHERE groupPath = @users_group INTO @users_group_id;
SET @users_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\',\'', @users_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @users_group, '%\') AND (p.flags & 4)) )');
SET @users_permission = 3; -- read, browse
SET @users_flags = 2; -- down

SET @public_group = '/everyone/public/';
SET @public_group_id = 0;
SELECT groupID FROM UPGroup WHERE groupPath = @public_group INTO @public_group_id;
SET @public_membership = CONCAT('( ((g.groupPath in (\'', @everyone_group, '\',\'', @public_group, '\')) AND (p.flags & 2)) OR (( groupPath like \'', @public_group, '%\') AND (p.flags & 4)) )');
SET @public_permission = 3; -- browse, read
SET @public_flags = 2; -- down
