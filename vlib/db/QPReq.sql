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

DROP TABLE IF EXISTS `QPReq`;

CREATE TABLE `QPReq` (
  `reqID` bigint(20) NOT NULL AUTO_INCREMENT,
  `svcID` int(11) NOT NULL,
  `objID` int(11) NOT NULL DEFAULT 0,
  `jobID` bigint(20) DEFAULT 0,
  `userID` bigint(20) DEFAULT 0,
  `subIp` bigint(20) DEFAULT 0,
  `cgiIp` bigint(20) DEFAULT 0,
  `stat` int(11) NOT NULL DEFAULT 1,
  `act` int(11) NOT NULL DEFAULT 1,
  `takenCnt` int(11) DEFAULT 0,
  `priority` int(11) DEFAULT 0,
  `inParallel` int(11) DEFAULT 0,
  `progress` bigint(20) DEFAULT 0,
  `progress100` int(11) DEFAULT 0,
  `cdate` timestamp NOT NULL DEFAULT current_timestamp(),
  `takenTm` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `actTm` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `aliveTm` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `doneTm` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `purgeTm` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `userKey` bigint(20) NOT NULL DEFAULT 0,
  `grabRand` bigint(20) DEFAULT 0,
  `scheduleGrab` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`reqID`),
  KEY `qpreq_grab` (`svcID`,`jobID`,`stat`,`act`),
  KEY `qpreq_grabParallel` (`inParallel`),
  KEY `qpreq_grab_schedule` (`svcID`,`stat`,`act`,`scheduleGrab`,`priority`),
  KEY `qpreq_userid` (`userID`),
  KEY `qpreq_grab_schedule2` (`stat`,`act`,`scheduleGrab`),
  KEY `qpreq_grabRand` (`grabRand`,`stat`,`act`,`svcID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
