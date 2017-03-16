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

#include <slib/std.hpp>
#include <slib/std/pipe2.hpp>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef SLIB_LINUX
#include <sys/prctl.h>
#endif

using namespace slib;

extern char ** environ;

#define _DEBUG_TRACE_PIPE2_off

#ifdef _DEBUG
    #define SPIPE2_EXEC_SYS_FAIL(fmt, ...) ::fprintf(stderr, "sPipe2::execute: %s:%d: "fmt"\n", __FILE__, __LINE__, __VA_ARGS__)
    #ifdef _DEBUG_TRACE_PIPE2
    #define SPIPE2_TRACE(fmt, ...) ::fprintf(stderr, "sPipe2: %s:%d, pid %d "fmt"\n", __FILE__, __LINE__, getpid(), __VA_ARGS__)
    #else
    #define SPIPE2_TRACE(...)
    #endif
#else
    #define SPIPE2_EXEC_SYS_FAIL(...)
    #define SPIPE2_TRACE(...)
#endif

void sPipe2::IO::reset()
{
    io = NULL;
    path_pos = -1;
    local_fd = -1;
    discard = false;
    append = false;

    want_pipe_fds = false;
    pipe_fds[0] = pipe_fds[1] = -1;
}

//static
bool sPipe2::openPipeFDs(int pipe_fds[2], bool nonblock_read, bool nonblock_write)
{
    if( ::pipe(pipe_fds) < 0 ) {
        SPIPE2_TRACE("pipe(2) failed: %s", strerror(errno));
        return false;
    }
    SPIPE2_TRACE("pipe(2) success: %d -> %d", pipe_fds[1], pipe_fds[0]);
    if( nonblock_read ) {
        ::fcntl(pipe_fds[0], F_SETFL, fcntl(pipe_fds[0], F_GETFL) | O_NONBLOCK);
    }
    if( nonblock_write ) {
        ::fcntl(pipe_fds[1], F_SETFL, fcntl(pipe_fds[1], F_GETFL) | O_NONBLOCK);
    }
    SPIPE2_TRACE("pipe flags : [%d, %d]", fcntl(pipe_fds[0], F_GETFL), fcntl(pipe_fds[1], F_GETFL));
    return true;
}

bool sPipe2::IO::openPipeFDs(bool nonblock_read, bool nonblock_write)
{
    return sPipe2::openPipeFDs(pipe_fds, nonblock_read, nonblock_write);
}

//static
bool sPipe2::closePipeFD(int pipe_fds[2], idx i)
{
    if( pipe_fds[i] ) {
        ::close(pipe_fds[i]);
        pipe_fds[i] = -1;
        return true;
    } else {
        return false;
    }
}

bool sPipe2::IO::closePipeFD(idx i)
{
    return sPipe2::closePipeFD(pipe_fds, i);
}

//static
bool sPipe2::closePipeFDs(int pipe_fds[2])
{
    bool ret = false;
    if( closePipeFD(pipe_fds, 0) ) {
        ret = true;
    }
    if( closePipeFD(pipe_fds, 1) ) {
        ret = true;
    }
    return ret;
}

bool sPipe2::IO::closePipeFDs()
{
    return sPipe2::closePipeFDs(pipe_fds);
}

idx sPipe2::IO::readFromPipe(char * buf, idx buf_len)
{
    if( pipe_fds[0] < 0 ) {
        return -1;
    }

    idx read_total = 0;
    while( 1 ) {
        idx read_len = ::read(pipe_fds[0], buf, buf_len);
        if( read_len > 0 ) {
            SPIPE2_TRACE("read %"DEC" bytes from fd %d", read_len, pipe_fds[0]);
            read_total += read_len;
            io->add(buf, read_len);
        } else if( read_len == 0 || errno == EAGAIN || errno == EWOULDBLOCK ) {
            // more IO might come later
            SPIPE2_TRACE("read %"DEC" bytes from fd %d; breaking", read_len, pipe_fds[0]);
            break;
        } else {
            SPIPE2_TRACE("read %"DEC" bytes from fd %d; %s; closing pipe", read_len, pipe_fds[0], strerror(errno));
            closePipeFD(0);
            break;
        }
    }
    return read_total;
}

idx sPipe2::IO::writeToPipe(idx io_pos)
{
    if( pipe_fds[1] < 0 ) {
        return -1;
    }

    idx wrote_total = 0;

    // don't die on SIGPIPE from write(2)!
    sigset_t sigpipeset, oldsigset;
    sigemptyset(&sigpipeset);
    sigemptyset(&oldsigset);
    sigaddset(&sigpipeset, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &sigpipeset, &oldsigset);

    while( io_pos < io->length() ) {
        idx wrote_len = ::write(pipe_fds[1], io->ptr(io_pos), io->length() - io_pos);
        if( wrote_len > 0 ) {
            wrote_total += wrote_len;
        } else if( wrote_len == 0 || errno == EAGAIN || errno == EWOULDBLOCK ) {
            // more IO might come later
            break;
        } else {
            if( errno == EPIPE ) {
                // clear pending SIGPIPE
                struct timespec zerotime;
                zerotime.tv_sec = 0;
                zerotime.tv_nsec = 0;
                sigtimedwait(&sigpipeset, 0, &zerotime);
            }
            closePipeFD(1);
            break;
        }
    }

    // unblock SIGPIPE if needed
    pthread_sigmask(SIG_SETMASK, &oldsigset, 0);

    return wrote_total;
}

