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
import pyhive
import unittest

class TestMex(unittest.TestCase):
    
    def test_mex_concat(self):
        hello_mex = pyhive.Mex(data="hello ")
        world_mex = pyhive.Mex(data="world")
        self.assertEqual(str(hello_mex + world_mex), "hello world")
        emptyMex = pyhive.Mex(data="")
        self.assertEqual(str(hello_mex + emptyMex), "hello ")
        self.assertEqual(str(emptyMex + emptyMex), "")
    
    def test_mex_next(self):
        m = pyhive.Mex(data="hello\nworld")
        self.assertEqual(next(m), "hello\n")
        
    def test_mex_read(self):
        m = pyhive.Mex(data="hello world")
        self.assertEqual(m.read(3), "hel")
        self.assertEqual(m.read(2), "he")
        self.assertEqual(m.read(0), "")
        self.assertEqual(m.read(), "hello world")
        
    def test_mex_readlines(self):
        m = pyhive.Mex(data="hello\nworld\ngoodbye\nworld\n")
        self.assertEqual(m.readlines(), "hello\nworld\ngoodbye\nworld\n")
        self.assertEqual(m.readlines(), "")
        m2 = pyhive.Mex(data="hello\nworld\ngoodbye\nworld\n")
        self.assertEqual(m2.readlines(7), "hello\n")
        
    def test_mex_truncate(self):
        m = pyhive.Mex(data="hello world")
        m.truncate(3)
        self.assertEqual(str(m), "hel")
        m.truncate(0)
        self.assertEqual(str(m), "")
        
    def test_mex_write(self):
        m = pyhive.Mex()
        m.write("hello")
        m.write(" world")
        self.assertEqual(str(m), "hello world")
        m.write("")
        self.assertEqual(str(m), "hello world")
        
    def test_mex_writelines(self):
        m = pyhive.Mex()
        m.writelines([])
        self.assertEqual(str(m), "")
        m.writelines(["hello ", "", "world"])
        self.assertEqual(str(m), "hello world")
        
    def test_mex_seek(self):
        m = pyhive.Mex(data="hello world")
        m.seek(3)
        self.assertEqual(m.tell(), 3)
        self.assertEqual(m.read(), "lo world")

class TestTaxIon(unittest.TestCase):
    
    def setUp(self):
        self.tax = pyhive.TaxIon()
        
    def test_get_tax_ids_by_name(self):
        tax_id_list = self.tax.get_tax_ids_by_name("Homo sapiens")
        self.assertTrue(len(tax_id_list) > 0)
    
    # NCBI website claims that taxids are assigned sequentially, never reused,
    # and are occasionally removed during a taxonomic revision. It appears
    # reasonably safe to assume 9606 will refer to Homo sapiens. If that
    # changes then this test will break
    def test_get_tax_id_info(self):
        tax_id_info = self.tax.get_tax_id_info(9606)
        self.assertEqual(tax_id_info["tax_id"], 9606)
        self.assertEqual(tax_id_info["name"], "Homo sapiens")
    
    # NCBI website claims that taxids are assigned sequentially, never reused,
    # and are occasionally removed during a taxonomic revision. It appears
    # reasonably safe to assume 9606 will refer to Homo sapiens. If that
    # changes then this test will break
    def test_ion_wander(self):
        wander = pyhive.IonWander(self.tax, "o=find.taxid_name(taxid=$t, tag='scientific name'); printCSV(o.taxid, o.name)")
        wander.set_search_template_variable('$t', '9606')
        wander.traverse()
        self.assertEqual(str(wander.result), "9606,Homo sapiens\n")

