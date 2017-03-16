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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <slib/std/file.hpp>

using namespace slib;


#ifdef BLOKED_SLIB_LINUX
idx sPS::getProcListReserv( sVec <sPS::Stat> * pi , const char * proc, const char * arguments )
{
    Stat pp;
    sStr str, path;
    idx size;
    sDir dir;

    // get the list of processes in /proc
    if( !dir.list( sFlag(sDir::bitSubdirs) , "/proc", "*", 0) ) return 0;

    for ( char * p=dir.ptr() ; p ; p=sString::next00(p) ) {
        pp.pid=0;pp.cmd[0]=0;

        // make a path to read command line
        if(sscanf(p+6,"%"DEC,&pp.pid)==0 || pp.pid==0 )continue;
        path.printf(0,"/proc/%"DEC"/cmdline",pp.pid);
        FILE * fp=fopen(path.ptr() ,"r");if(!fp)continue;
        size=fread(pp.cmd,sizeof(pp.cmd)-1,1,fp);
        fclose(fp);
        if(!size)continue;pp.cmd[size]=0;

        // now locate the arguments
        for(int jj=0; jj<size;++jj) if(pp.cmd[jj]==0)pp.cmd[jj]=' ';
        char * args=strpbrk(pp.cmd," \t\r\n");if(args){*args=0;++args;}
        char * prcnam=strrchr(pp.cmd,'/');if(!prcnam)prcnam=pp.cmd; else ++prcnam;
        if(arguments){
            if(!args)continue;
            if(!strstr(args,arguments))continue;
        }
        if(strcmp(prcnam, proc))continue;
        pp.args=(idx)(args-pp.cmd);
        pi->vadd(1,pp);
    }

    return pi->dim();
}

idx sPS::killProcess( idx pid, idx signal )
{
    sStr cmd;
    cmd.printf("kill %"DEC" %"DEC, signal, pid);
    return system(cmd.ptr());
    // return kill(pid,signal);
}



idx sPS::getMem(idx pid)
{
  sStr path;
  path.printf( "/proc/%"DEC"/statm", pid);
  FILE * fp = fopen (path.ptr(), "r"); if (!fp) return 0;
  idx mem=0;
  fscanf (fp, "%"DEC,&mem);
  fclose (fp);
  return mem;
}

#endif

#ifdef SLIB_WIN
#include <windows.h>
#include <tlhelp32.h>

