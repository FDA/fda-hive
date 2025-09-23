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
    #define SPIPE2_EXEC_SYS_FAIL(fmt, ...) ::fprintf(stderr, "sPipe2::execute: %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)
    #ifdef _DEBUG_TRACE_PIPE2
    #define SPIPE2_TRACE(fmt, ...) ::fprintf(stderr, "sPipe2: %s:%d, pid %d " fmt "\n", __FILE__, __LINE__, getpid(), __VA_ARGS__)
    #else
    #define SPIPE2_TRACE(...)
    #endif
#else
    #define SPIPE2_EXEC_SYS_FAIL(...)
    #define SPIPE2_TRACE(...)
#endif

sPipe2::CmdLine::CmdLine()
{
    _exe_pos = _dir_pos = -1;
}

sPipe2::CmdLine& sPipe2::CmdLine::operator=(const sPipe2::CmdLine & rhs)
{
    _exe_pos = rhs._exe_pos;
    _dir_pos = rhs._dir_pos;

    if( rhs._buf.length() ) {
        _buf.cutAddString(0, rhs._buf.ptr(), rhs._buf.length());
    } else {
        _buf.empty();
    }

    if( rhs._argv_pos.dim() ) {
        _argv_pos.cut(0);
        _argv_pos.add(rhs._argv_pos.dim());
        for(idx i = 0; i < rhs._argv_pos.dim(); i++) {
            _argv_pos[i] = rhs._argv_pos[i];
        }
    } else {
        _argv_pos.empty();
    }

    _env.empty();
    if( rhs._env.dim() ) {
        for(idx i = 0; i < rhs._env.dim(); i++) {
            idx name_len = 0;
            const char * name = static_cast<const char *>(rhs._env.id(i, &name_len));
            const idx * ppos = rhs._env.ptr(i);
            *_env.setString(name, name_len) = *ppos;
        }
    }

    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::exe(const char * exe_name)
{
    _exe_pos = _buf.length();
    _argv_pos.resize(1);
    _argv_pos[0] = _buf.length();
    _buf.addString(exe_name);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::vexe(const char * fmt, ...)
{
    _exe_pos = _buf.length();
    _argv_pos.resize(1);
    _argv_pos[0] = _buf.length();
    sCallVarg(_buf.vprintf, fmt);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::arg(const char * arg_val, idx len)
{
    *_argv_pos.add(1) = _buf.length();
    _buf.addString(arg_val, len);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::varg(const char * fmt, ...)
{
    *_argv_pos.add(1) = _buf.length();
    sCallVarg(_buf.vprintf, fmt);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::copyArgs(const CmdLine & rhs)
{
    for(idx i = 0; i < rhs._argv_pos.dim(); i++) {
        arg(rhs._buf.ptr(rhs._argv_pos[i]));
    }
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::cd(const char * dir)
{
    _dir_pos = _buf.length();
    _buf.addString(dir);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::env(const char * name, const char * value, idx value_len)
{
    *_env.setString(name) = _buf.length();
    _buf.addString(value, value_len);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::venv(const char * name, const char * value, ...)
{
    *_env.setString(name) = _buf.length();
    sCallVarg(_buf.vprintf, value);
    _buf.add0();
    return *this;
}

sPipe2::CmdLine & sPipe2::CmdLine::unsetEnv(const char * pattern)
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

const char * sPipe2::CmdLine::printBash(sStr * out) const
{
    if( !out ) {
        static sStr buf;
        out = &buf;
    }
    idx start_pos = out->length();
    out->add0cut();

    if( const char * dir = getStr(_dir_pos) ) {
        out->addString("cd ");
        sString::escapeForShell(*out, dir);
        out->addString(" && ");
    }

    bool weird_env_vars = false;
    for(idx i = 0; i < _env.dim() && !weird_env_vars; i++) {
        idx name_len = 0;
        const char * name = static_cast<const char *>(_env.id(i, &name_len));
        for(idx j = 0; j < name_len; j++) {
            if( !isalnum(name[j]) && name[j] != '_' ) {
                weird_env_vars = true;
                break;
            }
        }
    }
    if( weird_env_vars ) {
        out->addString("env ");
    }
    for(idx i = 0; i < _env.dim(); i++) {
        idx name_len = 0;
        const char * name = static_cast<const char *>(_env.id(i, &name_len));
        const char * value = getStr(*_env.ptr(i));
        if( name_len ) {
            if( weird_env_vars ) {
                sString::escapeForShell(*out, name, name_len);
            } else {
                out->addString(name, name_len);
            }
            out->addString("=");

            if( value && *value ) {
                sString::escapeForShell(*out, value);
            }
            out->addString(" ");
        }
    }

    for(idx i = 0; i < _argv_pos.dim(); i++) {
        if( i ) {
            out->addString(" ");
        }
        sString::escapeForShell(*out, getStr(_argv_pos[i]));
    }
    return out->ptr(start_pos);
}

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
            SPIPE2_TRACE("read %" DEC " bytes from fd %d", read_len, pipe_fds[0]);
            read_total += read_len;
            io->add(buf, read_len);
        } else if( read_len == 0 || errno == EAGAIN || errno == EWOULDBLOCK ) {
            SPIPE2_TRACE("read %" DEC " bytes from fd %d; breaking", read_len, pipe_fds[0]);
            break;
        } else {
            SPIPE2_TRACE("read %" DEC " bytes from fd %d; %s; closing pipe", read_len, pipe_fds[0], strerror(errno));
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
            break;
        } else {
            if( errno == EPIPE ) {
                struct timespec zerotime;
                zerotime.tv_sec = 0;
                zerotime.tv_nsec = 0;
                sigtimedwait(&sigpipeset, 0, &zerotime);
            }
            closePipeFD(1);
            break;
        }
    }

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

sPipe2::sPipe2(sPipe2::CmdLine * cmd_line) : _watch_entries(sMex::fExactSize)
{
    _stdin.reset();
    _stdout.reset();
    _stderr.reset();

    if( cmd_line ) {
        _cmd_line = cmd_line;
    } else {
        _cmd_line = &_local_cmd_line;
    }

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

sPipe2 & sPipe2::setStdOut(const char * file, bool append)
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

sPipe2 & sPipe2::setStdErr(const char * file, bool append)
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

sPipe2 & sPipe2::setStdOutErr(const char * file, bool append)
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

sPipe2 & sPipe2::setMonitor(sPipe2::monCb cb, const char * files00, void * param, real timeout_sec, bool always_call)
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

#define SELFPIPE_WAIT "w"
#define SELFPIPE_MON "m"
#define SELFPIPE_CPU_TIME "t"
#define SELFPIPE_DTOR "d"

bool sPipe2::kill(idx sig)
{
#ifdef SLIB_WIN
    return false;
#else

    bool ret = false;
    idx force_retcode = 0;

    if( _is_running ) {
        ret = (::kill(_mode == eDaemon ? -_pid : _pid, sig) == 0);

        if( sig && _mode == eDaemon ) {
            pthread_mutex_lock(&_waiter_timer_mtx);
            if( _waiter_tid > 0 ) {
                pthread_cancel(_waiter_tid);
                _is_running = false;
                force_retcode = 128 + sAbs(sig);
                ::write(_self_pipe_fds[1], SELFPIPE_WAIT, 1);
            }
            pthread_mutex_unlock(&_waiter_timer_mtx);
        }
    }

    if( sig ) {
        if( _callback_tid > 0 ) {
            pthread_join(_callback_tid, 0);
            _callback_tid = 0;
        }
    }

    if( force_retcode ) {
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
                    env_path = 0;
                    mode = eRelative;
                    SPIPE2_TRACE("constructing relative path resolver for %s", exe_name_);
                } else {
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

static int sLib_execvpe(const char * exe, char ** argv, char ** envp, const char * dir)
{
#ifdef SLIB_WIN
    return -1;
#else
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
        ::execve(abs_path, argv, envp);
    }

    int saved_errno = errno;
    SPIPE2_EXEC_SYS_FAIL("execve() failed: %s", strerror(errno));
    errno = saved_errno;

    switch(errno) {
        case EACCES:
        case ENOEXEC:
        case EISDIR:
            return 126;
        case ENOENT:
        case ENOTDIR:
        case ELIBBAD:
            return 127;
    }
    return 1;
#endif
}

#ifndef SLIB_WIN
void sPipe2::forkedChild()
{
    _stdin.setupForkedChild(0, stdin, _buf);
    _stdout.setupForkedChild(1, stdout, _buf);
    _stderr.setupForkedChild(1, stderr, _buf);

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
    sStr new_mutable_buf;
    if( _cmd_line->_env.dim() ) {
        sDic<idx> was_overridden_env;

        for(char ** pen = environ; pen && *pen; pen++) {
            const char * en = *pen;
            const char * equals = strchr(en, '=');
            idx name_len = equals ? equals - en : sLen(en);
            const char * name = name_len ? en : "";

            if( const idx * ppos = _cmd_line->_env.get(name, name_len) ) {
                if( *ppos >= 0 ) {
                    const char * new_value = _cmd_line->getStr(*ppos);
                    new_mutable_buf.addString(name, name_len);
                    new_mutable_buf.addString("=");
                    new_mutable_buf.add(new_value);
                    SPIPE2_TRACE("env: override %*s=%s", (int)name_len, name, new_value);

                    was_overridden_env.setString(name, name_len);
                } else {
                    SPIPE2_TRACE("env: unset %s", name);
                }
            } else {
                new_mutable_buf.add(en);
                SPIPE2_TRACE("env: keep %s", en);
            }
        }
        for(idx ie = 0; ie < _cmd_line->_env.dim(); ie++) {
            idx name_len = 0;
            const char * name = static_cast<const char *>(_cmd_line->_env.id(ie, &name_len));
            idx env_pos = *_cmd_line->_env.ptr(ie);
            if( env_pos >= 0 && name_len && !was_overridden_env.get(name, name_len) ) {
                const char * new_value = _cmd_line->getStr(env_pos);
                new_mutable_buf.addString(name, name_len);
                new_mutable_buf.addString("=");
                new_mutable_buf.add(new_value);
                SPIPE2_TRACE("env: add %s=%s", name, new_value);
            }
        }
        new_mutable_buf.add0(2);
        for(char * en = new_mutable_buf; en && *en; en = sString::next00(en)) {
            *new_env_vec.add(1) = en;
        }
        *new_env_vec.add(1) = 0;
        new_envp = new_env_vec.ptr();
    }

    sVec<char *> arg_vec(sMex::fExactSize);
    arg_vec.resize(_cmd_line->_argv_pos.dim() + 1);
    for(idx i = 0; i < _cmd_line->_argv_pos.dim(); i++) {
        arg_vec[i] = _cmd_line->getStr(_cmd_line->_argv_pos[i]);
    }
    arg_vec[_cmd_line->_argv_pos.dim()] = 0;

    if( _limit_cpu || _hard_limit_cpu ) {
        sSet(&lim);
        lim.rlim_cur = _limit_cpu;
        lim.rlim_max = _hard_limit_cpu ? _hard_limit_cpu : RLIM_SAVED_MAX;
        SPIPE2_TRACE("CPU limit: %" DEC " (%" DEC " hard)", _limit_cpu, _hard_limit_cpu);
        if( ::setrlimit(RLIMIT_CPU, &lim) ) {
            SPIPE2_TRACE("setrlimit() failed: %s", strerror(errno));
        }
    }

    if( _limit_heap || _hard_limit_heap ) {
        sSet(&lim);
        lim.rlim_cur = _limit_heap;
        lim.rlim_max = _hard_limit_heap ? _hard_limit_heap : RLIM_SAVED_MAX;
        SPIPE2_TRACE("Heap limit: %" DEC " (%" DEC " hard)", _limit_heap, _hard_limit_heap);
        if( ::setrlimit(RLIMIT_AS, &lim) || ::setrlimit(RLIMIT_DATA, &lim) ) {
            SPIPE2_TRACE("setrlimit() failed: %s", strerror(errno));
        }
    }

    if( int retcode = sLib_execvpe(_cmd_line->getStr(_cmd_line->_exe_pos), arg_vec.ptr(), new_envp, _cmd_line->getStr(_cmd_line->_dir_pos)) ) {
        exit(retcode);
    }

    exit(0);
}
#endif

sRC sPipe2::execute(sPipe2::ExecMode mode, sPipe2::doneCb cb, void * param)
{
#ifdef SLIB_WIN
    return sRC(sRC::eLaunching, sRC::eProcess, sRC::eOperation, sRC::eNotSupported);
#else
    if( mode == eDaemon ) {
        if( _stdin.io || _stdin.local_fd >= 0 || _stdout.io || _stdout.local_fd >= 0 || _stderr.io || _stderr.local_fd >= 0 ) {
            SPIPE2_EXEC_SYS_FAIL("%s", "invalid i/o redirection in daemon mode");
            return RC(sRC::eCreating, sRC::eProcess, sRC::eParameter, sRC::eInvalid);
        }
    }

    sRC rc;
    _mode = mode;
    _pid = 0;
    _is_running = false;
    _done_cb = cb;
    _done_cb_param = param;

    _retcode = 1;
    pid_t child_pid = -1;

    _stdin.want_pipe_fds = _stdin.io;
    if( _stdin.want_pipe_fds && !_stdin.openPipeFDs(false, true) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    _stderr.want_pipe_fds = _stderr.io;
    if( _stderr.want_pipe_fds && !_stderr.openPipeFDs(true, false) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    _stdout.want_pipe_fds = _stdout.io || mode == eDaemon;
    if( _stdout.want_pipe_fds && !_stdout.openPipeFDs(true, false) ) {
        SPIPE2_EXEC_SYS_FAIL("pipe() failed: %s", strerror(errno));
        RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
        goto FAIL;
    }

    child_pid = fork();

    if( child_pid < 0 ) {
        SPIPE2_EXEC_SYS_FAIL("fork() failed: %s", strerror(errno));
        RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::eOperation, sRC::eFailed);
    } else if( child_pid == 0 ) {

#ifdef SLIB_LINUX
        prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif

        _stdin.closePipeFD(1);
        _stdout.closePipeFD(0);
        _stderr.closePipeFD(0);

        if( mode == eDaemon ) {
            if( setsid() < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("setsid() failed: %s", strerror(errno));
                exit(-1);
            }

            pid_t daemon_pid = fork();
            if( daemon_pid < 0 ) {
                SPIPE2_EXEC_SYS_FAIL("fork() failed: %s", strerror(errno));
                exit(-1);
            } else if( daemon_pid == 0 ) {
                _stdout.closePipeFDs();
                forkedChild();
            } else {
                if( FILE * strm = ::fdopen(_stdout.pipe_fds[1], "w") ) {
                    fprintf(strm, "%" DEC, (idx)daemon_pid);
                    exit(0);
                } else {
                    SPIPE2_EXEC_SYS_FAIL("fdopen() failed: %s", strerror(errno));
                    exit(-1);
                }
            }
        } else {
            forkedChild();
        }
    } else {
        _pid = child_pid;
        _is_running = true;

        _stdin.closePipeFD(0);
        _stdout.closePipeFD(1);
        _stderr.closePipeFD(1);

        if( mode == eFG ) {
            if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
                goto FAIL;
            }

            if( pthread_create(&_waiter_tid, 0, waiterThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created waiter thread %lu", _waiter_tid);
            }

            if( _mon_cb || _limit_time ) {
                if( pthread_create(&_timer_tid, 0, timerThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created timer thread %lu", _timer_tid);
                }
            }

            handleCallbacks(false, &rc);
        } else if( mode == eBG ) {
            if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
                goto FAIL;
            }

            if( pthread_create(&_waiter_tid, 0, waiterThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created waiter thread %lu", _waiter_tid);
            }

            if( _mon_cb || _limit_time ) {
                if( pthread_create(&_timer_tid, 0, timerThread, (void*) this) == 0 ) {
                    SPIPE2_TRACE("created timer thread %lu", _timer_tid);
                }
            }

            if( pthread_create(&_callback_tid, 0, callbackThread, (void*) this) == 0 ) {
                SPIPE2_TRACE("created callback thread %lu", _callback_tid);
            }
        } else if( mode == eDaemon ) {
            if( FILE * strm = ::fdopen(_stdout.pipe_fds[0], "r") ) {
                ::fscanf(strm, "%" DEC, &_pid);
                ::fclose(strm);
                _stdout.pipe_fds[0] = -1;
                SPIPE2_TRACE("daemon PID is %" DEC, _pid);
            } else {
                SPIPE2_EXEC_SYS_FAIL("fdopen() failed: %s", strerror(errno));
                RCSET(rc, sRC::eWaiting, sRC::eProcess, sRC::eIO, sRC::eFailed);
                _stdout.closePipeFD(0);
            }

            if( ::waitpid(child_pid, &_retcode, 0) != child_pid ) {
                RCSET(rc, sRC::eWaiting, sRC::eProcess, sRC::eOperation, sRC::eFailed);
                if( !_retcode ) {
                    _retcode = 1;
                }
            }

            if( _mon_cb ) {
                if( !openPipeFDs(_self_pipe_fds, true, true) ) {
                    RCSET(rc, sRC::eCreating, sRC::eProcess, sRC::ePipe, sRC::eFailed);
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


    return rc;

  FAIL:

    kill();
    return rc;
#endif
}

void sPipe2::handleCallbacks(bool in_thread, sRC * out_rc)
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
        select_timeout.tv_usec = 100;

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

        if( _stderr.pipe_fds[0] >= 0 ) {
            _stderr.readFromPipe(buf.ptr(), buf_len);
        }
        if( _stdout.pipe_fds[0] >= 0 ) {
            if( !is_stdout_blocking && _stdin.pipe_fds[1] < 0 && _stderr.pipe_fds[0] < 0 ) {
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
                    if( _stderr.pipe_fds[0] >= 0 ) {
                        _stderr.readFromPipe(buf.ptr(), buf_len);
                    }
                    if( _stdout.pipe_fds[0] >= 0 ) {
                        _stdout.readFromPipe(buf.ptr(), buf_len);
                    }
                    if( _done_cb ) {
                        _done_cb(_retcode, _done_cb_param);
                    }
                    if( out_rc && out_rc->isUnset() && _retcode ) {
                        if( _retcode == 126 ) {
                            RCSETP(out_rc, sRC::eExecuting, sRC::eCommandLine, sRC::eFile, sRC::eInvalid);
                        } else if( _retcode == 127 ) {
                            RCSETP(out_rc, sRC::eExecuting, sRC::eCommandLine, sRC::eFile, sRC::eNotFound);
                        } else if( _retcode > 128 && _retcode <= 192 ) {
                            RCSETP(out_rc, sRC::eExecuting, sRC::eCommandLine, sRC::eProcess, sRC::eKilled);
                        } else {
                            RCSETP(out_rc, sRC::eExecuting, sRC::eCommandLine, sRC::eProcess, sRC::eFailed);
                        }
                    }
                    break;
                } else if( code == SELFPIPE_DTOR[0] ) {
                    break;
                } else if( code == SELFPIPE_CPU_TIME[0] ) {
                    SPIPE2_TRACE("kill %" DEC " -9", _mode == eDaemon ? -_pid : _pid);
                    if( ::kill(_mode == eDaemon ? -_pid : _pid, 9) ) {
                        SPIPE2_TRACE("kill failed: %s", strerror(errno));
                    }
                    if( out_rc && out_rc->isUnset() ) {
                        RCSETP(out_rc, sRC::eExecuting, sRC::eCommandLine, sRC::eTimeout, sRC::eExpired);
                    }
                } else if( code == SELFPIPE_MON[0] ) {
                    monitorFiles();
                }
            }
        }
    }

    _is_running = false;

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
void * sPipe2::waiterThread(void * param_)
{
    sPipe2 * self = static_cast<sPipe2*>(param_);
    pid_t self_pid = self->_pid;

    if( self->_mode == eDaemon ) {
        while( ::kill(self_pid, 0) == 0 ) {
            ::usleep(500 * 1000);
        }
    } else {
        int status = 0;
        pid_t ret_pid = waitpid(self_pid, &status, 0);
        if( WIFEXITED(status) ) {
            self->_retcode = WEXITSTATUS(status);
        } else if( WIFSIGNALED(status) ) {
            self->_retcode = 128 + WTERMSIG(status);
        } else {
            self->_retcode = 1;
        }

        if( ret_pid != self_pid ) {
            if( !self->_retcode ) {
                self->_retcode = 1;
            }
        }
    }

    SPIPE2_TRACE("waiter thread: waited for pid %" DEC "; return code = %d", self_pid, self->_retcode);

    self->_is_running = false;
    if( ::write(self->_self_pipe_fds[1], SELFPIPE_WAIT, 1) < 1 ) {
        SPIPE2_TRACE("waiter thread selfpipe write failed: %s", strerror(errno));
    }

    SPIPE2_TRACE("waiter thread exiting with code %d", self->_retcode);

    pthread_exit(0);
}

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
            break;
        } else if( self->_limit_time ) {
            cur_time = sTime::systime();
        }
    }

    SPIPE2_TRACE("%s", "timer thread exiting");

    pthread_exit(0);
}

void * sPipe2::callbackThread(void * param_)
{
    sPipe2 * self = static_cast<sPipe2*>(param_);

    self->handleCallbacks(true);

    pthread_exit(0);
}
#endif

sRC sPipe2::execute(idx * out_retcode)
{
    sRC rc = execute(eFG, 0, 0);
    if( out_retcode ) {
        *out_retcode = _retcode;
    }
    return rc;
}

sRC sPipe2::executeBG(sPipe2::doneCb cb, void * param)
{
    return execute(eBG, cb, param);
}

sRC sPipe2::executeDaemon(sPipe2::doneCb cb, void * param)
{
    return execute(eDaemon, cb, param);
}

sPipe2::~sPipe2()
{
    if( _is_running && _mode == eDaemon ) {

        pthread_mutex_lock(&_waiter_timer_mtx);
        if( _waiter_tid > 0 ) {
            pthread_cancel(_waiter_tid);
            _is_running = false;
            _retcode = 0;
            ::write(_self_pipe_fds[1], SELFPIPE_DTOR, 1);
        }
        pthread_mutex_unlock(&_waiter_timer_mtx);


        if( _callback_tid > 0 ) {
            pthread_join(_callback_tid, 0);
            _callback_tid = 0;
        }
    } else {
        kill();
    }

    pthread_mutex_destroy(&_waiter_timer_mtx);
}
