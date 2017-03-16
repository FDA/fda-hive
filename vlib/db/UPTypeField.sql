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

DROP TABLE IF EXISTS `UPTypeField`;

CREATE TABLE `UPTypeField` (
  `type_id` bigint(20) unsigned NOT NULL,
  `name` varchar(255) NOT NULL,
  `title` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `type` enum('string','text','integer','real','bool','array','list','date','time','datetime','url','obj','password','file','type2array','type2list') NOT NULL,
  `parent` varchar(255) DEFAULT NULL,
  `role` enum('in','out') DEFAULT NULL,
  `is_key_fg` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'field is part of unique identifier of the record',
  `is_readonly_fg` tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 - normal field\n1 - calculated outside and NOT editable on web pages\n2 - autofill with default value\n-1 - write-once\n-2 - read only in resubmit\n',
  `is_optional_fg` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'field could be missing completely',
  `is_multi_fg` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'field can have multiple values',
  `is_hidden_fg` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'field is not visible on pages',
  `brief` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'NULL or empty not a brief member; [text]$(v)[text], where $(v) is substituted by prop value',
  `is_summary_fg` tinyint(1) NOT NULL DEFAULT '0',
  `is_virtual_fg` tinyint(1) NOT NULL DEFAULT '0',
  `is_batch_fg` tinyint(4) NOT NULL DEFAULT '0',
  `order` float DEFAULT NULL,
  `default_value` mediumtext CHARACTER SET utf8 COLLATE utf8_unicode_ci COMMENT 'default vaue for the field',
  `default_encoding` smallint(6) DEFAULT NULL COMMENT 'default encoding for field values',
  `link_url` mediumtext CHARACTER SET utf8 COLLATE utf8_unicode_ci COMMENT 'used to link to a page using current field value as $(v) substitution in url',
  `constraint` enum('choice','choice+','regexp','range','url','search','search+','one-of','type','eval') DEFAULT NULL COMMENT 'type: list, select, url, etc...',
  `constraint_data` mediumtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `constraint_description` mediumtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `description` mediumtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  UNIQUE KEY `unique_id` (`type_id`,`name`),
  KEY `name_idx` (`name`),
  KEY `type_idx` (`type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
