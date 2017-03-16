#!/bin/bash

Hdeplcfg='deploy.cfg'

hive_setup_cfg()
{
    # append new values
    cat <<EOF>>${Hdeplcfg}
Huser=${Huser}
Huid=${Huid}
Hgrp=${Hgrp}
Hgid=${Hgid}
Hpswd1=${Hpswd1}
Hqdbdb=${Hqdbdb}
Hqdbip4=${Hqdbip4}
Hqdbport=${Hqdbport}
Hqdbuser=${Hqdbuser}
Hqdbpswd=${Hqdbpswd}
EOF
    . ${Hdeplcfg}
    # overwrite
    cat <<EOF>${Hdeplcfg}
Huser=${Huser}
Huid=${Huid}
Hgrp=${Hgrp}
Hgid=${Hgid}
Hpswd1=${Hpswd1}
Hqdbdb=${Hqdbdb}
Hqdbip4=${Hqdbip4}
Hqdbport=${Hqdbport}
Hqdbuser=${Hqdbuser}
Hqdbpswd=${Hqdbpswd}
EOF
    # secure file
    chown ${Huser}:${Hgrp} ${Hdeplcfg}
    chmod o-rwx ${Hdeplcfg}
}

hive_setup_account()
{
    local USR="$1"
    local AID="$2"
    local GRP="$3"
    local GID="$4"
    local PASS="$5"

    if [[ "${PASS}" = "" ]]; then
        echo "Missing account password"
        exit 30
    fi

    local GIDOLD=`getent group ${GRP} | awk -F : '{print $3}'`
    if [[ "${GIDOLD}" == "" ]]; then
        if [[ "${GID}" == "" ]]; then
            echo "Creating group ${GRP}"
            groupadd ${GRP}
        else
            echo "Creating group ${GRP} with id ${GID}"
            groupadd --gid ${GID} ${GRP}
        fi
    elif [[ "${GID}" != "" && "${GID}" != "${GIDOLD}" ]]; then
        echo "Changing group ${GRP} id from ${GIDOLD} to ${GID}"
        groupmod --gid ${GID} ${GRP}
    fi
    if [[ $? -ne 0 ]]; then
        echo "Group operation failed"
        exit 31
    fi
    GID=`getent group ${GRP} | awk -F : '{print $3}'`

    local AIDOLD=`getent passwd ${USR} | awk -F : '{print $3}'`
    if [[ "${AIDOLD}" == "" ]]; then
        if [[ "${AID}" == "" ]]; then
            echo "Creating user ${USR}"
            useradd --gid ${GID} --password ${PASS} --create-home ${USR}
        else
            echo "Creating user ${USR} with ${AID}"
            useradd --gid ${GID} --uid ${AID} --password ${PASS} --create-home ${USR}
        fi
    elif [[ "${AID}" != "" && "${AID}" != "${AIDOLD}" ]]; then
        echo "Changing user ${USR} id from ${AIDOLD} to ${AID}"
        usermod --gid ${GID} --uid ${AID} --password ${PASS} ${USR}
    fi
    if [[ $? -ne 0 ]]; then
        echo "User operation failed"
        exit 32
    fi
    chown -R ${USR}:${GRP} /home/${USR}
    if [[ $? -ne 0 ]]; then
        echo "Chown for ${USR}:${GRP} on /home/${USR} failed"
        exit 33
    fi
    chmod g+rx,g-w,o-rwx /home/${USR}
    if [[ $? -ne 0 ]]; then
        echo "Chmod on /home/${USR} failed"
        exit 34
    fi
    AID=`id -u ${USR}`

    id -u apache 2>&1 >/dev/null
    if [[ $? -eq 0 ]]; then
        groupmems -g ${GRP} -l | grep apache >/dev/null
        if [[ $? -ne 0 ]]; then
            echo "Adding user apache to ${GRP}"
            groupmems -g ${GRP} -a apache
            if [[ $? -ne 0 ]]; then
                echo "Faile to add apache to group ${GRP}"
                exit 35
            fi
        fi
    fi
    Huid="${AID}"
    Hgid="${GID}"
    hive_setup_cfg
    echo -n "Account '${USR}' has id ${AID} with primary group '${GRP}' with id ${GID}, press Enter"
    read xx
}