//////////////////////////////////////////////////////////////////////
// Type definitions for pointers to call tool help functions
typedef BOOL (WINAPI* MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI* THREADWALK)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
typedef BOOL (WINAPI* PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef HANDLE (WINAPI* CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);

// File scope globals. These pointers are declared because of the need
// to dynamically link to the functions. They are exported only by
// the Windows kernel. Explicitly linking to them will make this
// application unloadable in Windows NT and will produce an ugly
// system dialog box


static CREATESNAPSHOT pCreateToolhelp32Snapshot = NULL;
static MODULEWALK pModule32First = NULL;
static MODULEWALK pModule32Next = NULL;
static PROCESSWALK pProcess32First = NULL;
static PROCESSWALK pProcess32Next = NULL;
static THREADWALK pThread32First = NULL;
static THREADWALK pThread32Next = NULL;

// Function that initializes tool help functions

BOOL InitToolhelp32(void)
{
    BOOL bRet = FALSE;
    HINSTANCE hKernel = NULL;

    // Obtain the module handle of the kernel to retrieve addresses
    // of the tool helper functions

    hKernel = GetModuleHandle("KERNEL32.DLL");

    if(hKernel)
    {
    pCreateToolhelp32Snapshot =
    (CREATESNAPSHOT)GetProcAddress(hKernel,    "CreateToolhelp32Snapshot");
    pModule32First = (MODULEWALK)GetProcAddress(hKernel,"Module32First");
    pModule32Next = (MODULEWALK)GetProcAddress(hKernel,"Module32Next");
    pProcess32First=(PROCESSWALK)GetProcAddress(hKernel,"Process32First");
    pProcess32Next = (PROCESSWALK)GetProcAddress(hKernel,"Process32Next");
    pThread32First = (THREADWALK)GetProcAddress(hKernel,"Thread32First");
    pThread32Next = (THREADWALK)GetProcAddress(hKernel,"Thread32Next");

    // All addresses must be non-NULL to be successful.
    // If one of these addresses is NULL, one of the needed
    // list cannot be walked.
    bRet = pModule32First && pModule32Next && pProcess32First &&
           pProcess32Next && pThread32First && pThread32Next &&
           pCreateToolhelp32Snapshot;
    }
    else
        bRet = FALSE; // could not even get the handle of kernel
    return bRet;
}

idx sPS::getProcList( sVec <sPS::Stat> * pi , const char * proc, const char * arguments,  idx   ) // execMode
{
    static bool _InitToolsCalled=false;
    if(!_InitToolsCalled) {
        InitToolhelp32();
        _InitToolsCalled=true;
    }
    Stat pp;
    HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;

    if(!hSnapshot)
        return 0;

    // Initialize size in structure
    pe.dwSize = sizeof(pe);

    for(int i = pProcess32First(hSnapshot, &pe); i; i=pProcess32Next(hSnapshot, &pe))
    {
        pp.pid=pe.th32ProcessID;
        strcpy(pp.cmd,pe.szExeFile);

        // now locate the arguments
        char * prcnam=strrchr(pp.cmd,'/');
        if(!prcnam)prcnam=strrchr(pp.cmd,'\\');
        if(!prcnam)prcnam=pp.cmd;
        else ++prcnam;
        char * args=strpbrk(prcnam," \t\r\n");if(args){*args=0;++args;}
        if(arguments){
            if(!args)continue;
            if(!strstr(args,arguments))continue;
        }
        if(strcmp(prcnam, proc))continue;
        pp.args=args ? (idx)(args-pp.cmd) : sNotIdx ;
        pi->vadd(1,pp);

        /*
        HANDLE hModuleSnap = NULL;
        MODULEENTRY32 me;

        // Take a snapshot of all modules in the specified process
        hModuleSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pe.th32ProcessID);

        if(hModuleSnap == (HANDLE) -1)
            continue;//return pi->dim();

        // Fill the size of the structure before using it
        me.dwSize = sizeof(MODULEENTRY32);

        // Walk the module list of the process, and find the module of
        // interest. Then copy the information to the buffer pointed
        // to by lpMe32 so that it can be returned to the caller

        if(pModule32First(hModuleSnap, &me))
        {
            do
            {
                // if(me.th32ModuleID == pe.th32ModuleID)
                {
                    pp.pid=me.th32ProcessID;
                    strcpy(pp.cmd,me.szExePath);

                    // now locate the arguments
                    char * prcnam=strrchr(pp.cmd,'/');
                    if(!prcnam)prcnam=strrchr(pp.cmd,'\\');
                    if(!prcnam)prcnam=pp.cmd;
                    else ++prcnam;
                    char * args=strpbrk(prcnam," \t\r\n");if(args){*args=0;++args;}
                    if(arguments){
                        if(!args)continue;
                        if(!strstr(args,arguments))continue;
                    }
                    if(strcmp(prcnam, proc))continue;
                    pp.args=args ? (idx)(args-pp.cmd) : sNotIdx ;
                    pi->vadd(1,pp);

                    break;
                }
            }
            while(pModule32Next(hModuleSnap, &me));
        }*/
    }

    CloseHandle(hSnapshot); // Done with this snapshot. Free it
    return pi->dim();

}

idx sPS::killProcess( idx pid , idx , idx  ) // signal
{
    HANDLE hHandle = ::OpenProcess(PROCESS_ALL_ACCESS,0,(DWORD)pid); //procEntry.th32ProcessID
    DWORD dwExitCode;::GetExitCodeProcess(hHandle,&dwExitCode);
    ::TerminateProcess(hHandle,dwExitCode);
    return (idx) dwExitCode;
}


#include <psapi.h>
idx sPS::getMem(idx processID, idx )
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    // Print information about the memory usage of the process.
    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, (DWORD)processID );
    if (NULL == hProcess)
        return 0;

    idx mem=0;

    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
        mem=pmc.WorkingSetSize;
    }

    CloseHandle( hProcess );
    return mem;
}

#endif