bool sPipe2::IO::setupForkedChild(idx i, FILE * stream, const sStr & path_buf)
{
    bool ret = true;
    int stream_fd = ::fileno(stream);
    const char * rwmode = i ? append ? "ab" : "wb" : "rb";
    ::fflush(stream);

    if( pipe_fds[i] >= 0 ) {
        SPIPE2_TRACE("Using pipe fd %d for %s on fd %d", pipe_fds[i], i ? "write" : "read", stream_fd);
        if( pipe_fds[i] != stream_fd ) {
            if( ::dup2(pipe_fds[i], stream_fd) < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("Failed to dup2(%d, %d): %s", pipe_fds[i], stream_fd, strerror(errno));
                ret = false;
            }
            closePipeFD(i);
        }
    } else if( local_fd >= 0 ) {
        SPIPE2_TRACE("Using local fd %d on fd %d", local_fd, stream_fd);
        if( local_fd != stream_fd ) {
            if( ::dup2(local_fd, stream_fd) < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("Failed to dup2(%d, %d): %s", local_fd, stream_fd, strerror(errno));
                ret = false;
            }
            if( ::close(local_fd) < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("Failed to close fd %d: %s", local_fd, strerror(errno));
                ret = false;
            }
            local_fd = -1;
        }
    } else if( path_pos >= 0 ) {
        const char * path = path_buf.ptr(path_pos);
        SPIPE2_TRACE("Redirecting fd %d to %s", stream_fd, path);
        if( ::freopen(path, rwmode, stream) == 0 ) {
            SPIPE2_EXEC_SYS_FAIL("Failed to reopen fd %d as '%s' in %s mode: %s", stream_fd, path, rwmode, strerror(errno));
            ret = false;
        }
    } else if( discard ) {
        SPIPE2_TRACE("Discarding fd %d", stream_fd);
        if( ::freopen("/dev/null", rwmode, stream) == 0 ) {
            SPIPE2_EXEC_SYS_FAIL("Failed to reopen fd %d as /dev/null in %s mode: %s", stream_fd, rwmode, strerror(errno));
            ret = false;
        }
    }
    return ret;
}

sPipe2::sPipe2() : _watch_entries(sMex::fExactSize)
{
    _stdin.reset();
    _stdout.reset();
    _stderr.reset();
    _dir_pos = -1;
    _exe_pos = -1;

    _stdin.discard = _stdout.discard = _stderr.discard = false;
    _limit_time = _limit_cpu = _hard_limit_cpu = 0;
    _limit_heap = _hard_limit_heap = 0;
    _mon_cb = 0;
    _mon_cb_param = 0;
    _mon_timeout_sec = 0;
    _mon_timeout_nsec = 0;
    _mon_cb_always_call = false;

    _pid = 0;
    _retcode = 0;
    _is_running = false;

    _mode = eFG;
    priv = 0;

    _done_cb = 0;
    _done_cb_param = 0;

#ifndef SLIB_WIN
    pthread_mutex_init(&_waiter_timer_mtx, 0);
    _self_pipe_fds[0] = _self_pipe_fds[1] = -1;
    _waiter_tid = _timer_tid = _callback_tid = 0;
#endif
}

sPipe2 & sPipe2::setStdIn(sIO * io)
{
    _stdin.io = io;
    return *this;
}

sPipe2 & sPipe2::setStdIn(const char * file)
{
    _stdin.path_pos = _buf.length();
    _buf.add(file);
    return *this;
}

sPipe2 & sPipe2::setStdIn(int file_desc)
{
    _stdin.local_fd = file_desc;
    return *this;
}

sPipe2 & sPipe2::setStdOut(sIO * io)
{
    _stdout.io = io;
    return *this;
}

sPipe2 & sPipe2::setStdOut(const char * file, bool append/*=false*/)
{
    _stdout.path_pos = _buf.length();
    _stdout.append = append;
    _buf.add(file);
    return *this;
}

sPipe2 & sPipe2::setStdOut(int file_desc)
{
    _stdout.local_fd = file_desc;
    return *this;
}

sPipe2 & sPipe2::discardStdOut()
{
    _stdout.io = 0;
    _stdout.path_pos = -1;
    _stdout.local_fd = -1;
    _stdout.discard = true;
    return *this;
}

sPipe2 & sPipe2::setStdErr(sIO * io)
{
    _stderr.io = io;
    return *this;
}

sPipe2 & sPipe2::setStdErr(const char * file, bool append/*=false*/)
{
    _stderr.path_pos = _buf.length();
    _stderr.append = append;
    _buf.add(file);
    return *this;
}

sPipe2 & sPipe2::setStdErr(int file_desc)
{
    _stderr.local_fd = file_desc;
    return *this;
}

sPipe2 & sPipe2::discardStdErr()
{
    _stderr.io = 0;
    _stderr.path_pos = -1;
    _stderr.local_fd = -1;
    _stderr.discard = true;
    return *this;
}

sPipe2 & sPipe2::setStdOutErr(sIO * io)
{
    _stdout.io = _stderr.io = io;
    return *this;
}

sPipe2 & sPipe2::setStdOutErr(const char * file, bool append/*=false*/)
{
    _stdout.path_pos = _stderr.path_pos = _buf.length();
    _stdout.append = _stderr.append = append;
    _buf.add(file);
    return *this;
}

sPipe2 & sPipe2::setStdOutErr(int file_desc)
{
    _stdout.local_fd = _stderr.local_fd = file_desc;
    return *this;
}

sPipe2 & sPipe2::setDir(const char * dir)
{
    _dir_pos = _buf.length();
    _buf.addString(dir);
    _buf.add0();
    return *this;
}