hive_setup_mysql()
{
    local pkgnm="$1"
    systemctl stop ${pkgnm}
    if [[ "${OSxx}" == "ubuntu" ]]; then
        apt-get --assume-yes install mysql-server
    else
        yum install --assumeyes ${pkgnm}-server
    fi
    if [[ $? != 0 ]]; then
        echo "${pkgnm} installation failed"
        exit 61
    fi

    if [[ "${OSxx}" != "ubuntu" ]]; then

        mv /etc/my.cnf /etc/my.cnf.orig

        cat << EOF > /etc/my.cnf
[mysqld]
datadir=/var/lib/mysql
socket=/var/lib/mysql/mysql.sock
symbolic-links=0
max_allowed_packet=1024M
group_concat_max_len=4M
net_buffer_length=1M
max_connections=400 # depends on cluster size
thread_cache_size=512
query_cache_size=20M
skip-name-resolve
tmp_table_size=64M
max_heap_table_size=64M
table_open_cache=410
innodb_additional_mem_pool_size = 64M
innodb_buffer_pool_size = 2G
innodb_data_file_path = ibdata1:10M:autoextend
innodb_write_io_threads = 8
innodb_read_io_threads = 8
innodb_thread_concurrency = 16
innodb_flush_log_at_trx_commit = 1
innodb_log_buffer_size = 48M
innodb_log_file_size = 256M
innodb_log_files_in_group = 3
innodb_max_dirty_pages_pct = 90
innodb_lock_wait_timeout = 120

[mysqld_safe]
log-error=/var/log/${pkgnm}/${pkgnm}.log
pid-file=/var/run/${pkgnm}/${pkgnm}.pid

!includedir /etc/my.cnf.d
EOF
        if [[ $? != 0 ]]; then
            echo "/etc/my.cnf update failed"
            exit 62
        fi

        mkdir -p /etc/systemd/system/${pkgnm}.service.d
        if [[ $? != 0 ]]; then
            echo "${pkgnm}.service.d creation failed"
            exit 63
        fi
        cat << EOF > /etc/systemd/system/${pkgnm}.service.d/limits.conf
[Service]
LimitNOFILE=100000
EOF
        if [[ $? != 0 ]]; then
            echo "${pkgnm}.service.d/limits.conf update failed"
            exit 64
        fi
        mysql_install_db
        if [[ $? != 0 ]]; then
            echo "${pkgnm}_install_db failed"
            exit 65
        fi

        chown -R mysql:mysql /var/lib/mysql
        if [[ $? != 0 ]]; then
            echo "/var/lib/mysql chown to user mysql failed"
            exit 66
        fi
    else
        echo "WARNING: In mysql version 5.7 and above this var in [mysqld] section might be needed to support zero dates"
        echo "    sql_mode=ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION"
        echo "press Enter to continue"
        read xx
    fi #!ubuntu

    systemctl enable ${pkgnm}
    if [[ $? != 0 ]]; then
        echo "systemctl enable ${pkgnm} failed"
        exit 67
    fi
    systemctl restart ${pkgnm}
    if [[ $? != 0 ]]; then
        echo "systemctl restart ${pkgnm} failed"
        exit 68
    fi

    cat <<EOF


!!! Use this guide to answer next 5 questions:
set root password? Y
disallow root remote login? N
remove anonymous users? y
remove test? Y
reload privileged tables? Y
EOF
    mysql_secure_installation
    if [[ $? != 0 ]]; then
        echo "mysql_secure_installation failed"
        exit 69
    fi

    if [[ "${OSxx}" != "ubuntu" ]]; then
        echo "Uncomment ${pkgnm}.log file section in log rotation config, change to 'rotate 10'"
        echo "Press Enter now to edit /etc/logrotate.d/${pkgnm}"
        xx=; read xx
        vi /etc/logrotate.d/${pkgnm}

        echo -n "Supplement log rotation with root access, enter MYSQL root password: "
        rpswd=; read -s rpswd
        echo ""
        cat << EOF > ~/.my.cnf
[mysqladmin]
user=root
password=${rpswd}
EOF
        if [[ $? != 0 ]]; then
            echo "~/.my.cnf update failed"
            exit 70
        fi
        chmod go-rwx ~/.my.cnf
        # allow root global db access
        echo "UPDATE user SET Host = '%' WHERE Host = '::1' AND User = 'root';" | mysql -u root "--password=${rpswd}" mysql
    fi #!ubuntu

    systemctl restart ${pkgnm}
    if [[ $? != 0 ]]; then
        echo "${[pkgnm} service restart failed"
        exit 71
    fi
    echo 'Adding this host ip as qpridesrv to /etc/hosts'
    Hqdbip4=`/sbin/ip addr show | grep 'inet ' | awk -F '[: ]+' '{print $3;}' | grep -v "^127\." | head -1 | awk -F '/' '{print $1}'`
    if [[ "${Hqdbip4}" == "" ]]; then
        echo -e 'Cannot determine DB host IP\nPress Enter to continue...'
        read xx
    else
        hive_setup_cfg
    fi
}