idx sPS::setMode(eExecMode mode, const char * ps_sript /* = 0 */)
{
    idx retval = false;
    switch(mode) {
        case eExec_Local:
        case eExec_BG:
            m_mode = mode;
            m_extern.destroy();
            m_extern_path.destroy();
            retval = true;
            break;
        case eExec_Extern:
            if( ps_sript && ps_sript[0] ) {
                sFilePath extPS;
                if( ps_sript[0] != '/' && ps_sript[0] != '\\') {
                    extPS.curDir();
                }
                extPS.printf("%s.os"SLIB_PLATFORM, ps_sript);
                if( sFile::exists(extPS) ) {
                    m_mode = eExec_Extern;
                    m_extern.printf(0, "%s", ps_sript);
                    m_extern_path.printf(0, "%s", extPS.ptr());
                    retval = true;
                }
            }
            break;
    }
    return retval;
}

#ifndef SLIB_WIN
idx sPS::getProcList(sVec<sPS::Stat> * pi, const char * proc, bool currentUserOnly, const char * arguments) const
{
    sStr cexe;
    if( m_mode == eExec_Extern ) {
        cexe.printf("\"%s\" ps %s", m_extern_path.ptr(), currentUserOnly ? " user" : "");
    } else {
        cexe.printf("ps %s -o pid -o command | grep -v defunct", currentUserOnly ? "-u `id -un`" : "-e");
    }
    sStr buf;
    idx ret = 0;
    sPipe::exeSys(&buf, cexe, 0, 0, &ret);
    if( ret != 0 ) {
        return -1;
    }
    buf.add0(2);
    sString::searchAndReplaceSymbols(buf.ptr(), buf.length(), sString_symbolsEndline, 0, 0, true, true, true, true);
    buf.add0(2);
    Stat pp;
    for(const char * p = buf.ptr(); p && *p; p = sString::next00(p)) {
        pp.pid = 0;
        pp.cmd[0] = 0;
        if( sscanf(p, "%"DEC, &pp.pid) == 0 || pp.pid == 0 ) {
            continue;
        }
        const char * cmd = sString::skipWords(p, 0, 1);
        if( !cmd ) {
            continue;
        }
        strcpy(pp.cmd, cmd);
        // now locate the arguments
        char * args = strpbrk(pp.cmd, " \t\r\n");
        if( args ) {
            *args = 0;
            ++args;
        }
        char * prcnam = strrchr(pp.cmd, '/');
        if( !prcnam ) {
            prcnam = pp.cmd;
        } else {
            ++prcnam;
        }
        if( arguments ) {
            if( !args ) {
                continue;
            }
            if( !strstr(args, arguments) ) {
                continue;
            }
        }
        if( proc && strcmp(prcnam, proc) ) {
            continue;
        }
        pp.args = (idx) (args - pp.cmd);
        pi->vadd(1, pp);
    }
    return pi->dim();
}

class ProcSubdirIter
{
private:
    DIR * _rootdir, * _subdir;
    idx _pid;
    sStr _buf;

public:
    ProcSubdirIter(const char * s)
    {
        _rootdir = opendir(s);
        _subdir = 0;
        _pid = -1;
        if( _rootdir ) {
            nextSubdir();
        }
    }

    ~ProcSubdirIter()
    {
        if( _subdir ) {
            closedir(_subdir);
        }
        if( _rootdir ) {
            closedir(_rootdir);
        }
    }

    bool validSubdir() const { return _subdir; }

    void nextSubdir()
    {
        // opendir/readdir instead of sDir for efficiency and because everything about reading /proc is by definition Linux-specific
        if( _subdir ) {
            closedir(_subdir);
            _subdir = 0;
        }
        while( struct dirent * dt = readdir(_rootdir) ) {
            if( !isdigit(dt->d_name[0]) ) {
                continue;
            }
            int subdir_fd = openat(dirfd(_rootdir), dt->d_name, O_RDONLY);
            if( subdir_fd < 0 ) {
                continue;
            }
            _subdir = fdopendir(subdir_fd);
            if( !_subdir ) {
                continue;
            }
            _pid = atoidx(dt->d_name);
            break;
        }
    }

    idx pid() const { return _pid; }

