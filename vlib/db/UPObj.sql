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

DROP TABLE IF EXISTS `UPObj`;

CREATE TABLE `UPObj` (
  `domainID` bigint(20) unsigned NOT NULL,
  `objID` bigint(20) unsigned NOT NULL,
  `ionID` bigint(20) unsigned DEFAULT NULL COMMENT 'TO BE DELETED DO NOT USE',
  `objTypeID` bigint(20) unsigned NOT NULL,
  `creatorID` bigint(20) unsigned NOT NULL,
  `flags` int(11) NOT NULL DEFAULT 0,
  `xtraPass` varchar(256) DEFAULT NULL,
  `softExpiration` datetime DEFAULT NULL,
  `hardExpiration` datetime DEFAULT NULL,
  `ref_count` bigint(20) unsigned NOT NULL DEFAULT 0,
  `objTypeDomainID` bigint(20) unsigned NOT NULL,
  UNIQUE KEY `uniq_idx` (`domainID`,`objID`),
  KEY `softexp_idx` (`softExpiration`),
  KEY `creator_idx` (`creatorID`),
  KEY `objID_idx_for_purge` (`objID`),
  KEY `hardexp_idx` (`hardExpiration`),
  KEY `type_idx` (`objTypeID`),
  KEY `type_dom_idx` (`objTypeDomainID`,`objTypeID`),
  KEY `type_expir_idx` (`objTypeID`,`softExpiration`,`hardExpiration`),
  KEY `type_dom_expir_idx` (`objTypeDomainID`,`objTypeID`,`softExpiration`,`hardExpiration`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