hive_setup_httpd()
{
    local huser="${1}"
    local hgrp="${2}"
    local pkgnm=httpd
    local pkguser=apache

    if [[ "${OSxx}" == "ubuntu" ]]; then
        pkgnm=apache2
        pkguser="www-data"
        apt-get --assume-yes install ${pkgnm}
    else
        yum install --assumeyes ${pkgnm}
    fi

#Enable web server access, by granting first denial which is expected to be in <Directory />
    local ff=""
    if [[ "${OSxx}" == "ubuntu" ]]; then
        sed --in-place=.hive '0,/Require all denied/ s//Require all granted/' /etc/${pkgnm}/${pkgnm}.conf
        ff=/etc/apache2/conf-enabled/hive.conf
    else
        sed --in-place=.hive '0,/Require all denied/ s//Require all granted/' /etc/httpd/conf/${pkgnm}.conf
        ff=/etc/httpd/conf.d/hive.conf
    fi
    cat << EOF > ${ff}
#
# Timeout: The number of seconds before receives and sends time out.
#
Timeout 1200

<Directory "/var/www/html/">
    Options +Indexes +FollowSymLinks +ExecCGI
    AddHandler cgi-script .cgi
    AllowOverride None
    Order allow,deny
    Allow from all
    FileETag All

    # catch special files
    RewriteCond \$0 \.cfg\$ [nocase,OR]
    RewriteCond \$0 \.sql\$ [nocase,OR]
    RewriteCond \$0 _tmplt\.html\$ [nocase]
    RewriteRule (.*) / [R,L]

    RewriteCond \$0 !^\$ [nocase]
    RewriteCond \$0 !\.cgi\$ [nocase]
    RewriteCond \$1 !^ganglia\$
    RewriteRule ([^/]+)/(.+) hive.cgi?raw=1&cmd=file&f=\$1/\$2
</Directory>

EOF

    if [[ "${OSxx}" == "ubuntu" ]]; then
        echo 'LoadModule unique_id_module /usr/lib/apache2/modules/mod_unique_id.so' >> ${ff}
        echo 'LoadModule rewrite_module /usr/lib/apache2/modules/mod_rewrite.so' >> ${ff}
        echo 'LoadModule cgi_module /usr/lib/apache2/modules/mod_cgi.so' >> ${ff}
    else
        echo 'LoadModule unique_id_module modules/mod_unique_id.so' >> ${ff}
    fi

    if [[ "${OSxx}" != "ubuntu" ]]; then
        mkdir -p /etc/systemd/system/httpd.service.d
        cat << EOF > /etc/systemd/system/httpd.service.d/limits.conf
[Service]
LimitNOFILE=100000
EOF
    fi

    # add apache to HIVE linux group to allow data access
    if [[ "${hgrp}" != "" ]]; then
        usermod -G ${hgrp} ${pkguser}
        # make www root updateable by HIVE (not so secure)
        if [[ "${huser}" != "" ]]; then
            chown -R ${huser}:${hgrp} /var/www/html/
        fi
    fi
    systemctl enable ${pkgnm}
    systemctl restart ${pkgnm}
}

hive_setup_qappcfg()
{
    rm -f "$1/qapp.cfg"
    cat <<EOF >"$1/qapp.cfg"

[QPride]
   db=${Hqdbdb}
   server=${Hqdbip4}:${Hqdbport}
   user=${Hqdbuser}
   pass=${Hqdbpswd}
   pageSize=50
   permanent=true
   debug=0

[End]

EOF
    chown ${Huser}:${Hgrp} "$1/qapp.cfg" && \
    chmod ug+r,ug-wx,o-rwx "$1/qapp.cfg"
    if [[ $? != 0 ]]; then
        echo "Failed to setup $1/qapp.cfg user read-only permissions"
    fi
}