sPipe2 & sPipe2::setEnv(const char * name, const char * value, idx value_len/* = 0*/)
{
    *_env.setString(name) = _buf.length();
    _buf.addString(value, value_len);
    _buf.add0();
    return *this;
}

sPipe2 & sPipe2::unsetEnv(const char * pattern/* = "*" */)
{
    sFileGlob gl;
    if( gl.compile(pattern, true) ) {
        for(idx i = 0; i < _env.dim(); i++) {
            const char * name = static_cast<const char *>(_env.id(i));
            if( gl.match(name) ) {
                *_env.setString(name) = -1;
            }
        }

        sStr name;
        for(char ** pen = environ; pen && *pen; pen++) {
            const char * en = *pen;
            const char * name_end = strchr(en, '=');
            idx name_len = name_end ? name_end - en : sLen(en);
            name.cutAddString(0, name_len ? en : "", name_len);
            if( gl.match(name) ) {
                *_env.setString(name) = -1;
            }
        }
    }
    return *this;
}

sPipe2 & sPipe2::limitTime(idx sec)
{
    _limit_time = sec;
    return *this;
}

sPipe2 & sPipe2::limitCPU(idx sec)
{
    _limit_cpu = sec;
    return *this;
}

sPipe2 & sPipe2::hardLimitCPU(idx sec)
{
    _hard_limit_cpu = sec;
    _limit_cpu = sClamp<idx>(_limit_cpu, 1, sec, sec);
    return *this;
}

sPipe2 & sPipe2::limitMem(idx size)
{
    _limit_heap = size;
    return *this;
}

sPipe2 & sPipe2::hardLimitMem(idx size)
{
    _hard_limit_heap = size;
    _limit_heap = sClamp<idx>(_limit_heap, 1, size, size);
    return *this;
}

sPipe2 & sPipe2::setMonitor(sPipe2::monCb cb, const char * files00, void * param/* = 0 */, real timeout_sec/* = 5*/, bool always_call/* = false*/)
{
    _mon_cb = cb;
    _mon_cb_param = param;
    _mon_timeout_sec = timeout_sec;
    _mon_timeout_nsec = 1000000000 * (timeout_sec - _mon_timeout_sec);
    _mon_cb_always_call = always_call;

    _watch_entries.cut(0);
    idx cnt = files00 && files00[0] ? sString::cnt00(files00) : 0;
    _watch_entries.resize(cnt);

    for(idx i = 0; i < cnt; i++) {
        _watch_entries[i].path_pos = _buf.length();
        sSet(&_watch_entries[i].prev_stat);
        _buf.addString(files00);
        _buf.add0();
        files00 = sString::next00(files00);
    }

    return *this;
}

sPipe2 & sPipe2::setExe(const char * exe)
{
    _exe_pos = _buf.length();
    _argv_pos.cut(0);
    *_argv_pos.add(1) = _buf.length();
    _buf.addString(exe);
    _buf.add0();

    return *this;
}

sPipe2 & sPipe2::addArg(const char * arg, idx len/* = 0 */)
{
    *_argv_pos.add(1) = _buf.length();
    _buf.addString(arg, len);
    _buf.add0();
    return *this;
}

// messages for use in selfpipe
#define SELFPIPE_WAIT "w"
#define SELFPIPE_MON "m"
#define SELFPIPE_CPU_TIME "t"
#define SELFPIPE_DTOR "d"

bool sPipe2::kill(idx sig/* = -9 */)
{
#ifdef SLIB_WIN
    // TODO
    return false;
#else

    bool ret = false;
    idx force_retcode = 0;

    // kill a daemon's entire process group; otherwise, only kill the process itself
    if( _is_running ) {
        ret = (::kill(_mode == eDaemon ? -_pid : _pid, sig) == 0);

        // in daemon mode, the waiter thread works by polling. Instead of waiting for it to
        // discover that the process is killed, cancel the waiter and immediately tell the callback
        // thread over self-pipe.
        if( sig && _mode == eDaemon ) {
            pthread_mutex_lock(&_waiter_timer_mtx);
            if( _waiter_tid > 0 ) {
                pthread_cancel(_waiter_tid);
                _is_running = false;
                force_retcode = 128 + sAbs(sig); // same logic as bash, see http://unix.stackexchange.com/questions/99112/default-exit-code-when-process-is-terminated
                ::write(_self_pipe_fds[1], SELFPIPE_WAIT, 1);
            }
            pthread_mutex_unlock(&_waiter_timer_mtx);
        }
    }

    if( sig ) {
        // handleCallbacks() will reap waiter and timer threads, but the callback thread should be reaped by us here
        if( _callback_tid > 0 ) {
            pthread_join(_callback_tid, 0);
            _callback_tid = 0;
        }
    }

    if( force_retcode ) {
        // set _retcode after other threads have finished
        _retcode = force_retcode;
    }

    return ret;
#endif
}

static const char * sLib_getenv_PATH()
{
    if( const char * env = getenv("PATH") ) {
        return env;
    }

#ifdef _CS_PATH
    static sStr cs_path_buf;
    static idx cs_path_len = confstr(_CS_PATH, 0, 0);

    if( cs_path_len ) {
        if( !cs_path_buf.length() ) {
            cs_path_buf.resize(cs_path_len + 1);
            confstr(_CS_PATH, cs_path_buf.ptr(), cs_path_len + 1);
        }
        return cs_path_buf.ptr();
    }
#endif

    return "/bin:/usr/bin";
}

namespace {
    struct ExePathResolver {
        sStr buf;
        idx cwd_len;
        const char * env_path;
        const char * exe_name;
        enum {
            eAbsolute,
            eRelative,
            eEnvPath
        } mode;

