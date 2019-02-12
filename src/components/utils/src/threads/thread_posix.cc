/*
 * Copyright (c) 2015, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <signal.h>
#include <pthread.h>
#include <algorithm>
#include <functional>

#include "utils/threads/thread.h"
#include "utils/atomic.h"
#include "utils/threads/thread_delegate.h"
#include "utils/logger.h"

#ifndef __QNXNTO__
const int EOK = 0;
#endif

#if defined(OS_POSIX)
const size_t THREAD_NAME_SIZE = 15;
#endif

namespace threads {

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

size_t Thread::kMinStackSize =
    PTHREAD_STACK_MIN; /* Ubuntu : 16384 ; QNX : 256; */

void Thread::cleanup(void* arg) {
  LOG4CXX_AUTO_TRACE(logger_);
  Thread* thread = reinterpret_cast<Thread*>(arg);
  sync_primitives::AutoLock auto_lock(thread->state_lock_);
  thread->isThreadRunning_ = false;
  thread->state_cond_.Broadcast();
}

void* Thread::threadFunc(void* arg) {
  // 0 - state_lock unlocked
  //     stopped   = 0
  //     running   = 0
  //     finalized = 0
  // 4 - state_lock unlocked
  //     stopped = 1
  //     running = 1
  //     finalized = 0
  // 5 - state_lock unlocked
  //     stopped = 1
  //     running = 1
  //     finalized = 1
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  threads::Thread* thread = reinterpret_cast<Thread*>(arg);
  DCHECK(thread);

  pthread_cleanup_push(&cleanup, thread);

  {
  	/*
  	 This is not a lock for acquiring another lock.
	 This code part ensures correct notification sequence between Thread::start and threadFunc.
	 In Thread::start function after thread is created, we call Wait(),
	 but we cannot guarantee order of precedence of the following calls :
	 "Wait() in Thread::start" or "Broadcast in threadFunc".
	 sync_primitives::AutoLock auto_lock(thread->run_lock_); - is used to guarantee call to
	 "Wait() in Thread::start" before Broadcast.
    */
    sync_primitives::AutoLock auto_lock(thread->run_lock_);
    thread->state_lock_.Acquire();
  }

  thread->state_cond_.Broadcast();

  while (!thread->finalized_) {
    LOG4CXX_DEBUG(logger_, "Thread #" << pthread_self() << " iteration");
    thread->state_cond_.Wait(thread->state_lock_);
    LOG4CXX_DEBUG(logger_,
                  "Thread #" << pthread_self() << " execute. "
                             << "stopped_ = " << thread->stopped_
                             << "; finalized_ = " << thread->finalized_);
    if (!thread->stopped_ && !thread->finalized_) {
      thread->isThreadRunning_ = true;
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      {
        sync_primitives::AutoUnlock auto_unlock(thread->state_lock_);
        thread->delegate_->threadMain();
      }
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      thread->isThreadRunning_ = false;
    }

    {
      sync_primitives::AutoUnlock auto_unlock(thread->state_lock_);
      thread->state_cond_.Broadcast();
    }
    LOG4CXX_DEBUG(logger_,
                  "Thread #" << pthread_self() << " finished iteration");
  }

  thread->state_lock_.Release();
  pthread_cleanup_pop(1);

  LOG4CXX_DEBUG(logger_,
                "Thread #" << pthread_self() << " exited successfully");
  return NULL;
}

void Thread::SetNameForId(const PlatformThreadHandle& thread_id,
                          std::string name) {
  if (name.size() > THREAD_NAME_SIZE)
    name.erase(THREAD_NAME_SIZE);
  const int rc = pthread_setname_np(thread_id, name.c_str());
  if (rc != EOK) {
    LOG4CXX_WARN(logger_,
                 "Couldn't set pthread name \"" << name << "\", error code "
                                                << rc << " (" << strerror(rc)
                                                << ")");
  }
}

Thread::Thread(const char* name, ThreadDelegate* delegate)
    : name_(name ? name : "undefined")
    , delegate_(delegate)
    , handle_(0)
    , thread_options_()
    , isThreadRunning_(0)
    , stopped_(false)
    , finalized_(false)
    , thread_created_(false) {}

bool Thread::start() {
  return start(thread_options_);
}

PlatformThreadHandle Thread::CurrentId() {
  return pthread_self();
}

bool Thread::IsCurrentThread() const {
  return pthread_equal(CurrentId(), thread_handle());
}

