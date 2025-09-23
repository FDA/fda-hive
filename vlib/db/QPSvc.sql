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

DROP TABLE IF EXISTS `QPSvc`;

CREATE TABLE `QPSvc` (
  `svcID` bigint(20) NOT NULL AUTO_INCREMENT,
  `permID` bigint(20) NOT NULL DEFAULT 0,
  `svcType` int(11) NOT NULL DEFAULT 0,
  `knockoutSec` int(11) NOT NULL DEFAULT 900,
  `maxJobs` int(11) NOT NULL DEFAULT 2,
  `nice` int(11) NOT NULL DEFAULT 0,
  `maxLoops` int(11) NOT NULL DEFAULT 100,
  `sleepTime` int(11) NOT NULL DEFAULT 60000,
  `parallelJobs` int(11) NOT NULL DEFAULT 0,
  `delayLaunchSec` int(11) NOT NULL DEFAULT 1,
  `politeExitTimeoutSec` int(11) NOT NULL DEFAULT 60,
  `maxTrials` int(11) NOT NULL DEFAULT 3,
  `restartSec` int(11) NOT NULL DEFAULT 900,
  `priority` int(11) NOT NULL DEFAULT 0,
  `cleanUpDays` int(11) NOT NULL DEFAULT 28,
  `runInMT` int(11) NOT NULL DEFAULT 0,
  `noGrabDisconnect` int(11) NOT NULL DEFAULT 5,
  `noGrabExit` int(11) NOT NULL DEFAULT 10,
  `lazyReportSec` int(11) NOT NULL DEFAULT 10,
  `isUp` bigint(20) NOT NULL DEFAULT 0,
  `delayedAction` int(11) NOT NULL DEFAULT 0,
  `maxmemSoft` bigint(20) NOT NULL DEFAULT 0,
  `maxmemHard` bigint(20) NOT NULL DEFAULT 0,
  `cdate` timestamp NOT NULL DEFAULT current_timestamp(),
  `name` varchar(128) NOT NULL,
  `title` varchar(128) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `cmdLine` varchar(128) NOT NULL DEFAULT '$(svc).os$(os)',
  `hosts` varchar(4096) NOT NULL DEFAULT '/QPrideSrv/',
  `emails` varchar(128) NOT NULL DEFAULT '',
  `categories` varchar(128) NOT NULL DEFAULT 'All',
  `activeJobReserve` int(11) NOT NULL DEFAULT 0,
  `capacity` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`svcID`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
