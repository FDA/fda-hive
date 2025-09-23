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
            class CmdLine {
                public:
                    CmdLine();
                    CmdLine & operator=(const CmdLine & rhs);

                    CmdLine & exe(const char * exe_name);
                    CmdLine & vexe(const char * fmt, ...);
                    CmdLine & arg(const char * arg_val, idx len = 0);
                    CmdLine & varg(const char * fmt, ...);
                    CmdLine & copyArgs(const CmdLine & rhs);
                    CmdLine & cd(const char * dir);
                    CmdLine & env(const char * name, const char * value, idx value_len = 0);
                    CmdLine & venv(const char * name, const char * value, ...);
                    CmdLine & unsetEnv(const char * pattern = "*");

                    const char * printBash(sStr * out = 0) const;

                private:
                    friend class sPipe2;
                    const char * getStr(idx pos) const
                    {
                        return pos >= 0 ? _buf.ptr(pos) : 0;
                    }
                    char * getStr(idx pos)
                    {
                        return pos >= 0 ? _buf.ptr(pos) : 0;
                    }
                    sStr _buf;
                    sDic<idx> _env;
                    sVec<idx> _argv_pos;
                    idx _exe_pos;
                    idx _dir_pos;
            };

            sPipe2(CmdLine * cmd_line = 0);
            ~sPipe2();

            CmdLine & cmdLine() { return *_cmd_line; }
            const CmdLine & cmdLine() const { return *_cmd_line; }

            sPipe2 & setStdIn(sIO * io);
            sPipe2 & setStdIn(const char * file);
            sPipe2 & setStdIn(int file_desc);
            sPipe2 & setStdOut(sIO * io);
            sPipe2 & setStdOut(const char * file, bool append = false);
            sPipe2 & setStdOut(int file_desc);
            sPipe2 & discardStdOut();
            sPipe2 & setStdErr(sIO * io);
            sPipe2 & setStdErr(const char * file, bool append = false);
            sPipe2 & setStdErr(int file_desc);
            sPipe2 & discardStdErr();
            sPipe2 & setStdOutErr(sIO * io);
            sPipe2 & setStdOutErr(const char * file, bool append = false);
            sPipe2 & setStdOutErr(int file_desc);

            typedef bool (*monCb)(idx pid, const char * path, struct stat * st, void * param);
            sPipe2 & setMonitor(monCb cb, const char * paths00, void * param = 0, real timeout_sec = 5, bool always_call = false);

            sPipe2 & limitTime(idx sec);
            sPipe2 & limitCPU(idx sec);
            sPipe2 & hardLimitCPU(idx sec);
            sPipe2 & limitMem(idx size);
            sPipe2 & hardLimitMem(idx size);

            sRC execute(idx * out_retcode = 0);

            typedef void (*doneCb)(idx retcode, void * param);

            sRC executeBG(doneCb cb = 0, void * param = 0);

            sRC executeDaemon(doneCb cb = 0, void * param = 0);

            bool running() const { return _is_running; }
            idx pid() const { return _pid; }
            idx retcode() const { return _retcode; }
            bool kill(idx sig = 9);

        private:
            enum ExecMode {
                eFG,
                eBG,
                eDaemon
            };

            CmdLine * _cmd_line;
            CmdLine _local_cmd_line;

            sStr _buf;
            char * getStr(idx pos) { return pos >= 0 ? _buf.ptr(pos) : 0; }

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
            void handleCallbacks(bool in_thread, sRC * out_rc = 0);
            void monitorFiles();

            static bool openPipeFDs(int pipe_fds[2], bool nonblock_read, bool nonblock_write);
            static bool closePipeFD(int pipe_fds[2], idx i);
            static bool closePipeFDs(int pipe_fds[2]);

#ifndef SLIB_WIN
            pthread_mutex_t _waiter_timer_mtx;
            pthread_t _waiter_tid;
            pthread_t _timer_tid;
            pthread_t _callback_tid;
            int _self_pipe_fds[2];

            void forkedChild() __attribute__ ((noreturn));
            static void * waiterThread(void * param_);
            static void * timerThread(void * param_);
            static void * callbackThread(void * param_);
#endif
    };
};

#endif