hive_setup_hive()
{
    if [[ ! -d vlib ]]; then
        cd ../../
        if [[ ! -d vlib ]]; then
            echo "Package root not found"
            exit 1
        fi
    fi
    local DBC="`pwd`/vlib/db/db_create.sh"
    if [[ ! -x "${DBC}" ]]; then
        echo "Missing ${DBC}"
        exit 2
    fi
    local HIVESQL="`pwd`/hive/HIVE.sql"
    if [[ ! -s ${HIVESQL} ]]; then
        echo "Missing ${HIVESQL}"
        exit 3
    fi

    local sql_site_hosts="/`uname -n`"
    local sql_head_hosts="${sql_site_hosts}/"
    while [[ true ]]; do
        echo -n "Add hosts to list [${sql_site_hosts}]: "
        local hh=; read hh
        if [[ "${hh}" == "" ]]; then
            break
        fi
        sql_site_hosts="${sql_site_hosts}/${hh}"
    done
    sql_site_hosts="${sql_site_hosts}/"

    local sql_bin_dir="${HOME}/bin/"
    echo -n "Path to bin directory [${sql_bin_dir}]: "
    local bd=; read bd
    if [[ "${bd}" != "" ]]; then
        sql_bin_dir="${bd/%\//}/"
    fi
    mkdir -p "${sql_bin_dir}"
    if [[ $? != 0 ]]; then
        echo "Failed to create directory ${sql_bin_dir}"
        exit 4;
    fi
    cp -pv "${qapppath}" ${sql_bin_dir}
    if [[ $? != 0 ]]; then
        echo "Failed to copy qapp to directory ${sql_bin_dir}"
        exit 4;
    fi

    local sql_local_scratch_dir="/tmp/"
    echo -n "Path to _per_ host local scratch directory [${sql_local_scratch_dir}]: "
    local lsd=; read lsd
    if [[ "${lsd}" != "" ]]; then
        sql_local_scratch_dir="${lsd/%\//}/"
    fi
    mkdir -p "${sql_local_scratch_dir}"
    if [[ $? != 0 ]]; then
        echo "Failed to create directory ${sql_local_scratch_dir}"
        exit 5;
    fi

    local sql_global_scratch_dir="${HOME}/data/"
    echo -n "Path to global scratch (data, tables...) directory [${sql_global_scratch_dir}]: "
    local gsd=; read gsd
    if [[ "${gsd}" != "" ]]; then
        sql_global_scratch_dir="${gsd/%\//}/"
    fi
    mkdir -p "${sql_global_scratch_dir}"
    if [[ $? != 0 ]]; then
        echo "Failed to create directory ${sql_global_scratch_dir}"
        exit 6;
    fi

    local sql_obj_storage_dirs="${HOME}/store/"
    echo -n "Path to object storage directory [${sql_obj_storage_dirs}]: "
    local osd=; read osd
    if [[ "${osd}" != "" ]]; then
        sql_obj_storage_dirs="${osd/%\//}/"
    fi
    mkdir -p "${sql_obj_storage_dirs}"
    chmod g+w "${sql_obj_storage_dirs}"
    if [[ $? != 0 ]]; then
        echo "Failed to create directory ${sql_obj_storage_dirs}"
        exit 7;
    fi

    local sql_internalWWW="http://`uname -n`/"
    echo -n "Internal web server URL [${sql_internalWWW}]: "
    local iwww=; read iwww
    if [[ "${iwww}" != "" ]]; then
        sql_internalWWW="${iwww/%\//}/"
    fi

    echo -n "Create an admin account to be used to activate other accounts? [Y/n]"
    yn=; read yn
    if [[ "${yn}" == "y" || ${yn} == "" ]]; then
        local useremail=
        while [[ "${useremail}" == "" ]]; do
            echo -n "Enter admin user email [hiveadmin]: "
            read useremail
            if [[ "${useremail}" == "" ]]; then
                useremail="hiveadmin"
            fi
        done
        echo -e "First time logging in use '${useremail}' with password 'HiveUser1'\nPress Enter key to continue..."
        read xx
    fi

    local sql_encoder_hex=$(python -c "import os; print os.urandom(64).encode('hex').upper()")
    local sql_session_b64=$(python -c "import os; print ''.join(os.urandom(64).encode('base64').split())")

    export HIVECFG_SQL=/tmp/hivedeploy$$.sql
    cat <<EOF >${HIVECFG_SQL}

SET @head_hosts='${sql_head_hosts}';
SET @site_hosts='${sql_site_hosts}';
SET @bin_dir='${sql_bin_dir}';
SET @local_scratch_dir='${sql_local_scratch_dir}';
SET @global_scratch_dir='${sql_global_scratch_dir}';
SET @obj_storage_dirs='${sql_obj_storage_dirs}';
SET @internalWWW='${sql_internalWWW}';
SET @encoder_hex='${sql_encoder_hex}';
SET @session_bin='${sql_session_b64}';

source ${HIVESQL};

INSERT INTO UPUser (is_active_fg, is_admin_fg, is_email_valid_fg, type, email, pswd, first_name, last_name, createTm)
    VALUES (1, 0, 0, 'group', '', '', '/Projects/Team/', 'Team', CURRENT_TIMESTAMP);
INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath)
    VALUES (LAST_INSERT_ID(), -1, 1, '/Projects/Team/');

