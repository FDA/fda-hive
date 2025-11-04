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

source ~/.hive_bash_rc

# Input file containing a list of object IDs
ID_FILE="object_ids.txt" 

# Destination directory for copied files
DEST_DIR="/home/qpride/argos-data-transfer"

# Output zip file
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
ZIP_FILE="ARGOS_results${TIMESTAMP}.zip"

# Ensure the destination directory exists
mkdir -p "$DEST_DIR"

# Check if the ID file exists
if [ ! -f "$ID_FILE" ]; then
  echo "Error: ID file '$ID_FILE' does not exist"
  exit 1
fi

# Loop through each object ID
while IFS= read -r obj_id; do
  echo "Processing object ID: $obj_id"

  # Run qcd to navigate to the directory
  qcd "$obj_id"

  # Check if qcd succeeded
  if [ $? -ne 0 ]; then
    echo "Error: Could not run qcd $obj_id"
    continue
  fi

  # Get the current directory (after qcd)
  CURRENT_DIR=$(pwd)

  # Copy files to the destination directory
  cp "$CURRENT_DIR"/* "$DEST_DIR"

  # Go back to the initial directory
  cd - > /dev/null
done < "$ID_FILE"

# Zip the destination directory
echo "Zipping files..."
zip -r "$ZIP_FILE" "$DEST_DIR"


