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
from jsonflat import JsonFlat
import json
import argparse, os

def convert_csv(field_names, rows, output_name = "out_json.csv"):
  header = ""
  col_names = field_names
  i = 0
  with open(output_name, "w") as fp:
    #fp = open(output_name,"w"
    for col in col_names:
      if i > 0:
        header += ","
      header += col
      i += 1

    fp.write("{}.\n".format(header))
    for ir in range(len(rows)):
      line = ""
      i = 0
      for col in col_names:
        if i > 0:
          line += ","
        if col in rows[ir]:
          line += "\"" + str(rows[ir][col]).replace(",",";").replace("\r\n","").replace("\"","'") + "\""
        else:
          line += ""
        i+=1
      fp.write("{}\n".format(line))
    
  return 1

def flatten_json(json_file,field_separator):
  with open(json_file,"r") as f:
    data = json.loads(f.read())

  return JsonFlat(field_infix_char=field_separator).flatten(data) # by default is "."
  # ret["field_names"]
  # ret["rows"]

def main(args):
  
  inp = args.input
  nameWithExtension = os.path.basename(inp)
  nameOnly = ''.join(nameWithExtension.split(".")[:-1])
  outName = nameOnly + ".csv"

  output_dir = args.output_dir
  if output_dir == "NONE":
    output_dir=os.path.dirname(inp)
  
  outNameFull = os.path.join(output_dir,outName)
  ret = flatten_json(inp,args.field_separator)
  convert_csv(ret["field_names"],ret["rows"], outNameFull)
  return 0

if __name__=="__main__":
  
  parser = argparse.ArgumentParser(description="Json Flattener", usage='%(prog)s [options]')
  parser.add_argument("-i","--input",required=True, help="Please provide an input file, ex: test_123.json")
  parser.add_argument("-o","--output_name",default="NONE")
  parser.add_argument("-fd","--field_separator",help="Field separator for csv header",default="__")
  parser.add_argument("-od","--output_dir",default="NONE")

  args = parser.parse_args()
  main(args)

