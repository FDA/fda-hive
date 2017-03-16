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
/*
 * this script is automatically generated
 */
SET character_set_connection = utf8;
SET character_set_client = utf8;
SET character_set_results = utf8;
SET collation_connection = utf8_unicode_ci;
source cfgIns.sql;
source dataIns.sql;
source dbCln.sql;
source dbInfo.sql;
source grpIns.sql;
source grpSubmit.sql;
source jobIns.sql;
source recoverReqs.sql;
source reqGrab.sql;
source reqPeekOrder.sql;
source reqSubmit.sql;
source resourceIns.sql;
source sp_grp_progress.sql;
source sp_grp_submit.sql;
source sp_job_register.sql;
source sp_log_get.sql;
source sp_log_set.sql;
source sp_misc_purge.sql;
source sp_obj_by_time.sql;
source sp_obj_cast.sql;
source sp_obj_create.sql;
source sp_obj_create_new_in_domain.sql;
source sp_obj_create_v2.sql;
source sp_obj_delete.sql;
source sp_obj_delete_v2.sql;
source sp_obj_erase.sql;
source sp_obj_erase_v2.sql;
source sp_obj_get_v3.sql;
source sp_obj_get_v4_1.sql;
source sp_obj_perm_all_v2.sql;
source sp_obj_perm_copy.sql;
source sp_obj_perm_copy_v2.sql;
source sp_obj_perm_scanf.sql;
source sp_obj_perm_set.sql;
source sp_obj_perm_set_v2.sql;
source sp_obj_prop.sql;
source sp_obj_prop_del.sql;
source sp_obj_prop_del_v2.sql;
source sp_obj_prop_get.sql;
source sp_obj_prop_get_v1_1.sql;
source sp_obj_prop_get_v2.sql;
source sp_obj_prop_get_v2_1.sql;
source sp_obj_prop_init.sql;
source sp_obj_prop_init_v2.sql;
source sp_obj_prop_list.sql;
source sp_obj_prop_list_v1_1.sql;
source sp_obj_prop_list_v2.sql;
source sp_obj_prop_list_v2_1.sql;
source sp_obj_prop_set.sql;
source sp_obj_prop_set_v2.sql;
source sp_obj_prop_set_v3.sql;
source sp_obj_prop_v1_1.sql;
source sp_obj_prop_v2_1.sql;
source sp_obj_prop_v2_2.sql;
source sp_permission_check_v2.sql;
source sp_registerHostIP.sql;
source sp_registerHostIP_v2.sql;
source sp_req_by_time.sql;
source sp_req_data_set.sql;
source sp_req_grab.sql;
source sp_req_lock.sql;
source sp_req_peek_order.sql;
source sp_req_recover.sql;
source sp_req_submit.sql;
source sp_req_unlock.sql;
source sp_servicePurgeOld.sql;
source sp_svc_purge_old.sql;
source sp_svc_purge_v2.sql;
source sp_sys_capacity.sql;
source sp_sys_log.sql;
source sp_type_get_v5.sql;
source sp_type_get_v6.sql;
source sp_user_audit.sql;
source sp_user_init.sql;
source sp_user_list.sql;
source sp_user_login.sql;
source sp_user_logout.sql;
source sp_user_session_v2.sql;
source sp_user_usage_objs.sql;
