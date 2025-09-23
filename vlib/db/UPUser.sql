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

DROP TABLE IF EXISTS `UPUser`;

CREATE TABLE `UPUser` (
  `userID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `is_active_fg` tinyint(1) NOT NULL DEFAULT 0,
  `is_admin_fg` tinyint(1) NOT NULL DEFAULT 0,
  `is_email_valid_fg` tinyint(1) NOT NULL DEFAULT 0,
  `type` enum('user','group','system','service') NOT NULL,
  `email` varchar(128) NOT NULL,
  `pswd` varchar(128) NOT NULL,
  `pswd_reset_id` int(10) unsigned DEFAULT NULL,
  `pswd_changed` datetime DEFAULT NULL,
  `pswd_prev_list` mediumtext DEFAULT NULL,
  `first_name` varchar(128) NOT NULL,
  `last_name` varchar(128) NOT NULL,
  `logCount` bigint(20) unsigned NOT NULL DEFAULT 0,
  `createTm` datetime NOT NULL,
  `modifTm` datetime DEFAULT NULL,
  `loginTm` datetime DEFAULT NULL,
  `max_sessions` int(10) unsigned NOT NULL DEFAULT 1,
  `is_billable_fg` tinyint(1) DEFAULT NULL,
  `login_failed_date` datetime DEFAULT NULL,
  `login_failed_count` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`userID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
