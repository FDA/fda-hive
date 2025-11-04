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

import com.google.common.base.Function;

import java.io.IOException;
import java.nio.file.*;
import java.util.AbstractMap;
import java.util.logging.Logger;

public class DirectoryMonitor {
  private static final Logger LOG = Logger.getLogger("HIVE-ISA-REPORTER");

  public static final int EVENT_TYPE_CREATE = 0;
  public static final int EVENT_TYPE_DELETE = 1;
  public static final int EVENT_TYPE_MODIFY = 2;

  public boolean start(String directoryPath,
                Function<AbstractMap.SimpleImmutableEntry<Path, Integer>, Integer> callback) {
    Path dir = Paths.get(directoryPath).toAbsolutePath().normalize();
    WatchService watchService;
    try {
      watchService = FileSystems.getDefault().newWatchService();
      dir.register(watchService,
          StandardWatchEventKinds.ENTRY_CREATE,
          StandardWatchEventKinds.ENTRY_DELETE,
          StandardWatchEventKinds.ENTRY_MODIFY);
    } catch (IOException ex) {
      LOG.warning("Failed to start directory monitoring process: " + ex.getMessage());
      return false;
    }

    for(;;) {
      try {
        WatchKey key = watchService.take();
        for (WatchEvent<?> event : key.pollEvents()) {
          if(event.kind() == StandardWatchEventKinds.OVERFLOW) {
            continue;
          }

          int eventType = EVENT_TYPE_CREATE;
          if(event.kind() == StandardWatchEventKinds.ENTRY_DELETE) {
            eventType = EVENT_TYPE_DELETE;
          } else if(event.kind() == StandardWatchEventKinds.ENTRY_MODIFY) {
            eventType = EVENT_TYPE_MODIFY;
          }

          WatchEvent<Path> ev = (WatchEvent<Path>)event;
          callback.apply(new AbstractMap.SimpleImmutableEntry(dir.resolve(ev.context()), eventType));
        }

        key.reset();
      } catch (InterruptedException ex) {
      }
    }
  }
}
