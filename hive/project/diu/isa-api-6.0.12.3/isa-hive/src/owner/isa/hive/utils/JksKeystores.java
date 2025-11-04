//! 
//!   Distribution Statement A. Approved for Public Release. Distribution is Unlimited.
//!   
package owner.isa.hive.utils;

import com.google.common.base.Supplier;
import gov.isa.model.UCI;
import gov.isa.spi.ComponentFactoryOption;
import gov.isa.spi.ComponentFactoryProvider;
import gov.isa.spi.StandardComponentFactoryOptions;

import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.KeyStore;

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
public class JksKeystores {

  /**
   * Returns a KeyPasswordRetriever that reads a password from the console
   *
   * <p>
   *   Password retrievers must return <strong>new</strong> char[] arrays upon
   *   each call to {@link StandardComponentFactoryOptions.KeyPasswordRetriever#< UCI >apply(UCI) apply} as the
   *   resulting array will be zeroed after each use.
   * </p>
   *
   * @return A KeyPasswordRetriever
   */
  public static StandardComponentFactoryOptions.KeyPasswordRetriever createPasswordRetriever(final Path key_directory) {
    return new StandardComponentFactoryOptions.KeyPasswordRetriever() {
      @Override public char[] apply( final UCI uci ) {
        return retrievePassword(key_directory.resolve( uci.uci() + ".pw" ));
      }
    };
  }

  /**
   * Returns a KeyStoreRetriever that assumes keystores are stored in the
   * format <code>&lt;key_directory&gt;/&lt;uci&gt;.bcfks</code> and reads JKS
   * password from the file.
   *
   * @param key_directory Directory in which to find the keystore files.
   * @return A file-based KeyStoreRetriever
   */
  public static StandardComponentFactoryOptions.KeyStoreRetriever createKeystoreRetriever( final Path key_directory ) {
    return new StandardComponentFactoryOptions.KeyStoreRetriever() {
      @Override public KeyStore apply(final UCI uci) {
        try {
          return loadKeystore(
              key_directory.resolve( uci.uci() + ".jks" ),
              new Supplier<char[]>() {
                @Override public char[] get() {
                  return retrievePassword(key_directory.resolve(uci.uci() + ".jks" + ".pw"));
                }
              }
          );

        } catch ( Exception e ) {
          throw new IllegalStateException( "unable to read keystore", e );
        }
      }
    };
  }

  /**
   * Returns a KeyStoreRetriever that assumes truststores are stored in the
   * format <code>&lt;key_directory&gt;/&lt;uci&gt;.bcfks</code> and reads BCFKS
   * password from the console.
   *
   * @param key_directory Directory in which to find the truststore files.
   * @return A file-based KeyStoreRetriever
   */
  public static StandardComponentFactoryOptions.KeyStoreRetriever createTruststoreRetriever( final Path key_directory ) {
    return createKeystoreRetriever(key_directory);
  }

  /**
   * Loads a BCFKS Keystore from the specified file path and using the specified
   * password retriever.
   *
   * @param keystore the file path of the keystore
   * @param passwordRetriever a supplier which returns a <strong>new</strong> char[] when invoked
   * @return the resulting JKS instance
   */
  public static KeyStore loadKeystore(
    Path keystore,
    Supplier<char[]> passwordRetriever
  ) {
    try ( InputStream stream = Files.newInputStream( keystore ) ) {

      KeyStore ks = KeyStore.getInstance("JKS");
      char[] password = passwordRetriever.get();
      ks.load( stream, password );
      stream.close();
      return ks;
    } catch ( Exception t ) {
      throw new IllegalStateException( "unable to load keystore", t );
    }
  }

  /**
   * Reads a char[] from System.in.
   * @return the character contents of the password.
   */
  private static char[] retrievePassword( final Path password_path ) {
    try {
      final String pswd = new String(Files.readAllBytes(password_path));
      return pswd.trim().toCharArray();
    } catch ( Exception e ) {
      throw new IllegalStateException( "unable to read password", e );
    }
  }
}
