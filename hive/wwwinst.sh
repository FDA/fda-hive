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

if [ "$#" -lt 4 ]; then
    echo "Usage: $0 install_location version (0 - no version) bindir htmldir [htmldir[/ - dir itself, content otherwise] ...]"
    exit 1
fi
instdir="$1"
shift
VERSION="$1"
shift
bindir="$1"
shift

# check htmldirs for existance
arg_err=0
for arg in "$@"; do
    if [ ! -d "$arg" ]; then
        echo "Path not found '$arg'"
        let arg_err=arg_err+1
    fi
done
if [ $arg_err -gt 0 ]; then
    exit 5
fi

if [ ! -d "$bindir" ]; then
    echo "Path not found '$bindir'"
    exit 2
fi
if [ ! -d "$instdir" ]; then
    echo "Path not found '$instdir'"
    exit 3
fi
instver="$instdir/"
if [ "$VERSION" = "0" ]; then
    VERSION=""
fi
if [ ! "$VERSION" = "" ]; then
    VERDIR=".$VERSION" 
    if [ -d "$instdir/$VERDIR" ]; then
        echo "VERDIR '$instdir/$VERDIR' exists"
        exit 4
    fi
    instver="$instdir/$VERDIR/"
    mkdir -p "$instver"
    echo "Installing to $instver version $VERSION"
else
    echo "Installing to $instver"
fi

cd $bindir
CGIS="dmDownloader.cgi dna.cgi pdb.cgi taxTree.cgi hive.cgi upload.cgi"
for f in $CGIS; do
    rm -rf "$instver$f"
    cp -vp $f "$instver$f"
    if [ "$USER" = "root" ]; then
        chown root:root "$instver$f"
    fi
    chmod a+rx,a-w "$instver$f"
done
ln -fsv dmDownloader.cgi "$instver/dmUploader.cgi"

#special case of distributed apps
rm -rf $instver/apps
mkdir $instver/apps
cp -p vioapp $instver/apps/blue_nrapp
rm -rf /tmp/hive_insilico
#HIVE insilico
mkdir /tmp/hive_insilico
cp -p hive_insilico_README.txt $instver/apps/
cp -p hive_insilico_README.txt /tmp/hive_insilico/README.txt
cp -p vioapp /tmp/hive_insilico/hive_insilico
cp -p ionapp /tmp/hive_insilico/hive_ion
zip -j $instver/apps/hive_insilico.zip /tmp/hive_insilico/*
if [[ $? -ne 0 ]]; then
    echo "Failed to pack insilico"
    exit 22;
fi
rm -rf /tmp/hive_insilico
cd - >/dev/null


# process them
for arg in "$@"; do
    slen=${#arg}
    let slen=slen-1
    slast=${arg:$slen}
    if [ "$slast" = "/" ]; then
        snm=$arg
    else
        snm='*'
        cd $arg
    fi
    for d in $snm; do
        if [ "$d" = "makefile" ]; then
            continue
        fi
        echo "copying '$d'";
        if [ ! -d "$instver/$d" ]; then
            rm -f "$instver/$d"
        fi
        cp -Rfp $d "$instver"
        dd=`dirname $d`
        if [ ! "$dd" = "$d" ]; then
            if [ ! "$dd" = "." ]; then
                dlen=${#dd};
                let dlen=dlen+1
                d=${d:$dlen};
                echo "copied folder $d";
            fi
        fi
        if [ "$USER" = "root" ]; then
            chown -R root:root "$instver/$d"
        fi
        find "$instver/$d" -type d -exec chmod 0755 {} \;
        find "$instver/$d" -not -type d -exec chmod u+rw,g+r,o+r {} \;
    done
    echo "Copying libraries..."
    if [[ ! -d "$instver/lib" ]]; then
        rm -rf "$instver/lib"
        mkdir "$instver/lib" || exit 1
    fi
    if [ ! "$slast" = "/" ]; then
        cd - >/dev/null
    fi
done

# clean out svn traces
find "$instver" -name .svn -exec rm -rf {} \; 2>/dev/null

if [ ! "$VERDIR" = "" ]; then
    # update JavaScript to specific version
    os=`uname`
    SED="sed -r"
    if [ "$os" = "Darwin" ]; then
        SED="sed -E"
    fi
    cd $instver
    for f in *; do
        if [[ "$f" == "index.html" && -e "$instdir/index.html" && ! -L "$f" ]]; then
            echo "Skipped index.html: not symlink"
        else
            rm -rf "$instdir/$f"
            if [[ -x "$f" && ! -d "$f" ]]; then
                cp -pv "$f" "$instdir/$f"
            else
                ln -fsv "$VERDIR/$f" "$instdir/$f"
            fi
        fi
    done
    for h in `find -L . -name header_tmplt.html`; do
        mv "$h" "$h.sed"
        $SED -e "s/(gSysVersion\s*=\s*)[^;]+;/\1\"$VERSION\";/" "$h.sed" > "$h"
        rm "$h.sed"
    done
    cd - >/dev/null
fi
echo Done