    // Linux does not support lseek() or mmap() in /proc, so opening a file
    // using sMex will not work; need to manually copy into a memory buffer
    bool openFile(const char *s)
    {
        _buf.cut0cut();
        int fd = openat(dirfd(_subdir), s, O_RDONLY);
        if( fd < 0 ) {
            return false;
        }
        idx read_len = 0;
        do {
            idx start_len = _buf.length();
            char * addable = _buf.add(0, getpagesize());
            read_len = read(fd, addable, getpagesize());
            _buf.cut(start_len + sMax<idx>(0, read_len));
        } while (read_len > 0);
        close(fd);
        _buf.add0(2);
        return _buf.length() > 2;
    }

    const char * ptr(idx i=0) const { return _buf.ptr(i); }
};

inline static const char * nextStatusLine(const char * s)
{
    s = strchr(s, '\n');
    return s ? s + 1 : 0;
}

idx sPS::getProcList(StatList & pi, idx flags/*=0*/, const char * proc/*=0*/, const char * argument/*=0*/) const
{
    const idx current_uid = getuid();
    for( ProcSubdirIter iter("/proc"); iter.validSubdir(); iter.nextSubdir() ) {
        if( !iter.openFile("status") ) {
            continue;
        }

        bool need_uid = flags & fCurrentUserOnly;
        bool need_state = true;

        for( const char * s = iter.ptr(); s && *s && (need_state || need_uid); s = nextStatusLine(s) ) {
            if( need_uid && !strncmp("Uid:", s, 4) ) {
                if( current_uid != atoidx(s + 5) ) {
                    // not our process
                    break;
                } else {
                    need_uid = false;
                    continue;
                }
            } else if( need_state && !strncmp("State:", s, 6) ) {
                for( s += 6; isspace(*s); s++ );
                if( *s == 'Z' ) {
                    // zombie process
                    break;
                } else {
                    need_state = false;
                    continue;
                }
            }
        }

        if( need_uid || need_state ) {
            //didn't finish scanning /proc/$pid/status because of an error condition
            continue;
        }

        if( !iter.openFile("cmdline") ) {
            continue;
        }

        const char * cmd = iter.ptr();
        if( proc ) {
            if( flags & fMatchCaseInsensitive ) {
                if( strcasecmp(cmd, proc) != 0 ) {
                    continue;
                }
            } else if( strcmp(cmd, proc) != 0 ) {
                continue;
            }
        }

        const char * first_arg = sString::next00(cmd);
        if( argument ) {
            bool arg_match = false;
            for( const char * arg = first_arg; arg && *arg; arg = sString::next00(arg) ) {
                if( flags & fMatchCaseInsensitive ) {
                    if( strcasestr(arg, argument) ) {
                        arg_match = true;
                        break;
                    }
                } else if( strstr(arg, argument) ) {
                    arg_match = true;
                    break;
                }
            }
            if( !arg_match ) {
                continue;
            }
        }

        StatList::Stat * st = pi.list.add(1);
        st->pid = iter.pid();
        st->cmdline_offset = pi.buf.length();
        sString::escapeForShell(pi.buf, cmd);
        for( const char * arg = first_arg; arg && *arg; arg = sString::next00(arg) ) {
            pi.buf.addString(" ");
            sString::escapeForShell(pi.buf, arg);
        }
        pi.buf.add0();

        if( flags & fGetMem ) {
            if( !iter.openFile("statm") ) {
                continue;
            }
            const char * s = iter.ptr();
            for( ; !isspace(*s); s++ ); // skip vmSize column
            st->mem = atoidx(s); // vmRSS
        }
    }

    return pi.list.dim();
}

idx sPS::killProcess(idx pid, idx signal) const
{
    sStr cmd;
    if( m_mode == eExec_Extern ) {
        cmd.printf("\"%s\" kill %"DEC, m_extern_path.ptr(), pid);
    } else {
        cmd.printf("kill %"DEC" %"DEC, signal, pid);
    }
    return system(cmd.ptr());
}


idx sPS::getMem(idx pid) const
{
    sIO dst;
    sStr cmd;
    idx k = 1;
    if( m_mode == eExec_Extern ) {
        cmd.printf("\"%s\" mem %"DEC, m_extern_path.ptr(), pid);
    } else {
        cmd.printf("ps -p %"DEC" -o rss | grep -v RSS", pid);
        k = 1024;
    }
    sPipe::exePipe(&dst, cmd, 0);
    idx mem = 0;
    sscanf(dst.ptr(0), "%"DEC, &mem);
    return mem * k;
}