INSERT INTO UPUser (is_active_fg, is_admin_fg, is_email_valid_fg, type, email, pswd, first_name, last_name, createTm)
    VALUES (1, 1, 1, 'user', '${useremail}', 'pbkdf2_sha256\$100000\$NY13LgwIJo4j\$QKaMiT3Wfek3JxOqURlFwJvPOX1ucw0axCsuqfFMoME=', 'HIVE', 'User', CURRENT_TIMESTAMP);
INSERT INTO UPGroup (userID, flags, is_active_fg, groupPath) VALUES
    (LAST_INSERT_ID(), -1, 1, '/everyone/users/${useremail}'),
    (LAST_INSERT_ID(),  0, 1, '/Projects/Team/${useremail}');

UPDATE QPSvc set capacity = 1, maxLoops = 3, sleepTime = 5000 WHERE sleepTime >= 5000 AND maxLoops >= 3;

EOF
    echo -n "Database name? [${Hqdbdb}] "
    local DB=; read DB
    if [[ "${DB}" != "" ]]; then
        Hqdbdb="${DB}"
    fi
    echo -n "Database user? [${Hqdbuser}] "
    local DBUSER=; read DBUSER
    if [[ "${DBUSER}" != "" ]]; then
        Hqdbuser="${DBUSER}"
    fi
    echo -n "Database password? [${Hqdbpswd}] "
    local DBP=; read DBP
    if [[ "${DBP}" != "" ]]; then
        Hqdbpswd="${DBP}"
    fi
    echo -n "Database host? [${Hqdbip4} or localhost] "
    local DBH=; read DBH
    if [[ "${DBH}" != "" ]]; then
        Hqdbip4="${DBH}"
    fi
    echo -n "Database port? [${Hqdbport}] "
    local DBPORT=; read DBPORT
    if [[ "${DBPORT}" != "" ]]; then
        Hqdbport="${DBPORT}"
    fi

    cd `dirname ${DBC}`
    echo "Running ${DBC}..."
    ${DBC} "${Hqdbdb}" "${Hqdbuser}" "${Hqdbpswd}" "${Hqdbip4}" "${Hqdbport}"
    if [[ $? != 0 ]]; then
        echo "Database setup failed"
        exit 8;
    fi
    cd - 2>&1 1>/dev/null

    hive_setup_cfg
    hive_setup_qappcfg "${HOME}"
    # setup env for make to work properly
    if [[ -s vlib/qlib/hive_bash_rc ]]; then
        . vlib/qlib/hive_bash_rc
    else
        echo "Warning: vlib/qlib/hive_bash_rc is missing, install target might fail"
    fi
    rm -f ${HIVECFG_SQL}
    export HIVECFG_SQL=

    echo "Installing binaries for ${USER} at ${HOME}"
    make install
    if [[ $? != 0 ]]; then
        echo "Installation failed"
        exit 9;
    fi
    dd=`echo "SELECT val FROM QPCfg WHERE val LIKE '/%/' AND cleanUpDays IS NOT NULL" | mysql -u ${Hqdbuser} --password=${Hqdbpswd} ${Hqdbdb}`
    for d in ${dd}; do
        if [[ "${d}" != "val" ]]; then
            mkdir -p "${d}"
            if [[ $? != 0 ]]; then
                echo "Failed to create directory ${d}"
            else
                chmod g+w "${d}"
            fi
        fi
    done
}

