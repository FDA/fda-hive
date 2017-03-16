
DROP PROCEDURE IF EXISTS `sp_registerHostIP`;

DELIMITER //

CREATE PROCEDURE `sp_registerHostIP`( in host varchar (128), in addr varchar(16), in sys varchar(16) )
    MODIFIES SQL DATA
begin
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
        declare hostExists int;

        if (host is not null and length(host)>0) then

                if (addr is not null) then
                    delete from QPHosts where ((ip4=addr)and(name!=host));
                end if;

        select count(name) into hostExists from QPHosts where name=host;

            if (hostExists is not null) then
                        case
                when (hostExists=1) then
                update QPHosts set ip4=addr, htype=sys, mdate=now() where name=host;
            when (hostExists=0) then
                insert into QPHosts (name,ip4,htype,enabled) values (host,addr,sys,true);
                when (hostExists>1) then
           delete from QPHosts where name = host;
           insert into QPHosts (name, ip4, htype, enabled) values (host, addr, sys, true);
                end case;
        end if;

        end if;
end //
DELIMITER ;