class TestObj(unittest.TestCase):
    
    def setUp(self):
        self.obj = pyhive.proc.obj
    
    def test_file_operations(self):
        test1_path = self.obj.add_file_path("test1.txt", True)
        test2_path = self.obj.add_file_path("test2.txt", True)

        # make sure path is actually writeable
        test1_file = open(test1_path, "w+")
        test1_file.write("Hello World 1")
        test1_file.close()
        
        test2_file = open(test2_path, "w+")
        test2_file.write("Hello World 2")
        test2_file.close()
        
        self.assertEqual(self.obj.get_file_path("test1.txt"), test1_path)
        
        self.assertEqual(self.obj.files().sort(), ["test_results.txt", "test1.txt", "test2.txt"].sort())
        self.assertEqual(self.obj.files("*2*"), ["test2.txt"])
        
        self.obj.del_file_path("test2.txt")
        self.obj.del_file_path("test1.txt")
        self.assertEqual(self.obj.files(), ["test_results.txt"])
        
    def test_is_type_of(self):
        self.assertTrue(self.obj.is_type_of(self.obj.type.name))
        self.assertTrue(self.obj.is_type_of("not-a-real-type,{}".format(self.obj.type.name)))
        self.assertFalse(self.obj.is_type_of("!{}".format(self.obj.type.name)))

    def test_obj_list(self):
        objs = self.obj.obj_list(types=self.obj.type.name)
        ids = [x.id.obj_id for x in objs]
        self.assertTrue(self.obj.id.obj_id in ids)

class TestProcReq(unittest.TestCase):
    
    def setUp(self):
        pyhive.proc.req_set_data(name="test1", data="hello world 1")
        pyhive.proc.req_set_data(name="test2", data="hello world 2")
        
        # third way of adding requesting data
        req_path = pyhive.proc.add_file_path(name="req-test3")
        with open(req_path, "w+") as req_file:
            req_file.write("hello world 3")
    
    def test_req_data_names(self):
          self.assertEqual(pyhive.proc.req_data_names().sort(), ["test1", "test2", "test3"].sort())

    def test_req_get_data(self):
        data1 = pyhive.proc.req_get_data("test1")
        data2 = pyhive.proc.req_get_data("test2")
        data3 = pyhive.proc.req_get_data("test3")
        self.assertEqual(str(data1), "hello world 1")
        self.assertEqual(str(data2), "hello world 2")
        self.assertEqual(str(data3), "hello world 3")

    def test_get_file_path_req(self):
        req_path = pyhive.proc.get_file_path("req-test1")
        with open(req_path, "r") as req_file:
            self.assertEqual(req_file.read(), "hello world 1")
    
    def test_req_get_data_path(self):
        req_path = pyhive.proc.req_get_data_path("test1")
        with open(req_path, "r") as req_file:
            self.assertEqual(req_file.read(), "hello world 1")

#     def test_add_file_path_req(self):
#         with open(req_path, "r") as req_file:  
#             self.assertEqual(req_file.read(), "hello world")

def on_execute(req_id):
    pyhive.proc.req_progress(0, 100)
    
    pyhive.proc.req_set_info(pyhive.log_type.INFO, "Attempting to open file to print unit test results")
    try: 
        out_path = pyhive.proc.add_file_path("test_results.txt")
        out_file = open(out_path, "w+")
        out_file.write("Test results for pyhive object {} request {}\n\n"
                        .format(pyhive.proc.obj.id.obj_id, pyhive.proc.req_id))
        pyhive.proc.req_set_info(pyhive.log_type.INFO, "Successfully opened file to print test results")
    except:
        pyhive.proc.req_set_info(pyhive.log_type.ERROR, "Failed to open file to print test results")
        pyhive.proc.req_set_status(pyhive.req_status.PROG_ERROR)
        return
    
    pyhive.proc.req_progress(5, 100)
    pyhive.proc.req_set_info(pyhive.log_type.INFO, "Running tests")
    
    test_suite = unittest.TestLoader().discover("test_pyhive")
    test_runner = unittest.TextTestRunner(stream=out_file, verbosity=2)
    test_result = test_runner.run(test_suite)
    out_file.close()
    
    pyhive.proc.req_set_info(pyhive.log_type.INFO, "Tests finished")
    if test_result.wasSuccessful():
        pyhive.proc.req_set_info(pyhive.log_type.INFO, "Tests Successful. All Tests Passed")
    else:
        pyhive.proc.req_set_info(pyhive.log_type.INFO,
                                 "Tests NOT Successful: {} failures and {} errors. Download results file for more information"
                                    .format(len(test_result.failures), len(test_result.errors)))
    
    
    pyhive.proc.req_progress(100, 100)
    pyhive.proc.req_set_status(pyhive.req_status.DONE)
    return