        idx getCurPathLen(bool with_colon)
        {
            const char * colon = strchr(env_path, ':');
            if( colon ) {
                if( with_colon ) {
                    colon++;
                }
                return colon - env_path;
            } else {
                return sLen(env_path);
            }
        }

        ExePathResolver(const char * exe_name_)
        {
            exe_name = exe_name_;
            if( exe_name && exe_name[0] != sDir::sysSep ) {
                if( strchr(exe_name, sDir::sysSep) ) {
                    // exe_name is a relative path in current working directory
                    env_path = 0;
                    mode = eRelative;
                    SPIPE2_TRACE("constructing relative path resolver for %s", exe_name_);
                } else {
                    // exe_name is just the filename, iterate over PATH env variable to find it
                    env_path = sLib_getenv_PATH();
                    mode = eEnvPath;
                    SPIPE2_TRACE("constructing env path resolver for %s with PATH='%s'", exe_name_, env_path);
                }

                buf.resize(PATH_MAX);
                buf[0] = 0;
                if( getcwd(buf.ptr(), PATH_MAX) ) {
                    buf.cut(sLen(buf));
                    if( buf[buf.length() - 1] != sDir::sysSep ) {
                        buf.addString(&sDir::sysSep, 1);
                    }
                    cwd_len = buf.length();
                } else {
                    cwd_len = -1;
                }
            } else {
                // exe_name is an absolute path
                mode = eAbsolute;
                SPIPE2_TRACE("constructing absolute resolver for %s", exe_name_);
            }
        }

        bool ok()
        {
            switch(mode) {
                case eAbsolute:
                    return exe_name;
                case eRelative:
                    return exe_name && cwd_len >= 0;
                case eEnvPath:
                    return exe_name && (cwd_len >= 0 || env_path[0] == sDir::sysSep);
            }
            return false;
        }

        const char * getAbsPath()
        {
            switch(mode) {
                case eAbsolute:
                    return exe_name;
                case eRelative:
                    if( cwd_len < 0 ) {
                        return 0;
                    }
                    buf.cutAddString(cwd_len, exe_name);
                    return buf.ptr(0);
                case eEnvPath:
                {
                    if( cwd_len < 0 && env_path[0] != sDir::sysSep ) {
                        return 0;
                    }
                    idx path_pos = sMax<idx>(0, cwd_len);

                    if( idx path_len = getCurPathLen(false) ) {
                        buf.cutAddString(path_pos, env_path, path_len);
                    } else {
                        buf.cut0cut(path_pos);
                    }

                    if( buf[buf.length() - 1] != sDir::sysSep ) {
                        buf.addString(&sDir::sysSep, 1);
                    }
                    buf.addString(exe_name);
                    if( env_path[0] == sDir::sysSep ) {
                        return buf.ptr(path_pos);
                    } else {
                        return buf.ptr(0);
                    }
                }
            }
            return 0;
        }

        const char * nextAbsPath()
        {
            if( mode == eEnvPath && env_path && env_path[0] ) {
                env_path += getCurPathLen(true);
                if( env_path[0] ) {
                    return getAbsPath();
                }
            }
            return 0;
        }
    };
};

// this function exists because (1) for some reason there is no execvpe(3) on OSX, and
// (2) because we want to resolve exe's abspath using the original current working
// directory, not the new directory to which we switched in executeBG()
static int sLib_execvpe(const char * exe, char ** argv, char ** envp, const char * dir)
{
#ifdef SLIB_WIN
    // TODO
    return -1;
#else
    // construct path resolver before changing directory to properly resolve relative paths
    // as relative to initial working directory
    ExePathResolver path_resolver(exe);

    if( dir && !sDir::chDir(dir) ) {
        SPIPE2_EXEC_SYS_FAIL("chdir() failed: %s", strerror(errno));
        return -1;
    }

    if( !path_resolver.ok() ) {
        SPIPE2_EXEC_SYS_FAIL("path resolver for %s failed", exe ? exe : "(null)");
        errno = ENOENT;
        return -1;
    }

#ifdef _DEBUG
    sStr printbuf;
    for(char ** a = argv; *a; a++) {
        sString::escapeForShell(printbuf, *a);
        printbuf.addString(" ");
    }
#endif

    for(const char * abs_path = path_resolver.getAbsPath(); abs_path; abs_path = path_resolver.nextAbsPath()) {
        SPIPE2_TRACE("execve() on %s, argv = %s", abs_path, printbuf.ptr());
        ::execve(abs_path, argv, envp); // on success, does not return
    }

    SPIPE2_EXEC_SYS_FAIL("execve() failed: %s", strerror(errno));
    return -1;
#endif
}

