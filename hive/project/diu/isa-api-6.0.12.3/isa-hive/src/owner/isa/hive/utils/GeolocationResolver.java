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

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.net.*;
import java.io.*;
import java.util.logging.Logger;

public class GeolocationResolver {
  private static final Logger LOG = Logger.getLogger("HIVE-ISA-REPORTER");

  public static final String[] resolve() {
    String[] result = null;
    try {
      URL url = new URL("https://ipinfo.io");
      HttpURLConnection con = (HttpURLConnection) url.openConnection();
            con.setRequestMethod("GET");
      con.setRequestProperty("Accept", "*/*");
      con.setRequestProperty("User-Agent", "wget");
      con.setRequestProperty("Connection", "close");

      // Read the Response From Input Stream
      BufferedReader in = new BufferedReader(new InputStreamReader(con.getInputStream()));
      StringBuilder sb = new StringBuilder();
      String inputLine;
      while ((inputLine = in.readLine()) != null)
        sb.append(inputLine);
      in.close();

      final String jsonString = sb.toString().trim();
      JsonNode rootObj;
      try {
        ObjectMapper mapper = new ObjectMapper();
        rootObj = mapper.readTree(jsonString);
      } catch (JsonProcessingException ex) {
        LOG.warning("Error parsing the JSON string: " + ex.getMessage());
        return null;
      }

      if((rootObj == null) || !rootObj.isObject()) {
        LOG.warning("Error parsing the JSON output - it is not an object");
        return null;
      }

      JsonNode loc = rootObj.get("loc");
      if((loc == null) || !loc.isTextual()) {
        LOG.warning("Invalid 'loc' field,");
        return null;
      }

      final String location = loc.asText("0,0");
      LOG.info("Current geolocation is: " + location);
      result = location.split(",", 2);;
    } catch (MalformedURLException ex) {
      LOG.warning("The specified URL is malformed: " + ex.getMessage());
    } catch (IOException ex) {
      LOG.warning("An I/O error occurs: " + ex.getMessage());
    }

    return result;
  }
}
