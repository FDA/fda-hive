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
#include <slib/std/app.hpp>
#ifdef WIN32
#else
#include <signal.h>
#endif

using namespace slib;

sStr sApp::gCfg;
sStr * sApp::cfg = &sApp::gCfg;

sVar sApp::gVar;
sVar * sApp::var = &sApp::gVar;

idx sApp::argc;
const char ** sApp::argv;
const char ** sApp::envp;

static
udx commonsetup(void)
{
    srand((unsigned int) (time(0) + getpid()));
    return 0;
}

udx sApp::err_location = commonsetup();


static
void SigHupHandler(int sig)
{
    // ignore
}

static
void SigFaultHandler(int sig)
{
    if( sMex::newSize != 0 ) {
        ::fprintf(stderr, "\nInsufficient resources for the program, try freeing some memory and/or disk space\n");
    }
    if( sApp::err_location ) {
        ::fprintf(stderr, "\nError occurred at position %" UDEC "\n", sApp::err_location);
    }
    fflush(stdout);
    fflush(stderr);
    exit(1);
}

static struct
{
        void (*handler)(int);
        int sig;
} sigs[] = {
    { SigHupHandler, SIGHUP },
    { SigFaultHandler, SIGSEGV },
    { SigFaultHandler, SIGBUS }
};
static struct sigaction sig_saves[sDim(sigs)];

sApp::sApp(int largc /* = 0 */, const char ** largv /* = 0 */, const char ** lenvp /* = 0 */)
{
    args(largc, largv, lenvp);
#ifndef _DEBUG
#ifdef WIN32
#else
    /* install signal handlers */
    for(int i = 0; i < sDim(sigs); ++i) {
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = sigs[i].handler;
        act.sa_flags = SA_RESETHAND;
        sigaction(sigs[i].sig, &act, &sig_saves[i]);
    }
#endif
#endif
}

sApp::~sApp(void)
{
    for(idx i = 0; i < sDim(sigs); ++i) {
        sigaction(sigs[i].sig, &sig_saves[i], 0);
    }
}

char * sApp::cfgget(const char * section, const char * name, const char * defVal)
{
    sStr nm("%s%s%s", section, section ? "." : "", name), t;
    char * res = (char *) var->out(nm.ptr(), 0, 0);

    if( !res ) { // read from file
        if( !cfg || !cfg->length() ) {
            return (char *) sVar::nonconst(defVal);
        }
        idx st = 0, en = 0;
        sString::searchAndReplaceSymbols(&t, nm.ptr(), 0, ".", 0, 0, true, true, true);
        sString::searchStruc(cfg->ptr(),cfg->length(),t.ptr(), "[" _ "\n" __,&st,&en); // first we find out variable
        t.cut(0);
        if(st!=en)sString::cleanEnds(&t,cfg->ptr(st),en-st,"=" sString_symbolsBlank,true); // then we clean equal signs and spaces at both ends
        if( !t.length() ) {
            t.add(__, 2);
        }
        var->inp(nm.ptr(), t.ptr(), t.length()); // remember this so we don't go to the file next time
        res = (char *) var->out(nm.ptr(), defVal, 0);
    }
    return res;
}

idx sApp::cfgsave(const char * section00, bool truncate)
{
    if( truncate ) {
        cfg->cut(0);
    }
    idx iprt = 0;
    for(const char * section = section00; section; section = sString::next00(section)) {
        idx l = 0;
        if( section ) {
            cfg->printf("\n[%s]\n", section);
            l = sLen(section);
        }
        for(idx i = 0; i < var->dim(); ++i) {
            const char * nm = (const char *) var->id(i);
            if( section && (strncmp(nm, section, l) != 0 || nm[l] != '.') ) {
                // section specified and this is not one of those
                continue;
            }
            const char * val = var->value(nm, 0);
            if( !val ) {
                continue;
            }
            if( l ) {
                nm += l + 1;
            }
            cfg->printf("\t%s = %s\n", nm, val);
            ++iprt;
        }
        cfg->printf("\n");
    }
    return iprt;
}