#ifndef SLIB_WIN
void sPipe2::forkedChild()
{
    // redirect pipes
    _stdin.setupForkedChild(0, stdin, _buf);
    _stdout.setupForkedChild(1, stdout, _buf);
    _stderr.setupForkedChild(1, stderr, _buf);

    // close any extraneuous file descriptors
    int max_fd = 0;
    struct rlimit lim;
    sSet(&lim);
    if( getrlimit(RLIMIT_NOFILE, &lim) == 0 ) {
        max_fd = lim.rlim_max;
    } else {
        max_fd = sysconf(_SC_OPEN_MAX);
    }
    for(int fd = STDERR_FILENO + 1; fd < max_fd; fd++) {
        if( ::close(fd) == 0 ) {
            SPIPE2_TRACE("closed extraneous fd %d", fd);
        }
    }

    char ** new_envp = environ;
    sVec<char *> new_env_vec;
    sStr new_env_buf;
    if( _env.dim() ) {
        // filter parent env
        for(char ** pen = environ; pen && *pen; pen++) {
            const char * en = *pen;
            const char * equals = strchr(en, '=');
            idx name_len = equals ? equals - en : sLen(en);
            const char * name = name_len ? en : "";

            if( idx * ppos = _env.get(name, name_len) ) {
                // we have an override for this env variable
                if( *ppos >= 0 ) {
                    // .. and the override didn't unset this env variable
                    const char * new_value = getStr(*ppos);
                    new_env_buf.addString(name, name_len);
                    new_env_buf.addString("=");
                    new_env_buf.add(new_value);
                    SPIPE2_TRACE("env: override %*s=%s", (int)name_len, name, new_value);

                    *ppos = -1; // mark as done
                } else {
                    SPIPE2_TRACE("env: unset %s", name);
                }
            } else {
                // no override for this env variable, keep from parent env
                new_env_buf.add(en);
                SPIPE2_TRACE("env: keep %s", en);
            }
        }
        // add variables not present in parent env
        for(idx ie = 0; ie < _env.dim(); ie++) {
            if( _env[ie] >= 0 ) {
                const char * name = static_cast<const char *>(_env.id(ie));
                const char * new_value = getStr(_env[ie]);
                new_env_buf.addString(name);
                new_env_buf.addString("=");
                new_env_buf.add(new_value);
                SPIPE2_TRACE("env: add %s=%s", name, new_value);
            }
        }
        new_env_buf.add0(2);
        for(char * en = new_env_buf; en && *en; en = sString::next00(en)) {
            *new_env_vec.add(1) = en;
        }
        *new_env_vec.add(1) = 0;
        new_envp = new_env_vec.ptr();
    }

    sVec<char *> arg_vec(sMex::fExactSize);
    arg_vec.resize(_argv_pos.dim() + 1);
    for(idx i = 0; i < _argv_pos.dim(); i++) {
        arg_vec[i] = getStr(_argv_pos[i]);
    }
    arg_vec[_argv_pos.dim()] = 0;

    if( _limit_cpu || _hard_limit_cpu ) {
        sSet(&lim);
        lim.rlim_cur = _limit_cpu;
        lim.rlim_max = _hard_limit_cpu ? _hard_limit_cpu : RLIM_SAVED_MAX;
        SPIPE2_TRACE("CPU limit: %"DEC" (%"DEC" hard)", _limit_cpu, _hard_limit_cpu);
        if( ::setrlimit(RLIMIT_CPU, &lim) ) {
            SPIPE2_TRACE("setrlimit() failed: %s", strerror(errno));
        }
    }

    if( _limit_heap || _hard_limit_heap ) {
        sSet(&lim);
        lim.rlim_cur = _limit_heap;
        lim.rlim_max = _hard_limit_heap ? _hard_limit_heap : RLIM_SAVED_MAX;
        SPIPE2_TRACE("Heap limit: %"DEC" (%"DEC" hard)", _limit_heap, _hard_limit_heap);
        // set both RLIMIT_AS and RLIMIT_DATA, since which one is relevant depends on the operating system's malloc() implementation
        // http://stackoverflow.com/questions/23768601/posix-rlimit-what-exactly-can-we-assume-about-rlimit-data
        if( ::setrlimit(RLIMIT_AS, &lim) || ::setrlimit(RLIMIT_DATA, &lim) ) {
            SPIPE2_TRACE("setrlimit() failed: %s", strerror(errno));
        }
    }

    if( sLib_execvpe(getStr(_exe_pos), arg_vec.ptr(), new_envp, getStr(_dir_pos)) < 0 ) {
        exit(-1);
    }

    exit(0);
}
#endif