#endif

idx sPS::exec(const char * cmdline) const
{
    sStr str;
    if( m_mode == eExec_BG ) {
#ifdef SLIB_WIN
        cmdline = str.printf("start %s", cmdline);
#else
        cmdline = str.printf("nohup %s &", cmdline);
#endif
    } else if( m_mode == eExec_Extern ) {
        cmdline = str.printf("\"%s\" exec \"%s\"", m_extern_path.ptr(), cmdline);
    }
    return (idx) system(cmdline);
}

idx sPS::exec00(const char * cmdline00, sPS::callbackSetUp setupChild, void * param) const
{
#ifdef SLIB_WIN
    // TODO
    return -1;
#else
    // ignore sigint/sigquit while waiting for child, following system(3) sematics
    struct sigaction cur_act, orig_act_intr, orig_act_quit;
    cur_act.sa_handler = SIG_IGN;
    cur_act.sa_flags = 0;
    sigemptyset(&cur_act.sa_mask);
    sigaction(SIGINT, &cur_act, &orig_act_intr);
    sigaction(SIGQUIT, &cur_act, &orig_act_quit);

    // ... but don't ignore SIGCHLD :)
    sigset_t cur_sig_mask, orig_sig_mask;
    sigemptyset(&cur_sig_mask);
    sigaddset(&cur_sig_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &cur_sig_mask, &orig_sig_mask);

    pid_t pid = fork();
    if( pid < 0 ) {
        // error!
        return (idx)pid;
    } else if( pid == 0 ) {
        // inside forked child

        // restore signals
        sigaction(SIGINT, &orig_act_intr, 0);
        sigaction(SIGQUIT, &orig_act_quit, 0);
        sigprocmask(SIG_SETMASK, &orig_sig_mask, 0);

        // inside forked child
        if( setupChild ) {
            setupChild(param);
        }

        if( !cmdline00 || !*cmdline00 ) {
            // same semantics as glibc system() call - null cmdline means check for availability of a shell
            cmdline00 = "/bin/sh"_"-c"_"exit 0"__;
        }

        sStr buf;
        sVec<const char *> argv;

        if( m_mode == eExec_Extern ) {
            for(const char * arg = cmdline00; arg && *arg; arg = sString::next00(arg)) {
                if( arg == cmdline00 ) {
                    buf.addString(" ");
                }
                buf.addString(arg);
            }

            argv.resize(4);
            argv[0] = m_extern_path.ptr();
            argv[1] = "exec";
            argv[2] = buf.ptr();
            argv[3] = 0;
        } else {
            if( m_mode == eExec_BG ) {
                *argv.add(1) = "nohup";
            }
            for(const char * arg = cmdline00; arg && *arg; arg = sString::next00(arg)) {
                *argv.add(1) = arg;
            }
            *argv.add(1) = 0;
        }

        if( execvp(argv[0], const_cast<char * const *>(argv.ptr())) < 0 ) {
            perror("sPS::exec00() ");
            exit(-1);
        }
        exit(0);
    }

    // inside parent
    int ret;
    if( waitpid(pid, &ret, 0) != pid ) {
        ret = -1;
    }

    // restore signals
    sigaction(SIGINT, &orig_act_intr, 0);
    sigaction(SIGQUIT, &orig_act_quit, 0);
    sigprocmask(SIG_SETMASK, &orig_sig_mask, 0);

    return ret;
#endif
}

// static
idx sPS::execute(const char * cmdline)
{
    sPS ps(eExec_Local);
    return ps.exec(cmdline);
}

// static
idx sPS::execute00(const char * cmdline00, sPS::callbackSetUp setupChild, void * param)
{
    sPS ps(eExec_Local);
    return ps.exec00(cmdline00, setupChild, param);
}

// static
idx sPS::executeBG(const char * cmdline)
{
    sPS ps(eExec_BG);
    return ps.exec(cmdline);
}

// static
idx sPS::executeBG00(const char * cmdline00, sPS::callbackSetUp setupChild, void * param)
{
    sPS ps(eExec_BG);
    return ps.exec00(cmdline00, setupChild, param);
}
