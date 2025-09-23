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
SELECT 'Creating stored procedure cfgIns.sql' AS Message;
source cfgIns.sql;
SELECT 'Creating stored procedure dataIns.sql' AS Message;
source dataIns.sql;
SELECT 'Creating stored procedure dbCln.sql' AS Message;
source dbCln.sql;
SELECT 'Creating stored procedure dbInfo.sql' AS Message;
source dbInfo.sql;
SELECT 'Creating stored procedure grpIns.sql' AS Message;
source grpIns.sql;
SELECT 'Creating stored procedure grpSubmit_vBES.sql' AS Message;
source grpSubmit_vBES.sql;
SELECT 'Creating stored procedure infReq.sql' AS Message;
source infReq.sql;
SELECT 'Creating stored procedure infReqUser.sql' AS Message;
source infReqUser.sql;
SELECT 'Creating stored procedure jobIns.sql' AS Message;
source jobIns.sql;
SELECT 'Creating stored procedure recoverReqs.sql' AS Message;
source recoverReqs.sql;
SELECT 'Creating stored procedure reqGrab.sql' AS Message;
source reqGrab.sql;
SELECT 'Creating stored procedure reqPeekOrder.sql' AS Message;
source reqPeekOrder.sql;
SELECT 'Creating stored procedure reqSubmit.sql' AS Message;
source reqSubmit.sql;
SELECT 'Creating stored procedure resourceIns.sql' AS Message;
source resourceIns.sql;
SELECT 'Creating stored procedure sp_grp_get.sql' AS Message;
source sp_grp_get.sql;
SELECT 'Creating stored procedure sp_grp_get_log.sql' AS Message;
source sp_grp_get_log.sql;
SELECT 'Creating stored procedure sp_grp_progress.sql' AS Message;
source sp_grp_progress.sql;
SELECT 'Creating stored procedure sp_grp_submit_v2.sql' AS Message;
source sp_grp_submit_v2.sql;
SELECT 'Creating stored procedure sp_grp_submit_vBES.sql' AS Message;
source sp_grp_submit_vBES.sql;
SELECT 'Creating stored procedure sp_job_register.sql' AS Message;
source sp_job_register.sql;
SELECT 'Creating stored procedure sp_log_get.sql' AS Message;
source sp_log_get.sql;
SELECT 'Creating stored procedure sp_log_set.sql' AS Message;
source sp_log_set.sql;
SELECT 'Creating stored procedure sp_misc_purge.sql' AS Message;
source sp_misc_purge.sql;
SELECT 'Creating stored procedure sp_obj_by_time.sql' AS Message;
source sp_obj_by_time.sql;
SELECT 'Creating stored procedure sp_obj_cast.sql' AS Message;
source sp_obj_cast.sql;
SELECT 'Creating stored procedure sp_obj_create_new_in_domain.sql' AS Message;
source sp_obj_create_new_in_domain.sql;
SELECT 'Creating stored procedure sp_obj_create_v2.sql' AS Message;
source sp_obj_create_v2.sql;
SELECT 'Creating stored procedure sp_obj_delete_v2.sql' AS Message;
source sp_obj_delete_v2.sql;
SELECT 'Creating stored procedure sp_obj_erase_v2.sql' AS Message;
source sp_obj_erase_v2.sql;
SELECT 'Creating stored procedure sp_obj_get_v5.sql' AS Message;
source sp_obj_get_v5.sql;
SELECT 'Creating stored procedure sp_obj_perm_all_v2.sql' AS Message;
source sp_obj_perm_all_v2.sql;
SELECT 'Creating stored procedure sp_obj_perm_copy_v2.sql' AS Message;
source sp_obj_perm_copy_v2.sql;
SELECT 'Creating stored procedure sp_obj_perm_flip.sql' AS Message;
source sp_obj_perm_flip.sql;
SELECT 'Creating stored procedure sp_obj_perm_scanf.sql' AS Message;
source sp_obj_perm_scanf.sql;
SELECT 'Creating stored procedure sp_obj_perm_set_v2.sql' AS Message;
source sp_obj_perm_set_v2.sql;
SELECT 'Creating stored procedure sp_obj_prop.sql' AS Message;
source sp_obj_prop.sql;
SELECT 'Creating stored procedure sp_obj_prop_del_v2.sql' AS Message;
source sp_obj_prop_del_v2.sql;
SELECT 'Creating stored procedure sp_obj_prop_get_v2_1.sql' AS Message;
source sp_obj_prop_get_v2_1.sql;
SELECT 'Creating stored procedure sp_obj_prop_init_v2.sql' AS Message;
source sp_obj_prop_init_v2.sql;
SELECT 'Creating stored procedure sp_obj_prop_list_v2.sql' AS Message;
source sp_obj_prop_list_v2.sql;
SELECT 'Creating stored procedure sp_obj_prop_list_v2_1.sql' AS Message;
source sp_obj_prop_list_v2_1.sql;
SELECT 'Creating stored procedure sp_obj_prop_set_v3.sql' AS Message;
source sp_obj_prop_set_v3.sql;
SELECT 'Creating stored procedure sp_obj_prop_v2_2.sql' AS Message;
source sp_obj_prop_v2_2.sql;
SELECT 'Creating stored procedure sp_permission_check_v2.sql' AS Message;
source sp_permission_check_v2.sql;
SELECT 'Creating stored procedure sp_registerHostIP_v2.sql' AS Message;
source sp_registerHostIP_v2.sql;
SELECT 'Creating stored procedure sp_req_by_time.sql' AS Message;
source sp_req_by_time.sql;
SELECT 'Creating stored procedure sp_req_data_set.sql' AS Message;
source sp_req_data_set.sql;
SELECT 'Creating stored procedure sp_req_grab.sql' AS Message;
source sp_req_grab.sql;
SELECT 'Creating stored procedure sp_req_lock.sql' AS Message;
source sp_req_lock.sql;
SELECT 'Creating stored procedure sp_req_peek_order.sql' AS Message;
source sp_req_peek_order.sql;
SELECT 'Creating stored procedure sp_req_recover.sql' AS Message;
source sp_req_recover.sql;
SELECT 'Creating stored procedure sp_req_submit.sql' AS Message;
source sp_req_submit.sql;
SELECT 'Creating stored procedure sp_req_unlock.sql' AS Message;
source sp_req_unlock.sql;
SELECT 'Creating stored procedure sp_servicePurgeOld.sql' AS Message;
source sp_servicePurgeOld.sql;
SELECT 'Creating stored procedure sp_svc_purge_v2.sql' AS Message;
source sp_svc_purge_v2.sql;
SELECT 'Creating stored procedure sp_sys_capacity.sql' AS Message;
source sp_sys_capacity.sql;
SELECT 'Creating stored procedure sp_sys_log.sql' AS Message;
source sp_sys_log.sql;
SELECT 'Creating stored procedure sp_type_get_latest_mtime.sql' AS Message;
source sp_type_get_latest_mtime.sql;
SELECT 'Creating stored procedure sp_type_get_v6.sql' AS Message;
source sp_type_get_v6.sql;
SELECT 'Creating stored procedure sp_user_audit.sql' AS Message;
source sp_user_audit.sql;
SELECT 'Creating stored procedure sp_user_init.sql' AS Message;
source sp_user_init.sql;
SELECT 'Creating stored procedure sp_user_list.sql' AS Message;
source sp_user_list.sql;
SELECT 'Creating stored procedure sp_user_login.sql' AS Message;
source sp_user_login.sql;
SELECT 'Creating stored procedure sp_user_logout.sql' AS Message;
source sp_user_logout.sql;
SELECT 'Creating stored procedure sp_user_session_v2.sql' AS Message;
source sp_user_session_v2.sql;
SELECT 'Creating stored procedure sp_user_usage_objs.sql' AS Message;
source sp_user_usage_objs.sql;
