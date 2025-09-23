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

#
# Demo Python computational service
#

# pyhive module provides all the HIVE API Python bindings
import pyhive

# we will use CSV format for output
import csv

# for math.max/min
import math

# The on_execute() function from your module will be called when the
# backend grabs a new request for your service.
def on_execute(req_id):
    # If new requests arrive frequently enough, on_execute() will be
    # repeatedly called within the same Linux process. This means:
    # * avoid resource leaks in on_execute();
    # * don't cache request-local data (req_id, form values, etc.) in
    #   data structures which remain alive after on_execute() returns.

    # Your main interface to HIVE APIs is the pyhive.proc Python object of type pyhive.Proc

    # To log some debugging messages to the console (and DB, depending
    # on system settings), use pyhive.proc.log() :
    pyhive.proc.log(pyhive.log_type.TRACE,
        "Executing request {} for service '{}'".format(pyhive.proc.req_id, pyhive.proc.svc.name))

    # pyhive.proc.form is a dictionary containing HTTP form values
    # submitted by the user, plus some additional data
    pyhive.proc.log(pyhive.log_type.TRACE,
        "Form = {}".format(pyhive.proc.form))

    # pyhive.proc.obj is the HIVE computational process object associated with
    # the current request (note: requests can also exist without a HIVE object).
    # Requests are ephemeral and will be auto-deleted after a set interval,
    # while HIVE objects have permanent storage.
    pyhive.proc.log(pyhive.log_type.TRACE,
        "Process object {}".format(pyhive.proc.obj))

    # To read an object field, use prop_get; it automatically converts the
    # data to an appropriate python object (in this case, a list of HIVE IDs).
    try:
        file_ids = pyhive.proc.obj.prop_get("list")
    except:
        # On unrecoverable failure, use pyhive.proc.req_set_info to tell
        # the user what happened...
        pyhive.proc.req_set_info(pyhive.log_type.ERROR, "Failed to read 'list' field from process object")
        # ... and then either re-raise the exception, or alternatively,
        # set pyhive.proc.req_set_status(pyhive.req_status.PROG_ERROR) and return.
        # We will just re-raise for simplicity.
        raise

    # Requests can be automatically parallelized. The computation object
    # needs to have the following fields:
    # * split - name of splitter field (list, nrepeat, lines, query)
    # * slice - number of "items" (file lines etc.) to process per request
    # For the demo, the splitting field is "list", which contains a list of HIVE IDs
    # of file objects selected by the user.
    # To figure out which part of that list our request is supposed to process,
    # we use pyhive.proc.req_slice_id and pyhive.proc.req_slice_cnt to see how
    # many files each request should process
    files_per_req = int(max(1, math.ceil(len(file_ids) * 1.0 / pyhive.proc.req_slice_cnt)))

    # From files_per_req and file_ids, we find the list of HIVE file objects that
    # the current request needs to work on.
    # We could easily do this using a list comprehension, but an explicit loop
    # allows for better error messages for the user if something goes wrong
    files = []
    for fid in file_ids[files_per_req * pyhive.proc.req_slice_id : files_per_req * (1 + pyhive.proc.req_slice_id)]:
        try:
            # Open HIVE file object with given ID
            files.append(pyhive.FileObj(fid))
        except:
            pyhive.proc.req_set_info(pyhive.log_type.ERROR, "Failed to open file object {}".format(fid))
            raise

    # pyhive.proc.add_file_path gives us a path for a new output file.
    # We want this path unique for each request, this is indicated by
    # the magic "req-" prefix.
    with open(pyhive.proc.add_file_path("req-out.csv"), "wb") as outfile:
        writer = csv.writer(outfile)
        writer.writerow(["id","name","lines"])
        prog = 0
        # For our demo app, we want to count the number of lines in each file
        for fobj in files:
            # We must regularly report the computation progress. Otherwise, the
            # system will assume that the request has hung or crashed - and will
            # try kill and then restart it.
            pyhive.proc.req_progress(prog, len(files))
            prog += 1

            lines = 0
            # file_path property of proc.FileObj objects gives the path of
            # the object's primary file. Other object types don't have a primary file.
            with open(fobj.file_path, "r") as infile:
                for line in infile:
                    lines += 1

            name = fobj.prop_get("name")
            writer.writerow([fobj.id, name, lines])
            pyhive.proc.log(pyhive.log_type.DEBUG, "{} : {} lines in {}".format(fobj.id, lines, name))


    # If there are other requests which are still executing, we are done!
    if not pyhive.proc.is_last_in_group():
        # Set the appropriate status, and set progress level to 100/100
        pyhive.proc.req_progress(100, 100)
        pyhive.proc.req_set_status(pyhive.req_status.DONE)
        return

    # ... but if this is the last request, we now want to reduce the group's
    # results to a single output.

    pyhive.proc.log(pyhive.log_type.TRACE, "Last in group; reducing results")

    total_lines = 0
    max_lines = -1

    # iterate over the group's requests (including our own):
    for req in pyhive.proc.grp2req():
        try:
            # We can use req_get_data() to get another request's local data
            # (file-based or not) in the form of pyhive.Mex - a blob object
            # that supports the file-like python API
            reader = csv.DictReader(pyhive.proc.req_get_data("req-out.csv", req = req))
            for row in reader:
                total_lines += int(row["lines"])
                if int(row["lines"]) > max_lines:
                    max_lines = int(row["lines"])
                    max_name = row["name"]
                    max_id = row["id"]

        except IOError:
            pyhive.proc.req_set_info(pyhive.log_type.ERROR, "Failed to read intermediate result from request {}".format(req))
            raise

    # Record the final result in an explicit object file for pyhive.proc.obj:
    try:
        pyhive.proc.log(pyhive.log_type.DEBUG, "Total lines = {}".format(total_lines))

        with open(pyhive.proc.obj.add_file_path("result.csv", overwrite = True), "w") as outfile:
            writer = csv.writer(outfile)
            writer.writerow(["Total lines", "Max lines", "Max lines name", "Max lines ID"])
            writer.writerow([total_lines, max_lines, max_name, max_id])

    except IOError:
        pyhive.proc.req_set_info(pyhive.log_type.ERROR, "Failed to save final result")
        raise

    # Finally, we are done!
    pyhive.proc.req_progress(100, 100)
    pyhive.proc.req_set_status(pyhive.req_status.DONE)
    return

# Returns number of requests to split into if running in non-splitOnFrontEnd mode
def on_split(req_id):
    num_lists = len(pyhive.proc.obj.prop_get("list"))
    return max(num_lists, 1)
