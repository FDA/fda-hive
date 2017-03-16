#!/bin/bash
#/*
# *  ::718604!
# * 
# * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
# * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
# * Affiliation: Food and Drug Administration (1), George Washington University (2)
# * 
# * All rights Reserved.
# * 
# * The MIT License (MIT)
# * 
# * Permission is hereby granted, free of charge, to any person obtaining
# * a copy of this software and associated documentation files (the "Software"),
# * to deal in the Software without restriction, including without limitation
# * the rights to use, copy, modify, merge, publish, distribute, sublicense,
# * and/or sell copies of the Software, and to permit persons to whom the
# * Software is furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included
# * in all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# */

# !!! THIS SCRIPT IS AUTOMATICALLY GENERATED !!!

DB=$1
DBUSER=$2
MYSQL_PWD=$3
DBHOST=$4
DBPORT=$5
if [[ "${DB}" = "" ]]; then
  echo -n "Database name [QPride]? "
  read DB
  if [[ "${DB}" = "" ]]; then
      DB="QPride"
  fi
fi
if [[ "${DBUSER}" = "" ]]; then
  echo -n "Database user [qpride]? "
  read DBUSER
  if [[ "${DBUSER}" = "" ]]; then
    DBUSER="qpride"
  fi
fi
if [[ "${MYSQL_PWD}" = "" ]]; then
  echo -n "Database password [q666pride]? "
  read MYSQL_PWD
  if [[ "${MYSQL_PWD}" = "" ]]; then
    MYSQL_PWD="q666pride"
  fi
fi
if [[ "${DBHOST}" = "" ]]; then
  echo -n "Database host [localhost] or type qpridesrv? "
  read DBHOST
  if [[ "${DBHOST}" = "" ]]; then
    DBHOST="localhost"
  fi
fi
if [[ "${DBPORT}" = "" ]]; then
  echo -n "Database port [3306]? "
  read DBPORT
  if [[ "${DBPORT}" = "" ]]; then
    DBPORT="3306"
  fi
fi

echo "Creating database '${DB}' on host ${DBHOST}:${DBPORT} for user '${DBUSER}':'${MYSQL_PWD}'"

rm -rf /tmp/tmp_${DB}_stmt.sql
echo "USE mysql;" >> /tmp/tmp_${DB}_stmt.sql
echo "DROP PROCEDURE IF EXISTS sp_tmp_${DB}_create;" >> /tmp/tmp_${DB}_stmt.sql
echo "DELIMITER //" >> /tmp/tmp_${DB}_stmt.sql
echo "CREATE PROCEDURE sp_tmp_${DB}_create()" >> /tmp/tmp_${DB}_stmt.sql
echo "    MODIFIES SQL DATA" >> /tmp/tmp_${DB}_stmt.sql
echo "    COMMENT 'tmp stored proc, please drop it!! \$Id \$'" >> /tmp/tmp_${DB}_stmt.sql
echo "BEGIN" >> /tmp/tmp_${DB}_stmt.sql
echo "    IF CURRENT_USER() NOT LIKE 'root@%' THEN" >> /tmp/tmp_${DB}_stmt.sql
echo "        SELECT 'Must be root to run this script' as ErrorMessage;" >> /tmp/tmp_${DB}_stmt.sql
echo "    ELSEIF EXISTS (SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '${DB}') THEN" >> /tmp/tmp_${DB}_stmt.sql
echo "       SET @mt='Database ''${DB}'' already exists';" >> /tmp/tmp_${DB}_stmt.sql
echo "       SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = @mt;" >> /tmp/tmp_${DB}_stmt.sql
echo "    ELSE" >> /tmp/tmp_${DB}_stmt.sql
echo "        CREATE DATABASE IF NOT EXISTS ${DB} DEFAULT CHARACTER SET latin1;" >> /tmp/tmp_${DB}_stmt.sql
echo "        SELECT COUNT(*) FROM user WHERE User = '${DBUSER}' and Host = '%'" >> /tmp/tmp_${DB}_stmt.sql
echo "        INTO @qu;" >> /tmp/tmp_${DB}_stmt.sql
echo "        IF @qu = 0 THEN" >> /tmp/tmp_${DB}_stmt.sql
echo "            CREATE USER '${DBUSER}'@'%' IDENTIFIED BY '${MYSQL_PWD}';" >> /tmp/tmp_${DB}_stmt.sql
echo "        END IF;" >> /tmp/tmp_${DB}_stmt.sql
echo "        GRANT ALL ON ${DB}.* TO '${DBUSER}'@'%';" >> /tmp/tmp_${DB}_stmt.sql
echo "        SELECT COUNT(*) FROM user WHERE User = '${DBUSER}' and Host = 'localhost'" >> /tmp/tmp_${DB}_stmt.sql
echo "        INTO @qu;" >> /tmp/tmp_${DB}_stmt.sql
echo "        IF @qu = 0 THEN" >> /tmp/tmp_${DB}_stmt.sql
echo "            CREATE USER '${DBUSER}'@'localhost' IDENTIFIED BY '${MYSQL_PWD}';" >> /tmp/tmp_${DB}_stmt.sql
echo "        END IF;" >> /tmp/tmp_${DB}_stmt.sql
echo "        GRANT ALL ON ${DB}.* TO '${DBUSER}'@'localhost';" >> /tmp/tmp_${DB}_stmt.sql
echo "        SHOW GRANTS FOR '${DBUSER}'@'%';" >> /tmp/tmp_${DB}_stmt.sql
echo "        SHOW GRANTS FOR '${DBUSER}'@'localhost';" >> /tmp/tmp_${DB}_stmt.sql
echo "    END IF;" >> /tmp/tmp_${DB}_stmt.sql
echo "END //" >> /tmp/tmp_${DB}_stmt.sql
echo "DELIMITER ;" >> /tmp/tmp_${DB}_stmt.sql
echo "CALL sp_tmp_${DB}_create();" >> /tmp/tmp_${DB}_stmt.sql
echo "DROP PROCEDURE IF EXISTS sp_tmp_${DB}_create;" >> /tmp/tmp_${DB}_stmt.sql
echo -n "For MySQL root "; mysql -h ${DBHOST} -P${DBPORT} -u root -p --comments < /tmp/tmp_${DB}_stmt.sql
if [[ $? != 0 ]]; then
    echo "Database creation failed"
    exit 5
fi
rm -rf /tmp/tmp_${DB}_stmt.sql
./db_init.sh ${DB} ${DBUSER} ${MYSQL_PWD} ${DBHOST} ${DBPORT}