hive_setup_local()
{
    hive_setup_qappcfg "${HOME}"
    local bin_dir=`"${qapppath}" -configGet qm.resourceRoot`
    # ` #syntax highlight in midnight commander glitches without this
    if [[ ${bin_dir} == "" ]]; then
        echo "Cannot read config resourceRoot with qapp"
        exit 7
    fi
    "${qapppath}" -init ${bin_dir}/
    if [[ $? != 0 ]]; then
        echo "Binaries installation failed: $!"
        exit 8;
    fi
    cp -pv ${bin_dir}/qapp.osLinux ${bin_dir}/qapp

    echo -n "Setup crontab with \"maintain 1\"? [y/N]:"
    local yn=; read yn
    if [[ "${yn}" == "y" ]]; then
        echo "* * * * * ${bin_dir}/qpstart.sh.osLinux \" maintain 1\"" >/tmp/hivecron$$.tmp
    else
        echo "* * * * * ${bin_dir}/qpstart.sh.osLinux" >/tmp/hivecron$$.tmp
    fi
    crontab /tmp/hivecron$$.tmp
    rm -f /tmp/hivecron$$.tmp

    if [[ -s ~/.hive_bash_rc ]]; then
        echo -e "\n. ~/.hive_bash_rc\n" >> ~/.bashrc
    fi
}


hive_setup_www()
{
    if [[ ! -d hive ]]; then
        echo "Cannot find HIVE installation directory 'hive'"
        exit 51
    fi
    cd hive
    if [[ $? != 0 ]]; then
        echo "Cannot change directory to hive"
        exit 52
    fi
    local wwwroot='/var/www/html/'
    echo -n "www root location? [${wwwroot}]"
    wwr=; read wwr
    if [[ "${wwr}" != "" ]]; then
        wwwroot="${wwr}"
    fi
    if [[ -s hive.ver ]]; then
        make www "WDIR=${wwwroot}/" VER=`cat hive.ver`
    else
        make www "WDIR=${wwwroot}/"
    fi
    if [[ $? != 0 ]]; then
        echo "Web server HIVE setup failed"
        exit 53
    fi
    hive_setup_qappcfg "${wwwroot}"
}

hive_setup_data()
{
    local d1=`df -BG /tmp    | tail -1 | awk '{print substr($4,1,length($4)-1)}'`
    local d2=`df -BG ${HOME} | tail -1 | awk '{print substr($4,1,length($4)-1)}'`
    local rpp="/tmp"
    if [[ $d1 -lt $d2 ]]; then
        rpp="${HOME}"
    fi
    d2=`df -BG ${rpp} | tail -1 | awk '{print substr($4,1,length($4)-1)}'`
    if [[ $d2 -lt 500 ]]; then
        echo "No enough space on ${rpp}"
        exit 60
    fi
    for hp in ncbiTaxonomy NCBI-NT-latest human-genome-latest; do
        local pp="${rpp}/${hp}"
        if [[ ! -d ${pp} ]]; then
           mkdir ${pp}
            if [[ $? != 0 ]]; then
                echo "Cannot save file in /tmp, no space?"
                exit 61
            fi
        fi
        cd "${pp}"
        wget "https://hive.biochemistry.gwu.edu/static/deployment/${hp}.hivepack"
        if [[ $? != 0 ]]; then
            echo "${hp} download failed"
        else
            echo "Processing ${hp}..."
            unzip -o "${hp}.hivepack"
            if [[ $? != 0 ]]; then
                echo "Downloaded file is damaged, skipped"
            else
                for p in *.prop; do
                    ${qapppath} -user qapp -prop "${p}"
                done
            fi
        fi
        cd -
        rm -rf ${pp}
    done
}

lsb_release -a 2>/dev/null | grep -i ubuntu 1>/dev/null
if [[ $? == 0 ]]; then
    OSxx='ubuntu'
else
    OSxx='rhel'
fi

Huser=hive
Huid=
Hgrp=hive
Hgid=
Hpswd1=
Hqdbdb=hive
Hqdbip4=localhost
Hqdbport=3306
Hqdbuser=hive
Hqdbpswd=hive123

touch ${Hdeplcfg} 2>/dev/null
if [[ $? != 0 ]]; then
    Hdeplcfg=~/hive_deploy.cfg
fi
if [[ -s ${Hdeplcfg} ]]; then
    . ${Hdeplcfg}
fi

this_script=`pwd`/`find . -name $(basename $0)`
qapppath=`pwd`/`find $(dirname this_script) -name qapp | head -1`
if [[ "${qapppath}" == "" ]]; then
    qapppath=`pwd`/`find $(dirname this_script)/../.. -name qapp | head -1`
    if [[ "${qapppath}" == "" ]]; then
        echo "qapp not found in `pwd`"
        exit 26
    fi
fi

