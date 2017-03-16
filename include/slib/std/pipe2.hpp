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
#pragma once
#ifndef sLib_std_pipe2_hpp
#define sLib_std_pipe2_hpp

#include <slib/core/dic.hpp>
#include <slib/core/vec.hpp>
#include <slib/core/rc.hpp>
#include <slib/core/sIO.hpp>

#include <sys/stat.h>

namespace slib {
    class sPipe2 {
        public:
            sPipe2();
            ~sPipe2();
            //! redirect command's standard input
            sPipe2 & setStdIn(sIO * io);
            sPipe2 & setStdIn(const char * file);
            sPipe2 & setStdIn(int file_desc);
            //! redirect command's standard output
            sPipe2 & setStdOut(sIO * io);
            sPipe2 & setStdOut(const char * file, bool append = false);
            sPipe2 & setStdOut(int file_desc);
            sPipe2 & discardStdOut();
            //! redirect command's standard error
            sPipe2 & setStdErr(sIO * io);
            sPipe2 & setStdErr(const char * file, bool append = false);
            sPipe2 & setStdErr(int file_desc);
            sPipe2 & discardStdErr();
            //! redirect command's standard output and error to the same thing
            sPipe2 & setStdOutErr(sIO * io);
            sPipe2 & setStdOutErr(const char * file, bool append = false);
            sPipe2 & setStdOutErr(int file_desc);

            //! set current working directory for command
            sPipe2 & setDir(const char * dir);
            //! (re)define an environment variable for the command
            sPipe2 & setEnv(const char * name, const char * value, idx value_len = 0);
            //! unset existing environment variables whose names match a glob pattern
            sPipe2 & unsetEnv(const char * pattern = "*");

            //! file or directory monitor callback; return true to continue, false to stop process
            typedef bool (*monCb)(idx pid, const char * path, struct stat * st, void * param);
            //! define a file or directory monitor callback
            /*! \param cb callback to call; runs in the foreground when executing in foreground mode, or in a new thread when executing in background or daemon mode
                \param paths00 00-list of files or directories to monitor; cb will be called if these files' or directories' timestamps or sizes change; 0 to ignore
                \param param parameter for cb
                \param timeout_sec files will be monitored with this frequency; or if always_call is true, callback will always be called with this frequency
                \param always_call always call the callback, even if no files/dirs changed */
            sPipe2 & setMonitor(monCb cb, const char * paths00, void * param = 0, real timeout_sec = 5, bool always_call = false);

            //! limit total process run time
            sPipe2 & limitTime(idx sec);
            //! limit max CPU time
            sPipe2 & limitCPU(idx sec);
            //! soft-limit max CPU time
            sPipe2 & hardLimitCPU(idx sec);
            //! limit max size of heap and data segment
            sPipe2 & limitMem(idx size);
            //! soft-limit max size of heap and data segment
            sPipe2 & hardLimitMem(idx size);

            //! specify name or path of the executable
            sPipe2 & setExe(const char * exe);
            //! add a command-line argument
            sPipe2 & addArg(const char * arg, idx len = 0);

            //! execute a command specified by setExe() / addArg(), and wait until it exits
            sRC execute(idx * out_retcode = 0);

            typedef void (*doneCb)(idx retcode, void * param);

            //! execute a command specified by setExe() / addArg() in the background, and call a callback when it exits
            sRC executeBG(doneCb cb = 0, void * param = 0);

            //! spawn a command specified by setExe() / addArg() as a detached daemon, and call a callback if it exits before our process finishes
            sRC executeDaemon(doneCb cb = 0, void * param = 0);

            //! check if executed command is still running
            bool running() const { return _is_running; }
            //! retrieve PID of command (former PID, if it is no longer running)
            idx pid() const { return _pid; }
            //! retrieve return code (if process is finished)
            idx retcode() const { return _retcode; }
            //! kill the command if it's still running
            bool kill(idx sig = 9);

        private:
            enum ExecMode {
                eFG,
                eBG,
                eDaemon
            };

            sStr _buf;
            char * getStr(idx pos) { return pos >= 0 ? _buf.ptr(pos) : 0; }
            sDic<idx> _env;
            sVec<idx> _argv_pos;
            idx _exe_pos;
            idx _dir_pos;
            struct IO {
                sIO * io;
                idx path_pos;
                int local_fd;
                bool discard;
                bool append;

                bool want_pipe_fds;
                int pipe_fds[2];

                void reset();
                bool openPipeFDs(bool nonblock_read, bool nonblock_write);
                bool closePipeFD(idx i);
                bool closePipeFDs();
                bool setupForkedChild(idx i, FILE * stream, const sStr & path_buf);
                idx readFromPipe(char * buf, idx buf_len);
                idx writeToPipe(idx io_pos);
            };
            IO _stdin, _stdout, _stderr;

            idx _limit_time;
            idx _limit_cpu, _hard_limit_cpu;
            idx _limit_heap, _hard_limit_heap;

            struct MonEntry {
                idx path_pos;
                struct stat prev_stat;
            };
            sVec<MonEntry> _watch_entries;

            monCb _mon_cb;
            void * _mon_cb_param;
            idx _mon_timeout_sec;
            idx _mon_timeout_nsec;
            bool _mon_cb_always_call;

            doneCb _done_cb;
            void * _done_cb_param;

            idx _pid;
            int _retcode;
            bool _is_running;

            ExecMode _mode;
            void * priv;

            sRC execute(ExecMode mode, doneCb cb, void * param);
            void handleCallbacks(bool in_thread);
            void monitorFiles();

            static bool openPipeFDs(int pipe_fds[2], bool nonblock_read, bool nonblock_write);
            static bool closePipeFD(int pipe_fds[2], idx i);
            static bool closePipeFDs(int pipe_fds[2]);

#ifndef SLIB_WIN
            pthread_mutex_t _waiter_timer_mtx;
            pthread_t _waiter_tid; // waits for process termination, reaps zombies; reaped in handleCallbacks()
            pthread_t _timer_tid; // timer for file monitoring and CPU limit; reaped in handleCallbacks()
            pthread_t _callback_tid; // handles callbacks if in background mode
            int _self_pipe_fds[2];

            void forkedChild() __attribute__ ((noreturn));
            static void * waiterThread(void * param_);
            static void * timerThread(void * param_);
            static void * callbackThread(void * param_);
#endif
    };
};

#endif
