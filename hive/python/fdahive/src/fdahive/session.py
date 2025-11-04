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

import requests
import os
import json
import time
import urllib.parse
import concurrent.futures
import re
import stat
import getpass

class _HiveException(Exception):
    """Root class for exceptions raised by this module"""
    pass

class _BadResponseError(_HiveException):
    """
    Exception raised when response from response from server is malformed.
    Examples include the response json missing a required key, the response csv having the wrong number of columns, etc.
    """
    pass

class _HiveComputationError(_HiveException):
    """
    Excption raised when a HIVE computation does not finish successfully.
    For example, if the upload processor fails
    """
    pass

class _HiveLoggedOutError(_HiveException):
    """
    Exception raised when a HiveSession is unexpectedly logged out
    """
    pass


class _FileToUp:
    def __init__(self, pathname, hive_pathname, id):
        self.pathname = pathname
        self.hive_pathname = hive_pathname
        self.size = os.path.getsize(pathname)
        self.last_modified = os.path.getmtime(pathname)
        self.id = id
        self.total_parts = 0 # we will update when we split into parts

    def __repr__(self):
        return '{} {}'.format(self.file_to_up.pathname, self.size)

    def __str__(self):
        return '{} {}'.format(self.file_to_up.pathname, self.size)        

        
class _FilePart:
    def __init__(self, file_to_up, start, end):
        self.file_to_up = file_to_up
        self.start = start
        self.end = end

    def __repr__(self):
        return '{} {}-{}'.format(self.file_to_up.pathname, self.start, self.end)

    def __str__(self):
        return '{} {}-{}'.format(self.file_to_up.pathname, self.start, self.end)        

    def header(self):
        h = dict()
        h['start'] = self.start
        h['end'] = self.end
        h['path'] = self.file_to_up.hive_pathname
        h['size'] = self.file_to_up.size
        h['lastModified'] = self.file_to_up.last_modified
        h['id'] = self.file_to_up.id
        h['totalChunks'] = self.file_to_up.total_parts
        return h

    def open(self):
        self.file_obj = open(self.file_to_up.pathname, 'rb')
        self.pos = 0

    def close(self):
        self.file_obj.close()

    def read(self, size=None):
        max_size = self.end - self.start - self.pos
        if size is None:
            size = max_size
        else:
            size = min(size, max_size)
        self.file_obj.seek(self.start + self.pos)
        self.pos += size
        return self.file_obj.read(size)

def _gen_file_parts(files, max_chunk_size):
    parts = []
    cur_chunk_size = max_chunk_size 
    for file in files:
        cur_file_pos = 0
        cur_file_parts = 0
        while cur_file_pos < file.size:
            if cur_chunk_size == max_chunk_size:
                cur_chunk = []
                cur_chunk_size = 0
                parts.append(cur_chunk)
            part_size = min(file.size - cur_file_pos, max_chunk_size - cur_chunk_size)
            cur_chunk.append(_FilePart(file, cur_file_pos, cur_file_pos + part_size))
            cur_chunk_size += part_size
            cur_file_pos += part_size
            cur_file_parts += 1
        file.total_parts = cur_file_parts
    return parts