bool Thread::start(const ThreadOptions& options) {
  LOG4CXX_AUTO_TRACE(logger_);
  pthread_attr_t attributes;
  int pthread_result = -1;
  {
    sync_primitives::AutoLock auto_lock(run_lock_);
    // 1 - run_lock_ locked
    //     stopped = 0
    //     running = 0

    if (!delegate_) {
      LOG4CXX_ERROR(logger_,
                    "Cannot start thread " << name_ << ": delegate is NULL");
      // 0 - run_lock_ unlocked
      return false;
    }

    if (isThreadRunning_) {
      LOG4CXX_TRACE(logger_,
                    "EXIT thread " << name_ << " #" << handle_
                                  << " is already running");
      return true;
    }

    thread_options_ = options;

    pthread_result = pthread_attr_init(&attributes);
    if (pthread_result != EOK) {
      LOG4CXX_WARN(logger_,
                  "Couldn't init pthread attributes. Error code = "
                      << pthread_result << " (\"" << strerror(pthread_result)
                      << "\")");
    }

    if (!thread_options_.is_joinable()) {
      pthread_result =
          pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
      if (pthread_result != EOK) {
        LOG4CXX_WARN(logger_,
                    "Couldn't set detach state attribute. Error code = "
                        << pthread_result << " (\"" << strerror(pthread_result)
                        << "\")");
        thread_options_.is_joinable(false);
      }
    }

    const size_t stack_size = thread_options_.stack_size();
    if (stack_size >= Thread::kMinStackSize) {
      pthread_result = pthread_attr_setstacksize(&attributes, stack_size);
      if (pthread_result != EOK) {
        LOG4CXX_WARN(logger_,
                    "Couldn't set stacksize = "
                        << stack_size << ". Error code = " << pthread_result
                        << " (\"" << strerror(pthread_result) << "\")");
      }
    } else {
      ThreadOptions thread_options_temp(Thread::kMinStackSize,
                                        thread_options_.is_joinable());
      thread_options_ = thread_options_temp;
    }

    if (!thread_created_) {
      // run_lock_ 1
      pthread_result = pthread_create(&handle_, &attributes, threadFunc, this);
      if (pthread_result == EOK) {
        LOG4CXX_DEBUG(logger_, "Created thread: " << name_);
        SetNameForId(handle_, name_);
        // run_lock_ 0
        // possible concurrencies: stop and threadFunc
        state_cond_.Wait(auto_lock);
        thread_created_ = true;
      } else {
        LOG4CXX_ERROR(logger_,
                      "Couldn't create thread "
                          << name_ << ". Error code = " << pthread_result
                          << " (\"" << strerror(pthread_result) << "\")");
      }
    }
  }

  {
    sync_primitives::AutoLock auto_lock(state_lock_);
    stopped_ = false;
  }

  state_cond_.NotifyOne();
  LOG4CXX_DEBUG(logger_,
                "Thread " << name_ << " #" << handle_ << " started."
                          << " pthread_result = " << pthread_result);
  pthread_attr_destroy(&attributes);
  return pthread_result == EOK;
}

void Thread::yield() {
  sched_yield();
}

void Thread::stop() {
  LOG4CXX_AUTO_TRACE(logger_);
  sync_primitives::AutoLock auto_lock(state_lock_);

  stopped_ = true;

  LOG4CXX_DEBUG(logger_,
                "Stopping thread #" << handle_ << " \"" << name_ << "\"");

  if (delegate_ && isThreadRunning_) {
    delegate_->exitThreadMain();
  }

  LOG4CXX_DEBUG(logger_,
                "Stopped thread #" << handle_ << " \"" << name_ << " \"");
}

void Thread::join() {
  LOG4CXX_AUTO_TRACE(logger_);
  DCHECK_OR_RETURN_VOID(!IsCurrentThread());

  stop();

  state_cond_.NotifyOne();
  sync_primitives::AutoLock auto_lock(state_lock_);
  if (isThreadRunning_) {
    if (!pthread_equal(pthread_self(), handle_)) {
      LOG4CXX_DEBUG(logger_,
                    "Waiting for #" << handle_
                                    << " finished iteration in thread #"
                                    << pthread_self());
      state_cond_.Wait(auto_lock);
    }
  }
}

Thread::~Thread() {
  finalized_ = true;
  stopped_ = true;
  join();
  // in some platforms pthread_join behaviour is undefined when thread is
  // not created(pthread_create) and call pthread_join.
  if (handle_) {
    pthread_join(handle_, NULL);
  }
}

Thread* CreateThread(const char* name, ThreadDelegate* delegate) {
  Thread* thread = new Thread(name, delegate);
  delegate->set_thread(thread);
  return thread;
}

void DeleteThread(Thread* thread) {
  delete thread;
}

}  // namespace threads