sRC sPipe2::execute(sPipe2::ExecMode mode, sPipe2::doneCb cb, void * param)
{
#ifdef SLIB_WIN
    // TODO
    return sRC(sRC::eLaunching, sRC::eProcess, sRC::eOperation, sRC::eNotSupported);
#else
    // sanity check: if daemonizing, input redirection to callback or open file descriptor doesn't make sense
    if( mode == eDaemon ) {
        if( _stdin.io || _stdin.local_fd >= 0 || _stdout.io || _stdout.local_fd >= 0 || _stderr.io || _stderr.local_fd >= 0 ) {
            SPIPE2_EXEC_SYS_FAIL("%s", "invalid i/o redirection in daemon mode");
            return sRC(sRC::eCreating, sRC::eProcess, sRC::eParameter, sRC::eInvalid);
        }
    }

    sRC rc;
    _mode = mode;
    _pid = 0;
    _is_running = false;
    _done_cb = cb;
    _done_cb_param = param;

    _retcode = -1;
    pid_t child_pid = -1;

    _stdin.want_pipe_fds = _stdin.io;
    // stdin: blocking on reading side, non-blocking on writing side; so spawn will wait for our
    // stdin callback to finish, and we can use select() to see when spawn is ready to read from
    // the callback.
    if( _stdin.want_pipe_fds && !_stdin.openPipeFDs(false, true) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        rc.set(sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    _stderr.want_pipe_fds = _stderr.io;
    // stderr: blocking on writing side; we cannot allow spawn to overflow the kernel's pipe buffer
    if( _stderr.want_pipe_fds && !_stderr.openPipeFDs(true, false) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        rc.set(sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    // in daemon mode, we will need a pipe to retrieve daemon's pid
    _stdout.want_pipe_fds = _stdout.io || mode == eDaemon;
    // stdout: blocking on writing side; we cannot allow spawn to overflow the kernel's pipe buffer
    if( _stdout.want_pipe_fds && !_stdout.openPipeFDs(true, false) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        rc.set(sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    child_pid = fork();

    if( child_pid < 0 ) {
        // fork failed!
        SPIPE2_EXEC_SYS_FAIL("fork() failed: %s", strerror(errno));
        rc.set(sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
    } else if( child_pid == 0 ) {
        // inside forked child

#ifdef SLIB_LINUX
        // linux-only: die if parent is killed
        // http://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits
        prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif

        // close unnecessary pipe fds
        _stdin.closePipeFD(1);
        _stdout.closePipeFD(0);
        _stderr.closePipeFD(0);

        if( mode == eDaemon ) {
            // create a new session and process group, then fork again!
            if( setsid() < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("setsid() failed: %s", strerror(errno));
                exit(-1);
            }

            pid_t daemon_pid = fork();
            if( daemon_pid < 0 ) {
                // second fork failed!
                SPIPE2_EXEC_SYS_FAIL("fork() failed: %s", strerror(errno));
                exit(-1);
            } else if( daemon_pid == 0 ) {
                // daemonized (second forked) child
                _stdout.closePipeFDs();
                forkedChild(); // does not return
            } else {
                // original forked child, which needs to inform parent process about daemon_pid and then exit
                // POSIX says pipe writes of at least PIPE_BUF bytes are atomic, and PIPE_BUF >= 512
                if( FILE * strm = ::fdopen(_stdout.pipe_fds[1], "w") ) {
                    fprintf(strm, "%"DEC, (idx)daemon_pid);
                    exit(0);
                } else {
                    SPIPE2_EXEC_SYS_FAIL("fdopen() failed: %s", strerror(errno));
                    exit(-1);
                }
            }
        } else {
            forkedChild(); // does not return
        }
    } else {
        // inside parent on successful fork
        _pid = child_pid;
        _is_running = true;

        // close unnecessary pipe fds
        _stdin.closePipeFD(0);
        _stdout.closePipeFD(1);
        _stderr.closePipeFD(1);

        if( mode == eFG ) {
            // open a self-pipe
            if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                rc.set(sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
                goto FAIL;
            }

            // use a thread to waitpid so we can select() on it
            if( pthread_create(&_waiter_tid, 0, waiterThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created waiter thread %lu", _waiter_tid);
            }

            // timer for file monitoring and CPU limit
            if( _mon_cb || _limit_time ) {
                if( pthread_create(&_timer_tid, 0, timerThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created timer thread %lu", _timer_tid);
                }
            }

            // run any callbacks in the foreground
            handleCallbacks(false);
        } else if( mode == eBG ) {
            // open a self-pipe
            if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                rc.set(sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
                goto FAIL;
            }

            // wait for background process to finish in a thread
            if( pthread_create(&_waiter_tid, 0, waiterThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created waiter thread %lu", _waiter_tid);
            }

            // timer for file monitoring and CPU limit
            if( _mon_cb || _limit_time ) {
                if( pthread_create(&_timer_tid, 0, timerThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created timer thread %lu", _timer_tid);
                }
            }

            // handle any callbacks in a thread
            if( pthread_create(&_callback_tid, 0, callbackThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created callback thread %lu", _callback_tid);
            }
        } else if( mode == eDaemon ) {
            // read daemon pid from child's stdout pipe
            if( FILE * strm = ::fdopen(_stdout.pipe_fds[0], "r") ) {
                ::fscanf(strm, "%"DEC, &_pid);
                ::fclose(strm);
                _stdout.pipe_fds[0] = -1;
                SPIPE2_TRACE("daemon PID is %"DEC, _pid);
            } else {
                SPIPE2_EXEC_SYS_FAIL("fdopen() failed: %s", strerror(errno));
                rc.set(sRC::eWaiting, sRC::eProcess, sRC::eIO, sRC::eFailed);
                _stdout.closePipeFD(0);
            }

            // reap temporary child process
            if( ::waitpid(child_pid, &_retcode, 0) != child_pid ) {
                rc.set(sRC::eWaiting, sRC::eProcess, sRC::eOperation, sRC::eFailed);
                _retcode = -1;
            }

            // monitor daemon process in a thread, if needed
            if( _mon_cb ) {
                // open a self-pipe
                if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                    rc.set(sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
                    goto FAIL;
                }

                if( pthread_create(&_timer_tid, 0, timerThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created timer thread %lu", _timer_tid);
                }

                if( pthread_create(&_callback_tid, 0, callbackThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created callback thread %lu", _callback_tid);
                }
            }
        }
    }

    // either inside parent on successful fork, or on fork failure

    return rc;

  FAIL:

    kill();
    return rc;
#endif
}

// handles callbacks, reaps timer and waiter thread when done
void sPipe2::handleCallbacks(bool in_thread)
{
    fd_set rfds, wfds;
    struct timeval select_timeout;
    bool is_stdout_blocking = false;

    sStr buf;
#ifdef _DEBUG_TRACE_PIPE2
    sStr debug_lst_wfds, debug_lst_rfds;
#endif
    idx stdin_pos = 0;
    static const idx buf_len = sysconf(_SC_PAGESIZE);
    buf.resize(buf_len);
    while( 1 ) {
        int nfds = -1;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 100; // sleep for up to 100 microseconds before retrying to read spawn's stdout

#ifdef _DEBUG_TRACE_PIPE2
        debug_lst_wfds.cut0cut();
        debug_lst_rfds.cut0cut();
#endif

        if( _stdin.pipe_fds[1] >= 0 ) {
            if( stdin_pos >= _stdin.io->length() ) {
                _stdin.closePipeFDs();
            } else {
                nfds = sMax<int>(nfds, _stdin.pipe_fds[1] + 1);
                FD_SET(_stdin.pipe_fds[1], &wfds);
#ifdef _DEBUG_TRACE_PIPE2
                debug_lst_wfds.addString("stdin");
#endif
            }
        }
        if( _self_pipe_fds[0] >= 0 ) {
            nfds = sMax<int>(nfds, _self_pipe_fds[0] + 1);
            FD_SET(_self_pipe_fds[0], &rfds);
#ifdef _DEBUG_TRACE_PIPE2
            debug_lst_rfds.addString("self_pipe");
#endif
        } else {
            SPIPE2_TRACE("%s", "selfpipe not available");
        }

        // greedy read from stder/stdout; we cannot select() on them because they are blocking on spawn side
        // and spawn won't start writing until we read; we cannot make them non-blocking on spawn side because
        // a naive spawn that writes faster than we can read will overflow the kernel's pipe buffer.
        if( _stderr.pipe_fds[0] >= 0 ) {
            _stderr.readFromPipe(buf.ptr(), buf_len);
        }
        if( _stdout.pipe_fds[0] >= 0 ) {
            if( !is_stdout_blocking && _stdin.pipe_fds[1] < 0 && _stderr.pipe_fds[0] < 0 ) {
                // common case optimization: if there is no more stdin being read or stderr captured, we
                // can put stdout into blocking mode to avoid the non-blocking-read / spin cycle.
                ::fcntl(_stdout.pipe_fds[0], F_SETFL, fcntl(_stdout.pipe_fds[0], F_GETFL) | O_NONBLOCK);
                is_stdout_blocking = true;
            }
            _stdout.readFromPipe(buf.ptr(), buf_len);
        }

        if( nfds <= 0 && _stderr.pipe_fds[0] < 0 && _stdout.pipe_fds[0] < 0 ) {
            SPIPE2_TRACE("%s", "No file descriptors available for selecting on or reading from");
            break;
        }

        SPIPE2_TRACE("selecting on %d max fd; reading: %s; writing: %s", nfds, debug_lst_rfds.length() ? debug_lst_rfds.ptr() : "none", debug_lst_wfds.length() ? debug_lst_wfds.ptr() : "none");
        idx nselected = select(nfds, &rfds, &wfds, NULL, &select_timeout);
        // don't allow thread cancellation in the middle of a callback
        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if( nselected < 0 ) {
            SPIPE2_TRACE("%s", "No file descriptors got selected");
            break;
        } else if( nselected > 0 ) {
            if( _stdin.pipe_fds[1] >= 0 && FD_ISSET(_stdin.pipe_fds[1], &wfds) ) {
                SPIPE2_TRACE("%s", "stdin selected");
                stdin_pos += _stdin.writeToPipe(stdin_pos);
            }
            if( _self_pipe_fds[0] >= 0 && FD_ISSET(_self_pipe_fds[0], &rfds) ) {
                SPIPE2_TRACE("%s", "selfpipe selected");
                char code = 0;
                ::read(_self_pipe_fds[0], &code, 1);
                SPIPE2_TRACE("selfpipe says '%c'", code);
                if( code == SELFPIPE_WAIT[0] ) {
                    // launched process has terminated; _retcode is guaranteed to have been set
                    // first, handle any unflushed output from the child
                    if( _stderr.pipe_fds[0] >= 0 ) {
                        _stderr.readFromPipe(buf.ptr(), buf_len);
                    }
                    if( _stdout.pipe_fds[0] >= 0 ) {
                        _stdout.readFromPipe(buf.ptr(), buf_len);
                    }
                    // then run _done_cb and stop
                    if( _done_cb ) {
                        _done_cb(_retcode, _done_cb_param);
                    }
                    break;
                } else if( code == SELFPIPE_DTOR[0] ) {
                    // daemon mode, sPipe2 is being destructed, process will continue running
                    break;
                } else if( code == SELFPIPE_CPU_TIME[0] ) {
                    // launched process has run out of time; kill. Don't return right now - waiter thread neads to waitpid() it
                    // ::kill() instead of sPipe2::kill() because sPipe2::kill() should not be used from callback thread
                    SPIPE2_TRACE("kill %"DEC" -9", _mode == eDaemon ? -_pid : _pid);
                    if( ::kill(_mode == eDaemon ? -_pid : _pid, 9) ) {
                        SPIPE2_TRACE("kill failed: %s", strerror(errno));
                    }
                } else if( code == SELFPIPE_MON[0] ) {
                    // time to scan for changes in monitored files
                    monitorFiles();
                }
            }
        }
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

    _is_running = false;

    // If callbacks are done, we can stop the timer
    pthread_mutex_lock(&_waiter_timer_mtx);
    if( _timer_tid > 0 ) {
        SPIPE2_TRACE("Canceling timer thread %lu", _timer_tid);
        pthread_cancel(_timer_tid);
        SPIPE2_TRACE("Joining timer thread %lu", _timer_tid);
        pthread_join(_timer_tid, 0);
        _timer_tid = 0;
    }
    if( _waiter_tid > 0 ) {
        SPIPE2_TRACE("Joining waiter thread %lu", _waiter_tid);
        pthread_join(_waiter_tid, 0);
        _waiter_tid = 0;
    }

    _stdin.closePipeFDs();
    _stdout.closePipeFDs();
    _stderr.closePipeFDs();
    closePipeFDs(_self_pipe_fds);

    pthread_mutex_unlock(&_waiter_timer_mtx);

    return;
}

void sPipe2::monitorFiles()
{
    idx cntchanged = 0;
    for(idx i = 0; i < _watch_entries.dim(); i++) {
        struct stat st;
        struct stat * prev_st = &_watch_entries[i].prev_stat;
        const char * path = getStr(_watch_entries[i].path_pos);
        sSet(&st);
        if( ::stat(path, &st) != 0 ) {
            sSet(&st);
        }
        if( st.st_size != prev_st->st_size || st.st_mtime != prev_st->st_mtime || st.st_ino != prev_st->st_ino ) {
            SPIPE2_TRACE("calling monitor on '%s'", path);
            if( !_mon_cb(_pid, path, &st, _mon_cb_param) ) {
                SPIPE2_TRACE("monitor on '%s' returned false, killing process", path);
                kill();
            }
            memcpy(prev_st, &st, sizeof(st));
            cntchanged++;
        }
    }

    if( !cntchanged && _mon_cb_always_call ) {
        SPIPE2_TRACE("%s", "calling monitor on NULL (no files changed)");
        if( !_mon_cb(_pid, 0, 0, _mon_cb_param) ) {
            SPIPE2_TRACE("%s", "monitor on NULL returned false, killing process");
            kill();
        }
    }
}

#ifndef SLIB_WIN
//static
void * sPipe2::waiterThread(void * param_)
{
    sPipe2 * self = static_cast<sPipe2*>(param_);
    idx self_pid = self->_pid;

    if( self->_mode == eDaemon ) {
        // daemon process is not our child, so we must poll for it to finish
        // TODO on Linux, replace polling with netlink-based proc monitor + bpf filter
        // TODO see http://netsplit.com/the-proc-connector-and-socket-filters
        while( ::kill(self_pid, 0) == 0 ) {
            ::usleep(500 * 1000); // 0.5 seconds = 500k microseconds
        }
    } else if( waitpid(self_pid, &self->_retcode, 0) != self_pid ) {
        self->_retcode = -1;
    }

    SPIPE2_TRACE("waiter thread: waited for pid %"DEC"; return code = %d", self_pid, self->_retcode);

    self->_is_running = false;
    if( ::write(self->_self_pipe_fds[1], SELFPIPE_WAIT, 1) < 1 ) {
        SPIPE2_TRACE("waiter thread selfpipe write failed: %s", strerror(errno));
    }

    SPIPE2_TRACE("waiter thread exiting with code %d", self->_retcode);

    pthread_exit(0);
}

//static
void * sPipe2::timerThread(void * param_)
{
    sPipe2 * self = static_cast<sPipe2*>(param_);

    idx cur_time = 0, end_time = 0;
    if( self->_limit_time ) {
        cur_time = sTime::systime();
        end_time = cur_time + self->_limit_time;
    }

    while( 1 ) {
        const char * code = SELFPIPE_MON;
        struct timespec sleep_timeout;
        sleep_timeout.tv_sec = self->_mon_timeout_sec;
        sleep_timeout.tv_nsec = self->_mon_timeout_nsec;
        if( self->_limit_time && (!self->_mon_cb || sleep_timeout.tv_sec > end_time - cur_time) ) {
            sleep_timeout.tv_sec = end_time - cur_time;
            sleep_timeout.tv_nsec = 0;
            code = SELFPIPE_CPU_TIME;
        }
        ::nanosleep(&sleep_timeout, 0);
        if( ::write(self->_self_pipe_fds[1], code, 1) < 1 ) {
            SPIPE2_TRACE("timer thread selfpipe write failed: %s", strerror(errno));
        }

        if( code[0] == SELFPIPE_CPU_TIME[0] ) {
            // we've just told handleCallbacks() to kill the process - our thread's job is done
            break;
        } else if( self->_limit_time ) {
            cur_time = sTime::systime();
        }
    }

    SPIPE2_TRACE("%s", "timer thread exiting");

    pthread_exit(0);
}

//static
void * sPipe2::callbackThread(void * param_)
{
    sPipe2 * self = static_cast<sPipe2*>(param_);

    self->handleCallbacks(true);

    pthread_exit(0);
}
#endif

sRC sPipe2::execute(idx * out_retcode/* = 0 */)
{
    sRC rc = execute(eFG, 0, 0);
    if( out_retcode ) {
        *out_retcode = _retcode;
    }
    return rc;
}

sRC sPipe2::executeBG(sPipe2::doneCb cb/* = 0 */, void * param/* = 0 */)
{
    return execute(eBG, cb, param);
}

sRC sPipe2::executeDaemon(sPipe2::doneCb cb/* = 0 */, void * param/* = 0 */)
{
    return execute(eDaemon, cb, param);
}

sPipe2::~sPipe2()
{
    if( _is_running && _mode == eDaemon ) {
        // cancel threads but don't kill the process

        pthread_mutex_lock(&_waiter_timer_mtx);
        if( _waiter_tid > 0 ) {
            pthread_cancel(_waiter_tid);
            _is_running = false;
            _retcode = 0;
            ::write(_self_pipe_fds[1], SELFPIPE_DTOR, 1);
        }
        pthread_mutex_unlock(&_waiter_timer_mtx);

        // handleCallbacks will take care of reaping timer and waiter

        if( _callback_tid > 0 ) {
            pthread_join(_callback_tid, 0);
            _callback_tid = 0;
        }
    } else {
        kill();
    }

    pthread_mutex_destroy(&_waiter_timer_mtx);
}
