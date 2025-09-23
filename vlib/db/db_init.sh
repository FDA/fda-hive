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

# adjust DBHOST to be localhost if provided server name is local/sock
if [[ "${DBHOST}" != "localhost" && "${DBHOST}" != "127.0.0.1" ]]; then
   PING=`which ping 2>/dev/null`
   IPAD=`which ip 2>/dev/null`
   if [[ "x${IPAD}" == "x" ]]; then
       IPAD=/sbin/ip
   fi
   if [[ "x${PING}" != "x" ]]; then
       if [[ "x${IPAD}" != "x" ]]; then
           IP4=`ping -c 1 ${DBHOST} | head -2 | tail -1 | gawk '{print substr($5,2,length($5)-3)}'`
           if [[ "x${IP4}" != "x" ]]; then
               ${IPAD} ad | grep "${IP4}" 1>/dev/null 2>&1
               if [[ $? -eq 0 ]]; then
                   DBHOST=localhost
               fi
fi
fi
fi
fi
echo "Initializing database '${DB}' on host ${DBHOST}:${DBPORT} for user '${DBUSER}':'${MYSQL_PWD}'"

./db_init_sp.sh ${DB} ${DBUSER} ${MYSQL_PWD} ${DBHOST} ${DBPORT} || exit 1
echo "Initializing database '${DB}' tables on host ${DBHOST}:${DBPORT} for user '${DBUSER}':'${MYSQL_PWD}'"

cat db_init_tbl.sql | mysql -h ${DBHOST} -P${DBPORT} -u ${DBUSER} -p${MYSQL_PWD} --comments ${DB}
if [[ $? != 0 ]]; then
    echo "Database tables initialization failed"
    exit 5
fi
echo "Initializing database '${DB}' data on host ${DBHOST}:${DBPORT} for user '${DBUSER}':'${MYSQL_PWD}'"

cat db_init_data.sql | mysql -h ${DBHOST} -P${DBPORT} -u ${DBUSER} -p${MYSQL_PWD} --comments ${DB}
if [[ $? != 0 ]]; then
    echo "Database data initialization failed"
    exit 5
fi
if [[ -s hive_config.sql ]]; then
    HIVECFG_SQL=hive_config.sql
fi
if [[ ! -z ${HIVECFG_SQL} ]]; then
cat ${HIVECFG_SQL} | mysql -h ${DBHOST} -P${DBPORT} -u ${DBUSER} -p${MYSQL_PWD} --comments ${DB}
    if [[ $? != 0 ]]; then
        echo "HIVE db configuration failed"
        exit 5
    fi
fi