if [[ "$1" != "" ]]; then
    if [[ "${USER}" == "root" ]]; then
        echo "HIVE setup should be executed from ${Huser} account, not root, bye"
        exit 23
    fi
    if [[ "${Huser}" == "" ]]; then
        Huser="${USER}"
        Hgrp="${USER}"
    fi
    if [[ "$1" == "initial" ]]; then
        hive_setup_hive
    elif [[ "$1" == "local" ]]; then
        hive_setup_local
    elif [[ "$1" == "www" ]]; then
        hive_setup_www
    elif [[ "$1" == "data" ]]; then
        hive_setup_data
    else
        echo "Unknown command $1 to execute from ${Huser} account"
        exit 24
    fi
    exit 0
elif [[ "${USER}" == "root" ]]; then
    if [[ ! -d hive || ! -d vlib ]]; then
        cd ../..
        if [[ ! -d hive || ! -d vlib ]]; then
            echo "Package not found"
            exit 25
        fi
        # allow accounts access to install directories
        xxpwd=`pwd`
        while [[ true ]]; do
            find . -prune -printf '%m\n' | grep '..5' >/dev/null
            if [[ $? != 0 ]]; then
                echo -n "Package location is not read/write accessible by other, continue anyway? [y/N]"
                yn=; read yn
                if [[ "${yn}" == "y" ]]; then
                    break
                else
                    exit 27
                fi
            fi
            cd ..
            if [[ "`pwd`" == "/" ]]; then
                break
            fi
        done
        cd "${xxpwd}"
        echo -n "Please wait..."
        find . -type d -exec chmod o+rx {} \; 2>&1 1>/dev/null
        find . -type f -exec chmod o+r {} \; 2>&1 1>/dev/null
        echo -n -e "\r"
    fi
    echo -n "Create HIVE user account and group? [y/N]:"
    yn=; read yn
    if [[ "${yn}" == "y" ]]; then
        echo -n "HIVE account name [${Huser}]: "
        hu=; read hu
        if [[ "${hu}" != "" ]]; then
            Huser="${hu}"
        fi
        echo -n "HIVE account id (optional) [${Huid}]: "
        hu=; read hu
        if [[ "${hu}" != "" ]]; then
            Huid="${hu}"
        fi
        echo -n "HIVE group name [${Hgrp}]: "
        hg=; read hg
        if [[ "${hg}" != "" ]]; then
            Hgrp="${hg}"
        fi
        while [[ true ]]; do
            echo -n "HIVE group id (optional or >1000) [${Hgid}]: "
            hg=; read hg
            if [[ ${hg} -gt 1000 ]]; then
                Hgid=${hg}
                break
            elif [[ "${hg}" == "" ]]; then
                break
            fi
        done
        hkeeppwd=y
        if [[ "${Hpswd1}" != "" ]]; then
            echo -n "Keep password? [Y/n]:"
            xx=; read xx
            if [[ "${xx}" != "" ]]; then
                xkeeppwd="${xx}"
            fi
        fi
        if [[ "${Hpswd1}" == "" || "${hkeeppwd}" != "y" ]]; then
            while [[ true ]]; do
                echo -n "HIVE account password:"
                Hpswd1=; read -s Hpswd1
                echo ""
                echo -n "HIVE account password repeat:"
                Hpswd2=; read -s Hpswd2
                echo ""
                if [[ "${Hpswd1}" == "" ]]; then
                    echo "Password cannot be empty"
                elif [[ "${Hpswd1}" != "${Hpswd2}" ]]; then
                    echo "Passwords do not match"
                else
                    break
                fi
            done
        fi
        hive_setup_account "${Huser}" "${Huid}" "${Hgrp}" "${Hgid}" "${Hpswd1}"
    else
        echo "Will continue with HIVE account ${Huser}:${Hgrp}"
    fi
    echo -n "Setup prerequisites? [y/N]:"
    yn=; read yn
    if [[ "${yn}" == "y" ]]; then
        if [[ "${OSxx}" == "ubuntu" ]]; then
            apt-get --assume-yes install \
                mc zip unzip xz-utils wget curl htop dstat netcat \
                socat xfsprogs nfs-common system-storage-manager lvm2 \
                perl psmisc iotop bzip2
        else
            yum install --assumeyes \
                mc zip unzip xz wget curl iptables-services htop dstat nc \
                socat bind-utils xfsprogs nfs-utils system-storage-manager lvm2 \
                perl psmisc yum-plugin-remove-with-leaves iotop bzip2 perl-Env
        fi
        if [[ $? != 0 ]]; then
            echo "Prerequisites setup failed"
            exit 28
        fi
        if [[ "${OSxx}" == "ubuntu" ]]; then
            systemctl disable ufw
            systemctl stop ufw
        else
            systemctl disable firewalld
            systemctl stop firewalld
            sed --in-place=.hive '/SELINUX=/d' /etc/selinux/config
            echo "SELINUX=disabled" >> /etc/selinux/config
            setenforce 0
        fi

        let fmax1=`sysctl -a | grep file-max | awk '{r=match($0,/([0-9]+)/);print substr($0,r)}'`
        if [[ ${fmax1} < 65536 ]]; then
            echo "Change system setting for maximum open file to 65536 or more in /etc/sysctl.conf"
            echo "# insert or update line:"
            echo "fs.file-max = 100000"
            echo "Press Enter key to edit /etc/sysctl.conf..."
            read xx
            vi /etc/sysctl.conf
        fi
        let fmax2=`ulimit -a | grep 'open files' | awk '{r=match($0,/([0-9]+)/);print substr($0,r)}'`
        if [[ ${fmax2} < 65536 ]]; then
            echo "Change user setting for maximum open file to 65536 or more in /etc/security/limits.conf"
            echo "# insert or update lines:"
            echo "* hard nofile 100000"
            echo "* soft nofile 100000"
            echo "Press Enter key to edit /etc/security/limits.conf..."
            read xx
            vi /etc/security/limits.conf
        fi
        echo -n "Update system? [y/N]:"
        yn=; read yn
        if [[ "${yn}" == "y" ]]; then
            if [[ "${OSxx}" == "ubuntu" ]]; then
                apt-get --assume-yes upgrade
            else
                yum update --assumeyes
            fi
            if [[ $? != 0 ]]; then
                echo "Update failed"
                exit 29
            fi
        fi
        echo -n "Reboot (changes above always require a reboot!)? [y/N]:"
        yn=; read yn
        if [[ "${yn}" == "y" ]]; then
            reboot
        fi
    fi
    echo -n "Install mysql/mariadb server? [y/N]:"
    yn=; read yn
    if [[ "${yn}" == "y" ]]; then
        if [[ "${OSxx}" == "ubuntu" ]]; then
            pkg1=mysql
        else
            pkg1=mariadb
        fi
        echo -n "Type Linux package name mariadb or mysql [${pkg1}]: "
        pkg=; read pkg
        if [[ "${pkg}" == "" ]]; then
            pkg="${pkg1}"
        fi
        hive_setup_mysql ${pkg}
    fi
    echo -n "Install Apache (httpd)? [y/N]:"
    yn=; read yn
    if [[ "${yn}" == "y" ]]; then
        hive_setup_httpd "${Huser}" "${Hgrp}"
    fi