class HiveSession:
    """Main class used to interact with a HIVE instance.
    Enables logging in, uploading files, downloading files, and retreiving information on HIVE objects

    Example Usage:
    >>> import fdahive
    >>> hs = fdahive.HiveSession('https://dnahive.fda.gov')
    >>> hs.login('User.Name@email.com', 'token')
    >>> hs.download([1234, 7768, 9987])
    >>> hs.upload(['file1.txt', 'file2.txt'])
    >>> hs.logout()
    """

    def __init__(self, url, ssl_verify=True):
        """Initialization method. Speficies the HIVE instance to connect to

        Arguments:
        url - url of the HIVE instance to connect to
        ssl_verify - used to control SSL verification. If set to a string, the library will search for SSL certificates at the given path. If set to False,
        no SSL veification will be performed. This argument should usually be kept at its default value and only be modified by advanced users.
        """
        self.url = url
        self.dna_cgi = '{}/dna.cgi'.format(self.url)
        self.upload_cgi = '{}/upload.cgi'.format(self.url)
        self.session = requests.Session()
        self.logged_in = False
        if ssl_verify != True:
            self.session.verify = ssl_verify
            if ssl_verify == False:
                import urllib3
                urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

    @property
    def project(self):
        return self._project

    @project.setter
    def project(self, id):
        self._project = id
        self.session.cookies.update({'projectID': id})

    @project.deleter
    def project(self):
        del self._project
        self.session.cookies.update({'projectID': None})

    def _logged_in_check(self):
        if self.session.cookies['userName'] == 'Guest':
            raise _HiveLoggedOutError('User is logged out. Must be logged in')
    
    def _objqry(self, qry):
        params = dict()
        params['cmdr'] = 'objQry'
        params['qry'] = qry
        req = self.session.post(url=self.dna_cgi, data=params)
        req.raise_for_status()
        self._logged_in_check()
        return req.text

    def _objfile(self, obj, filename=None):
        params = dict()
        params['cmdr'] = 'objFile'
        params['ids'] = obj
        if filename:
            params['filename'] = filename
        
        req = self.session.get(url=self.dna_cgi, params=params, stream=True)
        req.raise_for_status()
        self._logged_in_check()
        return req
    
    def _check_obj_types(self, objs, types):
        qry = 'result=[];{}.foreach({{o = this;types={}.filter({{(o as obj).objoftype(this)}});result.push(types);}});return result;'.format(objs, types)
        result = self._objqry(qry)
        try:
            result = json.loads(result)
            assert type(result) is list
            assert len(result) == len(objs)
            return result
        except (ValueError, AssertionError):
            raise _BadResponseError('Unable to understand type query results')

    def _obj_file_list(self, obj):
        params = dict()
        params['files'] = '*.*'
        params['mode'] = 'json'
        params['prop'] = 'none'
        params['ids'] = obj
        params['cmdr'] = 'propget'

        req = self.session.get(url=self.dna_cgi, params=params)
        req.raise_for_status()
        self._logged_in_check()

        try:
            return req.json()['_file']
        except (ValueError, KeyError):
            raise _BadResponseError('Unable to read list of files from {} file list query'.format(obj))

    def _download_req(self, req, dst_dir, obj, overwrite, custom_filename=None, chunk_size=10485760):
        if custom_filename:
            filename = custom_filename
        else:
            given_name = None
            if 'Content-Disposition' in list(req.headers.keys()):
                given_name = re.search('filename=(.+)', req.headers['Content-Disposition'])
            if given_name:
                filename = given_name.group(1)
            else:
                filename = 'o{}'.format(obj)
        dst_path = os.path.join(dst_dir, filename)

        if overwrite is False and os.path.exists(dst_path):
            raise FileExistsError('File {} exists and overwrite set to False'.format(dst_path))
        os.makedirs(os.path.dirname(dst_path), exist_ok=True)

        with open(dst_path, "wb") as dst_f:
            for chunk in req.iter_content(chunk_size=chunk_size):
                dst_f.write(chunk) # what should we set the chunk size to???
        return dst_path

    def _alsam(self, obj):
        params = dict()
        params['cmdr'] = 'alSam'
        params['objs'] = obj
        params['down'] = 1
        params['useOriginalID'] = 1
        params['qty'] = -1

        req = self.session.get(url=self.dna_cgi, params=params, stream=True)
        req.raise_for_status()
        self._logged_in_check()
        return req

    def _alfasta(self, obj):
        params = dict()
        params['cmdr'] = 'alFasta'
        params['objs'] = obj
        params['wrap'] = 100
        params['cnt'] = 0
        params['info'] = 1
        params['multiple'] = 1
        params['mySubID'] = 1
        params['down'] = 1

        req = self.session.get(url=self.dna_cgi, params=params, stream=True)
        req.raise_for_status()
        self._logged_in_check()
        return req

    def _alconsensus(self, obj):
        params = dict()
        params['cmdr'] = 'alConsensus'
        params['objs'] = obj
        params['wrap'] = 100
        params['multiple'] = 1
        params['down'] = 1

        req = self.session.get(url=self.dna_cgi, params=params, stream=True)
        req.raise_for_status()
        self._logged_in_check()
        return req

    def _profvcf(self, obj, threshold=None):
        params = dict()
        params['cmdr'] = 'profVCF'
        params['start'] = 0
        params['cnt'] = 20
        params['objs'] = obj
        params['idSub'] = -1
        params['down'] = 1
        if threshold:
            params['cutOffCall'] = threshold
        
        req = self.session.get(url=self.dna_cgi, params=params, stream=True)
        req.raise_for_status()
        self._logged_in_check()
        return req        

    def _check_proc_status(self, req_id):
        params = dict()
        params['cmdr'] = '-qpGRList'
        params['showreqs'] = False
        params['req'] = req_id

        req = self.session.get(self.dna_cgi, params=params)
        req.raise_for_status()
        self._logged_in_check()

        try:
            return req.json()['Head']['status']
        except (ValueError, KeyError):
            raise _BadResponseError('Unable to retrieve status of process with req id {}'.format(req_id))
        
    def _poll_proc_status(self, req_id, sleep_time):
        while True:
            status = self._check_proc_status(req_id)
            if status == 'Done':
                return True
            elif status in ['Killed', 'ProgError', 'SystemError', 'Suspended']:
                raise _HiveComputationError('HIVE process with req id {} failed'.format(req_id))
            else:
                time.sleep(sleep_time)

    def _init_upload(self, file_list, qc, screen, reference):
        params = dict()
        params['raw'] = 1
        params['cmd'] = 'init'
        params['prop.svc-pipeline-upload.st2_datasource'] = 'file://'
        params['prop.svc-pipeline-upload.isPostponed'] = 0
        params['prop.svc-pipeline-upload.st2_dissect'] = -1
        params['prop.svc-pipeline-upload.st2_run_qc'] = int(qc)
        params['prop.svc-pipeline-upload.st2_run_screen'] = int(screen)

        if reference:
            params['prop.svc-pipeline-upload.st2_upload_subject.1'] = reference

        # construct comma separated file list
        up_name = ','.join(file_list)
        if len(up_name) >= 100:
            up_name = up_name[:97] + '...'
        params['prop.svc-pipeline-upload.name'] = up_name

        init_req = self.session.get(url=self.upload_cgi, params=params)
        self._logged_in_check()
        init_req.raise_for_status()
        try:
            req_json = init_req.json()
            return req_json['reqid'], req_json['chunkSize'], req_json['concurrency']
        except (ValueError, KeyError):
            raise _BadResponseError('Unable to retrieve request id, chunk size, or concurrency from upload initialization request')

    def _finish_upload(self, req_id):
        fin_req = self.session.put(self.upload_cgi, params={'req': req_id, 'raw': 1})
        fin_req.raise_for_status()
        self._logged_in_check()
        try:
            req_json = fin_req.json()
            return req_json['id'], req_json['goto']
        except (ValueError, KeyError):
            raise _BadResponseError('Unable to retrieve upload path when finalizing upload for req id {}'.format(req_id))

    def _delete_upload(self, req_id):
        del_req = self.session.delete(self.upload_cgi, params={'req': req_id})
        del_req.raise_for_status()
        self._logged_in_check()

    def _upload_parts(self, part_group, req_id, req_num, total_req):
        upload_params = dict()
        upload_params['req'] = req_id
        upload_params['sendid'] = 'part{}'.format(req_num)

        head_blob = dict()
        head_blob['index'] = req_num
        head_blob['total'] = total_req
        head_blob['files'] = []

        upload_data = dict()
        upload_data['info'] = 0 # order of dicitonary matters, info must come first

        uploaded_parts = []
        try:
            for p in part_group:
                p.open()
                head_blob['files'].append(p.header())
                file_id = str(p.file_to_up.id)
                filename = os.path.basename(p.file_to_up.hive_pathname)
                upload_data[str(file_id)] = (filename, p)
                uploaded_parts.append(p)
            upload_data['info'] = ('blob', json.dumps(head_blob))

            req = self.session.post(url=self.upload_cgi, params=upload_params, files=upload_data)
            req.raise_for_status()
            self._logged_in_check()
        finally:
            for p in uploaded_parts:
                p.close()
        return uploaded_parts

    def _login_req(self, username, token):
        try:
            print('Logging into {} as user {}...'.format(self.url, username))
            login_data = dict()

            login_data['email'] = username
            login_data['token'] = token 
            login_data['cmdr'] = 'loginToken'  
        
            login_req = self.session.post(url=self.dna_cgi, data=login_data)
            login_req.raise_for_status()
            self._logged_in_check()
        except requests.RequestException as err:
            print('Login failed. Problem with HTTP requests: {}'.format(err))
            return False
        except _HiveLoggedOutError as err:
            print('Login failed: incorrect username or token')
            return False
        else:
            print('Login successful')
            self.logged_in = True
            return True        

    def _get_creds_from_file(self, pathname):
            username = None
            password = None
            hive_hostname = urllib.parse.urlparse(self.url).hostname
            with open(pathname, 'r') as f:
                for url in f:
                    url_info = urllib.parse.urlparse(url.rstrip())
                    if url_info.hostname == hive_hostname:
                        username = url_info.username
                        password = urllib.parse.unquote(url_info.password)
                        break
            return username, password
    
    def login(self, username=None, token=None, cred_file='~/.hive-creds', save_creds=False):
        """Logs into the HIVE instance with the username and token passed in as parameters, the username and token from
        a credentials file, or username and token given through console input, in that order of priority.

        Arguments:
        username - username of the account you will log into
        token - token of the account given by the username
        cred_file - credentials file containing username:token@hostname lines. Must have file permission mask of 600
        save_creds - whether to save login credentials to cred_file on successful login. Defaults to False

        Usage:
        >>> hs = HiveSession('https://dnahive.fda.gov')
        >>> hs.login('User.Name@email.com', 'token')
        Logging into https://dnahive.fda.gov as user User.Name@email.com...
        Login successful

        >>> hs = HiveSession('https://dnahive.fda.gov')
        >>> hs.login(cred_file='~/.hive-creds')
        Username and token not provided. Attempting to access credentials file ~/.hive-creds
        Credentials file ~/.hive-creds exists. Attempting to read credentials
        Logging into https://dnahive.fda.gov as user User.Name@email.com...
        Login successful

        >>> hs = HiveSession('https://dnahive.fda.gov')
        >>> hs.login(cred_file=None)
        Username not provided through parameter or credentials file. Please input username
        >>> Username:
        Token not provided through parameter or credentials file. Please input token
        >>> Token:
        Logging into https://dnahive.fda.gov as user User.Name@email.com...
        Login successful
        """
        cred_file_used = False
        cred_file_found = False
        cred_file_good_perm = False
        if cred_file is not None:
            cred_file = os.path.expanduser(cred_file)
            cred_file_found = os.path.exists(cred_file)
            if cred_file_found:
                print('Credentials file found at {}'.format(cred_file))
                cred_stat = os.stat(cred_file)
                cred_file_good_perm = stat.S_IMODE(cred_stat.st_mode) == 0o600
                if cred_file_good_perm is False:
                    print('Credentials file {} has wrong permissions. Must have permission mask 600 to use. Either delete or change permissions'.format(cred_file))    
            else:
                print('Credentials file not found at {}.'.format(cred_file))
                
        
        if cred_file_found and cred_file_good_perm and (username is None or token is None):
            tmp_u, tmp_p = self._get_creds_from_file(cred_file)
            if tmp_u is not None and tmp_p is not None:
                username = tmp_u
                token = tmp_p
                cred_file_used = True
                print('Credentials read from {}'.format(cred_file))
            else:
                print('Credentials not found in {}'.format(cred_file))

        if username is None:
            print('Username not provided. Please input username')
            username = input('Username:')
        if token is None:
            print('Token not provided. Please input token')
            token = getpass.getpass('Token: ')

        login_suc = self._login_req(username, token)

        if login_suc and save_creds and cred_file_used is False and cred_file is not None:
            if cred_file_found is False or (cred_file_found and cred_file_good_perm):
                old_mask = os.umask(0)
                try:
                    fd = os.open(cred_file, os.O_CREAT | os.O_WRONLY, 0o600)
                    with open(fd, 'a') as f:
                        f.write('https://{}:{}@{}\n'.format(username, urllib.parse.quote(token), urllib.parse.urlparse(self.url).hostname))
                except OSError as err:
                    print('Problem writing credentials file: {}'.format(err))
                else:
                    print('Credentials saved to {}'.format(cred_file))
                finally:
                    os.umask(old_mask)

    def logout(self):
        """Logs out from the current HIVE session. Will clear all cookies even if logout request fails"""
        try:
            print('Logging out from {}...'.format(self.url))
            req = self.session.get(url=self.dna_cgi, params={'cmdr': 'logout'})
            req.raise_for_status()
        except requests.RequestException as err:
            print('Logout failed. Problem with HTTP requests: {}'.format(err))
        else:
            print('Logout successful')
            self.logged_in = False
        finally:
            self.session.close()
        
    def download(self, objs, dst_dir=None, overwrite=False, make_obj_dir=False, heptagon_csv_profile=False, heptagon_threshold=None):
        """Downloads a list of HIVE objects into either the current working directory or a supplied directory. Returns a list of successfully downloaded files.

        Arguments:
        objs - the list of HIVE objects to download. Each object is represented as an integer
        dst_dir - the path to the directory the files will be downloaded in. Defaults to the current working directory
        overwrite - boolean specificying whether to overwrite existing files. Defaults to False
        make_obj_dir - boolean specifying whether to place each downloaded object into a separate directory named after the object ID. Defaults to False
        heptagon_csv_profile - boolean specifying how to handle the downloading of Heptagon objects.  If True, a SNP profile in HIVE's CSV format is downloaded. If False,
        a standard VCF is downloaded. Defaults to False
        
        Usage:
        >>> hs.download([1456, 22246, 22248], 'my/download/dir')
        will download HIVE objects 1456, 22246, and 22248 into the directory my/download/dir. 
        """
        downloaded = []
        if dst_dir is None:
            dst_dir = os.getcwd()
        try:
            q_types = ['u-file+', 'svc-align-pairwise+', 'svc-align-multiple+', 'svc-profiler-heptagon+']
            obj_types = self._check_obj_types(objs, q_types)
            for obj, types in zip(objs, obj_types):                
                print('Downloading object {}...'.format(obj))
                if make_obj_dir:
                    obj_dst_dir = os.path.join(dst_dir, '{}'.format(obj))
                else:
                    obj_dst_dir = dst_dir
                try:
                    if 'u-file+' in types:
                        req = self._objfile(obj)
                        downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite))
                    elif 'svc-align-pairwise+' in types:
                        req = self._alsam(obj)
                        downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite))
                    elif 'svc-align-multiple+' in types:
                        req = self._alfasta(obj)
                        downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite))
                        req = self._alconsensus(obj)
                        downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite))
                    elif 'svc-profiler-heptagon+' in types:
                        if heptagon_csv_profile:
                            req = self._objfile(obj, 'SNPprofile.csv')
                        else:
                            req = self._profvcf(obj, threshold=heptagon_threshold)
                        downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite))
                    else:
                        file_list = self._obj_file_list(obj)           
                        for pathname in file_list:
                            if os.path.basename(pathname).startswith('_.'):
                                continue
                            req = self._objfile(obj, pathname)
                            req.raise_for_status()
                            self._logged_in_check()
                            downloaded.append(self._download_req(req, obj_dst_dir, obj, overwrite, pathname))
                except (requests.RequestException, _BadResponseError, FileExistsError) as err:
                    print('Skipping object {}: {}'.format(obj, err))
                    continue
        except KeyboardInterrupt:
            print('Cancelling download')
        except (_BadResponseError, requests.RequestException, _HiveLoggedOutError, OSError) as err:
            print('Download failed: {}'.format(err))
        else:
            print('Download finished')
        return downloaded                
        
    def upload(self, pathnames, max_threads=None, folder=None, reference=None, qc=False, screen=False):
        """Uploads a list of files to the HIVE instance. Returns a list of successfully uploaded parts.

        Arguments:
        pathnames - a list of paths of files to upload. When given paths to folders, those folders will be uploaded recursively with the paths inside the
        folder preserved.
        max_threads - maximum number of threads to be executed concurrently. The maximum number is not guaranteed to be utilized, as it depends on
        the response from the server
        folder - object ID (as an integer) of the HIVE folder to upload to. Defaults to None
        reference - object ID (as an integer) of the HIVE reference to associate with the uploaded files. Used when uploading alignments
        qc - boolean determing whether to perform quality control on upload. Defaults to False
        screen - boolean determining whether to perform screening on upload. Defaults to False
        
        Usage:
        >>> hs.upload(['file1.txt', 'file2.txt' 'path/to/dir'])
        """
        uploaded_parts = []
        try:
            print('Initializing upload...')
            req_id, chunk_size, max_conc = self._init_upload(pathnames, qc, screen, reference)
            if max_threads:
                max_conc = min(max_conc, max_threads)
            print('Upload initialized. Using request id {} with chunk size {} and max concurrency {}'.format(req_id, chunk_size, max_conc))
            print('Dividing files into parts for upload...')
            files_to_up = []
            file_cnt = 0
            for pathname in pathnames:
                if os.path.isdir(pathname):
                    for dirpath, dirnames, filenames in os.walk(pathname):
                        for f in filenames:
                            files_to_up.append(_FileToUp(os.path.join(dirpath, f), os.path.join(dirpath, f), file_cnt))
                            file_cnt += 1
                else:
                    files_to_up.append(_FileToUp(pathname, pathname, file_cnt))
                    file_cnt += 1
            reqs = _gen_file_parts(files_to_up, chunk_size)
            total_reqs = len(reqs)
            print('Divided file into {} requests'.format(total_reqs))
            try:
                print('Sending requests...')
                init_time = time.time()
                with concurrent.futures.ThreadPoolExecutor(max_workers=max_conc) as executor:
                    futures = []
                    for req_num, part_group in enumerate(reqs):
                        futures.append(executor.submit(self._upload_parts, part_group, req_id, req_num, total_reqs))
                    for future in concurrent.futures.as_completed(futures):
                        uploaded_parts.extend(future.result())
                elapsed = time.time() - init_time
                print('All requests uploaded in {:.3f} seconds. Finalizing upload...'.format(elapsed))
                pipeline_id, pipeline_goto = self._finish_upload(req_id)
            except Exception as err:
                try:
                    self._delete_upload(req_id)
                finally:
                    raise err
            pipeline_url = '{}?{}'.format(self.dna_cgi, pipeline_goto) 
            print('Upload finalized. Check object id {} at {} to monitor progress'.format(pipeline_id, pipeline_url))
        except KeyboardInterrupt:
            print('Cancelling upload')
        except (_BadResponseError, _HiveComputationError, _HiveLoggedOutError, OSError, requests.RequestException) as err:
            print('Upload failed: {}'.format(err))
        else:
            print('Upload completed sucessfully')
        return uploaded_parts

    def get_all_objs(self, type='', print_table=True):
        """Retreives information about objects in a HIVE instance. Can either print the information in a tabular format
        or return a list of objects.

        Arguments:
        type - The internal type of HIVE objects to return. Default return all types. This should only be used by advanced users knowledgable
        of HIVE's internal type system. Normal users can utilize HiveSessions other "get_*" functions to retreive specific categories of objects
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        try:
            qry = 'alloftype("{}").map({{[this.id, this.name, this.created, this._type]}})'.format(type)
            result = self._objqry(qry)
            result = json.loads(result)
            if print_table:
                print('{:^10} | {:^50} | {:^27} | {:^10}'.format('obj', 'name', 'created', 'type'))
                for obj, name, created, type in result:
                    print('{:<10} | {:<50} | {:^27} | {:<10}'.format(obj, str(name)[:50], created, type))
            else:
                return result
        except KeyboardInterrupt:
            print('Cancelling object query')
        except ValueError:
            print('Object query failed. Unable to understand type query results')
        except (requests.RequestException, _HiveLoggedOutError) as err:
            print('Object query failed: {}'.format(err))

    def get_folders(self, print_table=True):
        """Retreives information about all folder HIVE objects. Can either print the information in a tabular format or return a list of objects.

        Arguments:
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        return self.get_all_objs('folder', print_table)

    
    def get_genomes(self, print_table=True):
        """Retreives information about all genome HIVE objects. Can either print the information in a tabular format or return a list of objects.

        Arguments:
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        return self.get_all_objs('genome', print_table)

    def get_reads(self, print_table=True):
        """Retreives information about all read HIVE objects. Can either print the information in a tabular format or return a list of objects.

        Arguments:
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        return self.get_all_objs('nuc-read', print_table)

    def get_files(self, print_table=True):
        """Retreives information about all file HIVE objects. Can either print the information in a tabular format or return a list of objects.

        Arguments:
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        return self.get_all_objs('u-file', print_table)

    def get_computations(self, print_table=True):
        """Retreives information about all computation HIVE objects. Can either print the information in a tabular format or return a list of objects.

        Arguments:
        print_table - Whether to print information about the objects in a tabular format or return a python list. Defaults to True.
        """
        return self.get_all_objs('svc-computations-base+', print_table)

    def __enter__(self):
        return self

    def __exit__(self, exec_type, exec_value, traceback):
        if self.logged_in:
            self.logout()
