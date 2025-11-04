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
package owner.isa.hive.utils;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.logging.Logger;

import com.google.common.collect.ImmutableSet;

public class ProcessRunner {
  private static final Logger LOG = Logger.getLogger("HIVE-ISA-REPORTER");

  public static final String run(ImmutableSet<Integer> expectedNormExitCodes, String... args) {
    StringBuilder processOutput = new StringBuilder();
    try {
      ProcessBuilder builder = new ProcessBuilder(args);
      builder.redirectErrorStream(true);
      Process process = builder.start();
      BufferedReader processOutputReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
      String readLine;
      while ((readLine = processOutputReader.readLine()) != null) {
        processOutput.append(readLine + System.lineSeparator());
      }

      int exitCode = process.waitFor();
      if(expectedNormExitCodes != null && !expectedNormExitCodes.isEmpty() && !expectedNormExitCodes.contains(exitCode)) {
        processOutput.setLength(0);
      }
    } catch (Exception ex) {
      processOutput.setLength(0);
      LOG.warning("Failed to run the process: " + ex.getMessage());
    }

    return processOutput.toString().trim();
  }
}