else
    echo -n "Run this script as root to setup HIVE account, web and db server, etc"
    echo "Usage: ONLY when running from HIVE account, not root"
    echo "    initial - create DB, load config, loda binaries"
    echo "    local   - init bin directory, setup crontab"
    echo "    www     - install CGIs, pages, images, etc to www, should be done on web server"
    echo "    data    - download and install basic NGS datasets: taxonomy, NCBI NT, Human Genome"
    exit 0
fi

echo -n "Initial HIVE setup? [y/N]"
yn=; read yn
if [[ "${yn}" == "y" ]]; then
    sudo -H -u ${Huser} ${this_script} "initial"
    if [[ $? != 0 ]]; then
        exit 51
    fi
fi
echo -n "Host bin and crontab setup? [y/N]"
yn=; read yn
if [[ "${yn}" == "y" ]]; then
    sudo -H -u ${Huser} ${this_script} "local"
    if [[ $? != 0 ]]; then
        exit 52
    fi
fi
echo -n "Web server HIVE setup? [y/N]"
yn=; read yn
if [[ "${yn}" == "y" ]]; then
    sudo -H -u ${Huser} ${this_script} "www"
    if [[ $? != 0 ]]; then
        exit 53
    fi
fi
echo -n "Download and install basic NGS datasets: taxonomy, NCBI NT, Human Genome? [y/N]"
yn=; read yn
if [[ "${yn}" == "y" ]]; then
    sudo -H -u ${Huser} ${this_script} "data"
    if [[ $? != 0 ]]; then
        exit 54
    fi
fi
