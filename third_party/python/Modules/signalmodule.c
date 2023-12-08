/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:8;coding:utf-8 -*-│
│ vi: set et ft=c ts=4 sts=4 sw=4 fenc=utf-8                               :vi │
╞══════════════════════════════════════════════════════════════════════════════╡
│ Python 3                                                                     │
│ https://docs.python.org/3/license.html                                       │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/calls/calls.h"
#include "libc/calls/struct/itimerval.h"
#include "libc/calls/struct/sigset.h"
#include "libc/dce.h"
#include "libc/errno.h"
#include "libc/math.h"
#include "libc/sysv/consts/itimer.h"
#include "libc/sysv/consts/sig.h"
#include "libc/thread/thread.h"
#include "libc/time/time.h"
#include "third_party/python/Include/abstract.h"
#include "third_party/python/Include/ceval.h"
#include "third_party/python/Include/dictobject.h"
#include "third_party/python/Include/fileutils.h"
#include "third_party/python/Include/floatobject.h"
#include "third_party/python/Include/import.h"
#include "third_party/python/Include/intrcheck.h"
#include "third_party/python/Include/longobject.h"
#include "third_party/python/Include/modsupport.h"
#include "third_party/python/Include/pgenheaders.h"
#include "third_party/python/Include/pyatomic.h"
#include "third_party/python/Include/pyerrors.h"
#include "third_party/python/Include/pylifecycle.h"
#include "third_party/python/Include/pymacro.h"
#include "third_party/python/Include/setobject.h"
#include "third_party/python/Include/tupleobject.h"
#include "third_party/python/Include/yoink.h"
#include "third_party/python/Modules/posixmodule.h"
#include "third_party/python/pyconfig.h"

PYTHON_PROVIDE("_signal");
PYTHON_PROVIDE("_signal.ITIMER_PROF");
PYTHON_PROVIDE("_signal.ITIMER_REAL");
PYTHON_PROVIDE("_signal.ITIMER_VIRTUAL");
PYTHON_PROVIDE("_signal.ItimerError");
PYTHON_PROVIDE("_signal.NSIG");
PYTHON_PROVIDE("_signal.SIGABRT");
PYTHON_PROVIDE("_signal.SIGALRM");
PYTHON_PROVIDE("_signal.SIGBUS");
PYTHON_PROVIDE("_signal.SIGCHLD");
PYTHON_PROVIDE("_signal.SIGCONT");
PYTHON_PROVIDE("_signal.SIGFPE");
PYTHON_PROVIDE("_signal.SIGHUP");
PYTHON_PROVIDE("_signal.SIGILL");
PYTHON_PROVIDE("_signal.SIGINT");
PYTHON_PROVIDE("_signal.SIGIO");
PYTHON_PROVIDE("_signal.SIGIOT");
PYTHON_PROVIDE("_signal.SIGKILL");
PYTHON_PROVIDE("_signal.SIGPIPE");
PYTHON_PROVIDE("_signal.SIGPOLL");
PYTHON_PROVIDE("_signal.SIGPROF");
PYTHON_PROVIDE("_signal.SIGPWR");
PYTHON_PROVIDE("_signal.SIGQUIT");
PYTHON_PROVIDE("_signal.SIGRTMAX");
PYTHON_PROVIDE("_signal.SIGRTMIN");
PYTHON_PROVIDE("_signal.SIGSEGV");
PYTHON_PROVIDE("_signal.SIGSTOP");
PYTHON_PROVIDE("_signal.SIGSYS");
PYTHON_PROVIDE("_signal.SIGTERM");
PYTHON_PROVIDE("_signal.SIGTRAP");
PYTHON_PROVIDE("_signal.SIGTSTP");
PYTHON_PROVIDE("_signal.SIGTTIN");
PYTHON_PROVIDE("_signal.SIGTTOU");
PYTHON_PROVIDE("_signal.SIGURG");
PYTHON_PROVIDE("_signal.SIGUSR1");
PYTHON_PROVIDE("_signal.SIGUSR2");
PYTHON_PROVIDE("_signal.SIGVTALRM");
PYTHON_PROVIDE("_signal.SIGWINCH");
PYTHON_PROVIDE("_signal.SIGXCPU");
PYTHON_PROVIDE("_signal.SIGXFSZ");
PYTHON_PROVIDE("_signal.SIG_BLOCK");
PYTHON_PROVIDE("_signal.SIG_DFL");
PYTHON_PROVIDE("_signal.SIG_IGN");
PYTHON_PROVIDE("_signal.SIG_SETMASK");
PYTHON_PROVIDE("_signal.SIG_UNBLOCK");
PYTHON_PROVIDE("_signal.alarm");
PYTHON_PROVIDE("_signal.default_int_handler");
PYTHON_PROVIDE("_signal.getitimer");
PYTHON_PROVIDE("_signal.getsignal");
PYTHON_PROVIDE("_signal.pause");
PYTHON_PROVIDE("_signal.pthread_kill");
PYTHON_PROVIDE("_signal.pthread_sigmask");
PYTHON_PROVIDE("_signal.set_wakeup_fd");
PYTHON_PROVIDE("_signal.setitimer");
PYTHON_PROVIDE("_signal.siginterrupt");
PYTHON_PROVIDE("_signal.signal");
PYTHON_PROVIDE("_signal.sigpending");
PYTHON_PROVIDE("_signal.sigtimedwait");
PYTHON_PROVIDE("_signal.sigwait");
PYTHON_PROVIDE("_signal.sigwaitinfo");
PYTHON_PROVIDE("_signal.struct_siginfo");

/* Signal module -- many thanks to Lance Ellinghaus */

/* XXX Signals should be recorded per thread, now we have thread state. */

#if defined(HAVE_PTHREAD_SIGMASK) && !defined(HAVE_BROKEN_PTHREAD_SIGMASK)
#  define PYPTHREAD_SIGMASK
#endif

#include "third_party/python/Modules/clinic/signalmodule.inc"

/*[clinic input]
module signal
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=b0301a3bde5fe9d3]*/


/*
   NOTES ON THE INTERACTION BETWEEN SIGNALS AND THREADS

   When threads are supported, we want the following semantics:

   - only the main thread can set a signal handler
   - any thread can get a signal handler
   - signals are only delivered to the main thread

   I.e. we don't support "synchronous signals" like SIGFPE (catching
   this doesn't make much sense in Python anyway) nor do we support
   signals as a means of inter-thread communication, since not all
   thread implementations support that (at least our thread library
   doesn't).

   We still have the problem that in some implementations signals
   generated by the keyboard (e.g. SIGINT) are delivered to all
   threads (e.g. SGI), while in others (e.g. Solaris) such signals are
   delivered to one random thread (an intermediate possibility would
   be to deliver it to the main thread -- POSIX?).  For now, we have
   a working implementation that works in all three cases -- the
   handler ignores signals if getpid() isn't the same as in the main
   thread.  XXX This is a hack.
*/

#ifdef WITH_THREAD
#include "third_party/python/Include/pythread.h"
static long main_thread;
static pid_t main_pid;
#endif

static volatile struct {
    _Py_atomic_int tripped;
    PyObject *func;
} Handlers[Py_NSIG];

#ifdef MS_WINDOWS
#define INVALID_FD ((SOCKET_T)-1)

static volatile struct {
    SOCKET_T fd;
    int use_send;
    int send_err_set;
    int send_errno;
    int send_win_error;
} wakeup = {INVALID_FD, 0, 0};
#else
#define INVALID_FD (-1)
static volatile sig_atomic_t wakeup_fd = -1;
#endif

/* Speed up sigcheck() when none tripped */
static _Py_atomic_int is_tripped;

static PyObject *DefaultHandler;
static PyObject *IgnoreHandler;
static PyObject *IntHandler;

#ifdef MS_WINDOWS
static HANDLE sigint_event = NULL;
#endif

#ifdef HAVE_GETITIMER
static PyObject *ItimerError;

/* auxiliary functions for setitimer/getitimer */
static void
timeval_from_double(double d, struct timeval *tv)
{
    tv->tv_sec = floor(d);
    tv->tv_usec = fmod(d, 1.0) * 1000000.0;
    /* Don't disable the timer if the computation above rounds down to zero. */
    if (d > 0.0 && tv->tv_sec == 0 && tv->tv_usec == 0) {
        tv->tv_usec = 1;
    }
}

Py_LOCAL_INLINE(double)
double_from_timeval(struct timeval *tv)
{
    return tv->tv_sec + (double)(tv->tv_usec / 1000000.0);
}

static PyObject *
itimer_retval(struct itimerval *iv)
{
    PyObject *r, *v;

    r = PyTuple_New(2);
    if (r == NULL)
        return NULL;

    if(!(v = PyFloat_FromDouble(double_from_timeval(&iv->it_value)))) {
        Py_DECREF(r);
        return NULL;
    }

    PyTuple_SET_ITEM(r, 0, v);

    if(!(v = PyFloat_FromDouble(double_from_timeval(&iv->it_interval)))) {
        Py_DECREF(r);
        return NULL;
    }

    PyTuple_SET_ITEM(r, 1, v);

    return r;
}
#endif

static PyObject *
signal_default_int_handler(PyObject *self, PyObject *args)
{
    PyErr_SetNone(PyExc_KeyboardInterrupt);
    return NULL;
}

PyDoc_STRVAR(default_int_handler_doc,
"default_int_handler(...)\n\
\n\
The default handler for SIGINT installed by Python.\n\
It raises KeyboardInterrupt.");


static int
report_wakeup_write_error(void *data)
{
    int save_errno = errno;
    errno = (int) (intptr_t) data;
    PyErr_SetFromErrno(PyExc_OSError);
    PySys_WriteStderr("Exception ignored when trying to write to the "
                      "signal wakeup fd:\n");
    PyErr_WriteUnraisable(NULL);
    errno = save_errno;
    return 0;
}

#ifdef MS_WINDOWS
static int
report_wakeup_send_error(void* Py_UNUSED(data))
{
    PyObject *res;

    if (wakeup.send_win_error) {
        /* PyErr_SetExcFromWindowsErr() invokes FormatMessage() which
           recognizes the error codes used by both GetLastError() and
           WSAGetLastError */
        res = PyErr_SetExcFromWindowsErr(PyExc_OSError, wakeup.send_win_error);
    }
    else {
        errno = wakeup.send_errno;
        res = PyErr_SetFromErrno(PyExc_OSError);
    }

    assert(res == NULL);
    wakeup.send_err_set = 0;

    PySys_WriteStderr("Exception ignored when trying to send to the "
                      "signal wakeup fd:\n");
    PyErr_WriteUnraisable(NULL);

    return 0;
}
#endif   /* MS_WINDOWS */

static void
trip_signal(int sig_num)
{
    unsigned char byte;
    int fd;
    Py_ssize_t rc;

    _Py_atomic_store_relaxed(&Handlers[sig_num].tripped, 1);

    /* Set is_tripped after setting .tripped, as it gets
       cleared in PyErr_CheckSignals() before .tripped. */
    _Py_atomic_store(&is_tripped, 1);

    /* Notify ceval.c */
    _PyEval_SignalReceived();

    /* And then write to the wakeup fd *after* setting all the globals and
       doing the _PyEval_SignalReceived. We used to write to the wakeup fd
       and then set the flag, but this allowed the following sequence of events
       (especially on windows, where trip_signal may run in a new thread):

       - main thread blocks on select([wakeup_fd], ...)
       - signal arrives
       - trip_signal writes to the wakeup fd
       - the main thread wakes up
       - the main thread checks the signal flags, sees that they're unset
       - the main thread empties the wakeup fd
       - the main thread goes back to sleep
       - trip_signal sets the flags to request the Python-level signal handler
         be run
       - the main thread doesn't notice, because it's asleep

       See bpo-30038 for more details.
    */

#ifdef MS_WINDOWS
    fd = Py_SAFE_DOWNCAST(wakeup.fd, SOCKET_T, int);
#else
    fd = wakeup_fd;
#endif

    if (fd != INVALID_FD) {
        byte = (unsigned char)sig_num;
#ifdef MS_WINDOWS
        if (wakeup.use_send) {
            do {
                rc = send(fd, &byte, 1, 0);
            } while (rc < 0 && errno == EINTR);

            /* we only have a storage for one error in the wakeup structure */
            if (rc < 0 && !wakeup.send_err_set) {
                wakeup.send_err_set = 1;
                wakeup.send_errno = errno;
                wakeup.send_win_error = GetLastError();
                /* Py_AddPendingCall() isn't signal-safe, but we
                   still use it for this exceptional case. */
                Py_AddPendingCall(report_wakeup_send_error, NULL);
            }
        }
        else
#endif
        {
            byte = (unsigned char)sig_num;

            /* _Py_write_noraise() retries write() if write() is interrupted by
               a signal (fails with EINTR). */
            rc = _Py_write_noraise(fd, &byte, 1);

            if (rc < 0) {
                /* Py_AddPendingCall() isn't signal-safe, but we
                   still use it for this exceptional case. */
                Py_AddPendingCall(report_wakeup_write_error,
                                  (void *)(intptr_t)errno);
            }
        }
    }
}

static void
signal_handler(int sig_num)
{
    int save_errno = errno;

#ifdef WITH_THREAD
    /* See NOTES section above */
    if (getpid() == main_pid)
#endif
    {
        trip_signal(sig_num);
    }

#ifndef HAVE_SIGACTION
#ifdef SIGCHLD
    /* To avoid infinite recursion, this signal remains
       reset until explicit re-instated.
       Don't clear the 'func' field as it is our pointer
       to the Python handler... */
    if (sig_num != SIGCHLD)
#endif
    /* If the handler was not set up with sigaction, reinstall it.  See
     * Python/pylifecycle.c for the implementation of PyOS_setsig which
     * makes this true.  See also issue8354. */
    PyOS_setsig(sig_num, signal_handler);
#endif

    /* Issue #10311: asynchronously executing signal handlers should not
       mutate errno under the feet of unsuspecting C code. */
    errno = save_errno;

#ifdef MS_WINDOWS
    if (sig_num == SIGINT)
        SetEvent(sigint_event);
#endif
}


#ifdef HAVE_ALARM

/*[clinic input]
signal.alarm -> long

    seconds: int
    /

Arrange for SIGALRM to arrive after the given number of seconds.
[clinic start generated code]*/

static long
signal_alarm_impl(PyObject *module, int seconds)
/*[clinic end generated code: output=144232290814c298 input=0d5e97e0e6f39e86]*/
{
    /* alarm() returns the number of seconds remaining */
    return (long)alarm(seconds);
}

#endif

#ifdef HAVE_PAUSE

/*[clinic input]
signal.pause

Wait until a signal arrives.
[clinic start generated code]*/

static PyObject *
signal_pause_impl(PyObject *module)
/*[clinic end generated code: output=391656788b3c3929 input=f03de0f875752062]*/
{
    Py_BEGIN_ALLOW_THREADS
    (void)pause();
    Py_END_ALLOW_THREADS
    /* make sure that any exceptions that got raised are propagated
     * back into Python
     */
    if (PyErr_CheckSignals())
        return NULL;

    Py_RETURN_NONE;
}

#endif


/*[clinic input]
signal.signal

    signalnum: int
    handler:   object
    /

Set the action for the given signal.

The action can be SIG_DFL, SIG_IGN, or a callable Python object.
The previous action is returned.  See getsignal() for possible return values.

*** IMPORTANT NOTICE ***
A signal handler function is called with two arguments:
the first is the signal number, the second is the interrupted stack frame.
[clinic start generated code]*/

static PyObject *
signal_signal_impl(PyObject *module, int signalnum, PyObject *handler)
/*[clinic end generated code: output=b44cfda43780f3a1 input=deee84af5fa0432c]*/
{
    PyObject *old_handler;
    void (*func)(int);
#ifdef MS_WINDOWS
    /* Validate that signalnum is one of the allowable signals */
    switch (signalnum) {
        case SIGABRT: break;
#ifdef SIGBREAK
        /* Issue #10003: SIGBREAK is not documented as permitted, but works
           and corresponds to CTRL_BREAK_EVENT. */
        case SIGBREAK: break;
#endif
        case SIGFPE: break;
        case SIGILL: break;
        case SIGINT: break;
        case SIGSEGV: break;
        case SIGTERM: break;
        default:
            PyErr_SetString(PyExc_ValueError, "invalid signal value");
            return NULL;
    }
#endif
#ifdef WITH_THREAD
    if (PyThread_get_thread_ident() != main_thread) {
        PyErr_SetString(PyExc_ValueError,
                        "signal only works in main thread");
        return NULL;
    }
#endif
    if (signalnum < 1 || signalnum >= Py_NSIG) {
        PyErr_SetString(PyExc_ValueError,
                        "signal number out of range");
        return NULL;
    }
    if (handler == IgnoreHandler)
        func = SIG_IGN;
    else if (handler == DefaultHandler)
        func = SIG_DFL;
    else if (!PyCallable_Check(handler)) {
        PyErr_SetString(PyExc_TypeError,
"signal handler must be signal.SIG_IGN, signal.SIG_DFL, or a callable object");
                return NULL;
    }
    else
        func = signal_handler;
    /* Check for pending signals before changing signal handler */
    if (PyErr_CheckSignals()) {
        return NULL;
    }
    if (PyOS_setsig(signalnum, func) == SIG_ERR) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    old_handler = Handlers[signalnum].func;
    Py_INCREF(handler);
    Handlers[signalnum].func = handler;
    if (old_handler != NULL)
        return old_handler;
    else
        Py_RETURN_NONE;
}


/*[clinic input]
signal.getsignal

    signalnum: int
    /

Return the current action for the given signal.

The return value can be:
  SIG_IGN -- if the signal is being ignored
  SIG_DFL -- if the default action for the signal is in effect
  None    -- if an unknown handler is in effect
  anything else -- the callable Python object used as a handler
[clinic start generated code]*/

static PyObject *
signal_getsignal_impl(PyObject *module, int signalnum)
/*[clinic end generated code: output=35b3e0e796fd555e input=ac23a00f19dfa509]*/
{
    PyObject *old_handler;
    if (signalnum < 1 || signalnum >= Py_NSIG) {
        PyErr_SetString(PyExc_ValueError,
                        "signal number out of range");
        return NULL;
    }
    old_handler = Handlers[signalnum].func;
    if (old_handler != NULL) {
        Py_INCREF(old_handler);
        return old_handler;
    }
    else {
        Py_RETURN_NONE;
    }
}

#ifdef HAVE_SIGINTERRUPT

/*[clinic input]
signal.siginterrupt

    signalnum: int
    flag:      int
    /

Change system call restart behaviour.

If flag is False, system calls will be restarted when interrupted by
signal sig, else system calls will be interrupted.
[clinic start generated code]*/

static PyObject *
signal_siginterrupt_impl(PyObject *module, int signalnum, int flag)
/*[clinic end generated code: output=063816243d85dd19 input=4160acacca3e2099]*/
{
    if (signalnum < 1 || signalnum >= Py_NSIG) {
        PyErr_SetString(PyExc_ValueError,
                        "signal number out of range");
        return NULL;
    }
    if (siginterrupt(signalnum, flag)<0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    Py_RETURN_NONE;
}

#endif


static PyObject*
signal_set_wakeup_fd(PyObject *self, PyObject *args)
{
    struct _Py_stat_struct status;
#ifdef MS_WINDOWS
    PyObject *fdobj;
    SOCKET_T sockfd, old_sockfd;
    int res;
    int res_size = sizeof res;
    PyObject *mod;
    int is_socket;

    if (!PyArg_ParseTuple(args, "O:set_wakeup_fd", &fdobj))
        return NULL;

    sockfd = PyLong_AsSocket_t(fdobj);
    if (sockfd == (SOCKET_T)(-1) && PyErr_Occurred())
        return NULL;
#else
    int fd, old_fd;

    if (!PyArg_ParseTuple(args, "i:set_wakeup_fd", &fd))
        return NULL;
#endif

#ifdef WITH_THREAD
    if (PyThread_get_thread_ident() != main_thread) {
        PyErr_SetString(PyExc_ValueError,
                        "set_wakeup_fd only works in main thread");
        return NULL;
    }
#endif

#ifdef MS_WINDOWS
    is_socket = 0;
    if (sockfd != INVALID_FD) {
        /* Import the _socket module to call WSAStartup() */
        mod = PyImport_ImportModuleNoBlock("_socket");
        if (mod == NULL)
            return NULL;
        Py_DECREF(mod);

        /* test the socket */
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
                       (char *)&res, &res_size) != 0) {
            int fd, err;

            err = WSAGetLastError();
            if (err != WSAENOTSOCK) {
                PyErr_SetExcFromWindowsErr(PyExc_OSError, err);
                return NULL;
            }

            fd = (int)sockfd;
            if ((SOCKET_T)fd != sockfd) {
                PyErr_SetString(PyExc_ValueError, "invalid fd");
                return NULL;
            }

            if (_Py_fstat(fd, &status) != 0)
                return NULL;

            /* on Windows, a file cannot be set to non-blocking mode */
        }
        else {
            is_socket = 1;

            /* Windows does not provide a function to test if a socket
               is in non-blocking mode */
        }
    }

    old_sockfd = wakeup.fd;
    wakeup.fd = sockfd;
    wakeup.use_send = is_socket;

    if (old_sockfd != INVALID_FD)
        return PyLong_FromSocket_t(old_sockfd);
    else
        return PyLong_FromLong(-1);
#else
    if (fd != -1) {
        int blocking;

        if (_Py_fstat(fd, &status) != 0)
            return NULL;

        blocking = _Py_get_blocking(fd);
        if (blocking < 0)
            return NULL;
        if (blocking) {
            PyErr_Format(PyExc_ValueError,
                         "the fd %i must be in non-blocking mode",
                         fd);
            return NULL;
        }
    }

    old_fd = wakeup_fd;
    wakeup_fd = fd;

    return PyLong_FromLong(old_fd);
#endif
}

PyDoc_STRVAR(set_wakeup_fd_doc,
"set_wakeup_fd(fd) -> fd\n\
\n\
Sets the fd to be written to (with the signal number) when a signal\n\
comes in.  A library can use this to wakeup select or poll.\n\
The previous fd or -1 is returned.\n\
\n\
The fd must be non-blocking.");

/* C API for the same, without all the error checking */
int
PySignal_SetWakeupFd(int fd)
{
    int old_fd;
    if (fd < 0)
        fd = -1;

#ifdef MS_WINDOWS
    old_fd = Py_SAFE_DOWNCAST(wakeup.fd, SOCKET_T, int);
    wakeup.fd = fd;
#else
    old_fd = wakeup_fd;
    wakeup_fd = fd;
#endif
    return old_fd;
}


#ifdef HAVE_SETITIMER

/*[clinic input]
signal.setitimer

    which:    int
    seconds:  double
    interval: double = 0.0
    /

Sets given itimer (one of ITIMER_REAL, ITIMER_VIRTUAL or ITIMER_PROF).

The timer will fire after value seconds and after that every interval seconds.
The itimer can be cleared by setting seconds to zero.

Returns old values as a tuple: (delay, interval).
[clinic start generated code]*/

static PyObject *
signal_setitimer_impl(PyObject *module, int which, double seconds,
                      double interval)
/*[clinic end generated code: output=6f51da0fe0787f2c input=0d27d417cfcbd51a]*/
{
    struct itimerval new, old;

    timeval_from_double(seconds, &new.it_value);
    timeval_from_double(interval, &new.it_interval);
    /* Let OS check "which" value */
    if (setitimer(which, &new, &old) != 0) {
        PyErr_SetFromErrno(ItimerError);
        return NULL;
    }

    return itimer_retval(&old);
}

#endif


#ifdef HAVE_GETITIMER

/*[clinic input]
signal.getitimer

    which:    int
    /

Returns current value of given itimer.
[clinic start generated code]*/

static PyObject *
signal_getitimer_impl(PyObject *module, int which)
/*[clinic end generated code: output=9e053175d517db40 input=f7d21d38f3490627]*/
{
    struct itimerval old;

    if (getitimer(which, &old) != 0) {
        PyErr_SetFromErrno(ItimerError);
        return NULL;
    }

    return itimer_retval(&old);
}

#endif

#if defined(PYPTHREAD_SIGMASK) || defined(HAVE_SIGWAIT) || \
        defined(HAVE_SIGWAITINFO) || defined(HAVE_SIGTIMEDWAIT)
/* Convert an iterable to a sigset.
   Return 0 on success, return -1 and raise an exception on error. */

static int
iterable_to_sigset(PyObject *iterable, sigset_t *mask)
{
    int result = -1;
    PyObject *iterator, *item;
    long signum;

    sigemptyset(mask);

    iterator = PyObject_GetIter(iterable);
    if (iterator == NULL)
        goto error;

    while (1)
    {
        item = PyIter_Next(iterator);
        if (item == NULL) {
            if (PyErr_Occurred())
                goto error;
            else
                break;
        }

        signum = PyLong_AsLong(item);
        Py_DECREF(item);
        if (signum == -1 && PyErr_Occurred())
            goto error;
        if (0 < signum && signum < Py_NSIG) {
            /* bpo-33329: ignore sigaddset() return value as it can fail
             * for some reserved signals, but we want the `range(1, NSIG)`
             * idiom to allow selecting all valid signals.
             */
            (void) sigaddset(mask, (int)signum);
        }
        else {
            PyErr_Format(PyExc_ValueError,
                         "signal number %ld out of range", signum);
            goto error;
        }
    }
    result = 0;

error:
    Py_XDECREF(iterator);
    return result;
}
#endif

#if defined(PYPTHREAD_SIGMASK) || defined(HAVE_SIGPENDING)
static PyObject*
sigset_to_set(sigset_t mask)
{
    PyObject *signum, *result;
    int sig;

    result = PySet_New(0);
    if (result == NULL)
        return NULL;

    for (sig = 1; sig < Py_NSIG; sig++) {
        if (sigismember(&mask, sig) != 1)
            continue;

        /* Handle the case where it is a member by adding the signal to
           the result list.  Ignore the other cases because they mean the
           signal isn't a member of the mask or the signal was invalid,
           and an invalid signal must have been our fault in constructing
           the loop boundaries. */
        signum = PyLong_FromLong(sig);
        if (signum == NULL) {
            Py_DECREF(result);
            return NULL;
        }
        if (PySet_Add(result, signum) == -1) {
            Py_DECREF(signum);
            Py_DECREF(result);
            return NULL;
        }
        Py_DECREF(signum);
    }
    return result;
}
#endif

#ifdef PYPTHREAD_SIGMASK

/*[clinic input]
signal.pthread_sigmask

    how:  int
    mask: object
    /

Fetch and/or change the signal mask of the calling thread.
[clinic start generated code]*/

static PyObject *
signal_pthread_sigmask_impl(PyObject *module, int how, PyObject *mask)
/*[clinic end generated code: output=ff640fe092bc9181 input=f3b7d7a61b7b8283]*/
{
    sigset_t newmask, previous;
    int err;

    if (iterable_to_sigset(mask, &newmask))
        return NULL;

    err = pthread_sigmask(how, &newmask, &previous);
    if (err != 0) {
        errno = err;
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    /* if signals was unblocked, signal handlers have been called */
    if (PyErr_CheckSignals())
        return NULL;

    return sigset_to_set(previous);
}

#endif   /* #ifdef PYPTHREAD_SIGMASK */


#ifdef HAVE_SIGPENDING

/*[clinic input]
signal.sigpending

Examine pending signals.

Returns a set of signal numbers that are pending for delivery to
the calling thread.
[clinic start generated code]*/

static PyObject *
signal_sigpending_impl(PyObject *module)
/*[clinic end generated code: output=53375ffe89325022 input=e0036c016f874e29]*/
{
    int err;
    sigset_t mask;
    err = sigpending(&mask);
    if (err)
        return PyErr_SetFromErrno(PyExc_OSError);
    return sigset_to_set(mask);
}

#endif   /* #ifdef HAVE_SIGPENDING */


#ifdef HAVE_SIGWAIT

/*[clinic input]
signal.sigwait

    sigset: object
    /

Wait for a signal.

Suspend execution of the calling thread until the delivery of one of the
signals specified in the signal set sigset.  The function accepts the signal
and returns the signal number.
[clinic start generated code]*/

static PyObject *
signal_sigwait(PyObject *module, PyObject *sigset)
/*[clinic end generated code: output=557173647424f6e4 input=11af2d82d83c2e94]*/
{
    sigset_t set;
    int err, signum;

    if (iterable_to_sigset(sigset, &set))
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    err = sigwait(&set, &signum);
    Py_END_ALLOW_THREADS
    if (err) {
        errno = err;
        return PyErr_SetFromErrno(PyExc_OSError);
    }

    return PyLong_FromLong(signum);
}

#endif   /* #ifdef HAVE_SIGWAIT */


#if defined(HAVE_SIGWAITINFO) || defined(HAVE_SIGTIMEDWAIT)
static int initialized;
static PyStructSequence_Field struct_siginfo_fields[] = {
    {"si_signo",        PyDoc_STR("signal number")},
    {"si_code",         PyDoc_STR("signal code")},
    {"si_errno",        PyDoc_STR("errno associated with this signal")},
    {"si_pid",          PyDoc_STR("sending process ID")},
    {"si_uid",          PyDoc_STR("real user ID of sending process")},
    {"si_status",       PyDoc_STR("exit value or signal")},
    {"si_band",         PyDoc_STR("band event for SIGPOLL")},
    {0}
};

PyDoc_STRVAR(struct_siginfo__doc__,
"struct_siginfo: Result from sigwaitinfo or sigtimedwait.\n\n\
This object may be accessed either as a tuple of\n\
(si_signo, si_code, si_errno, si_pid, si_uid, si_status, si_band),\n\
or via the attributes si_signo, si_code, and so on.");

static PyStructSequence_Desc struct_siginfo_desc = {
    "signal.struct_siginfo",           /* name */
    struct_siginfo__doc__,       /* doc */
    struct_siginfo_fields,       /* fields */
    7          /* n_in_sequence */
};

static PyTypeObject SiginfoType;

static PyObject *
fill_siginfo(siginfo_t *si)
{
    PyObject *result = PyStructSequence_New(&SiginfoType);
    if (!result)
        return NULL;

    PyStructSequence_SET_ITEM(result, 0, PyLong_FromLong((long)(si->si_signo)));
    PyStructSequence_SET_ITEM(result, 1, PyLong_FromLong((long)(si->si_code)));
    PyStructSequence_SET_ITEM(result, 2, PyLong_FromLong((long)(si->si_errno)));
    PyStructSequence_SET_ITEM(result, 3, PyLong_FromPid(si->si_pid));
    PyStructSequence_SET_ITEM(result, 4, _PyLong_FromUid(si->si_uid));
    PyStructSequence_SET_ITEM(result, 5,
                                PyLong_FromLong((long)(si->si_status)));
    PyStructSequence_SET_ITEM(result, 6, PyLong_FromLong(si->si_band));
    if (PyErr_Occurred()) {
        Py_DECREF(result);
        return NULL;
    }

    return result;
}
#endif

#ifdef HAVE_SIGWAITINFO

/*[clinic input]
signal.sigwaitinfo

    sigset: object
    /

Wait synchronously until one of the signals in *sigset* is delivered.

Returns a struct_siginfo containing information about the signal.
[clinic start generated code]*/

static PyObject *
signal_sigwaitinfo(PyObject *module, PyObject *sigset)
/*[clinic end generated code: output=c40f27b269cd2309 input=f3779a74a991e171]*/
{
    sigset_t set;
    siginfo_t si;
    int err;
    int async_err = 0;

    if (iterable_to_sigset(sigset, &set))
        return NULL;

    do {
        Py_BEGIN_ALLOW_THREADS
        err = sigwaitinfo(&set, &si);
        Py_END_ALLOW_THREADS
    } while (err == -1
             && errno == EINTR && !(async_err = PyErr_CheckSignals()));
    if (err == -1)
        return (!async_err) ? PyErr_SetFromErrno(PyExc_OSError) : NULL;

    return fill_siginfo(&si);
}

#endif   /* #ifdef HAVE_SIGWAITINFO */

#ifdef HAVE_SIGTIMEDWAIT

/*[clinic input]
signal.sigtimedwait

    sigset:  object
    timeout as timeout_obj: object
    /

Like sigwaitinfo(), but with a timeout.

The timeout is specified in seconds, with floating point numbers allowed.
[clinic start generated code]*/

static PyObject *
signal_sigtimedwait_impl(PyObject *module, PyObject *sigset,
                         PyObject *timeout_obj)
/*[clinic end generated code: output=f7eff31e679f4312 input=53fd4ea3e3724eb8]*/
{
    struct timespec ts;
    sigset_t set;
    siginfo_t si;
    int res;
    _PyTime_t timeout, deadline, monotonic;

    if (_PyTime_FromSecondsObject(&timeout,
                                  timeout_obj, _PyTime_ROUND_CEILING) < 0)
        return NULL;

    if (timeout < 0) {
        PyErr_SetString(PyExc_ValueError, "timeout must be non-negative");
        return NULL;
    }

    if (iterable_to_sigset(sigset, &set))
        return NULL;

    deadline = _PyTime_GetMonotonicClock() + timeout;

    do {
        if (_PyTime_AsTimespec(timeout, &ts) < 0)
            return NULL;

        Py_BEGIN_ALLOW_THREADS
        res = sigtimedwait(&set, &si, &ts);
        Py_END_ALLOW_THREADS

        if (res != -1)
            break;

        if (errno != EINTR) {
            if (errno == EAGAIN)
                Py_RETURN_NONE;
            else
                return PyErr_SetFromErrno(PyExc_OSError);
        }

        /* sigtimedwait() was interrupted by a signal (EINTR) */
        if (PyErr_CheckSignals())
            return NULL;

        monotonic = _PyTime_GetMonotonicClock();
        timeout = deadline - monotonic;
        if (timeout < 0)
            break;
    } while (1);

    return fill_siginfo(&si);
}

#endif   /* #ifdef HAVE_SIGTIMEDWAIT */


#if defined(HAVE_PTHREAD_KILL) && defined(WITH_THREAD)

/*[clinic input]
signal.pthread_kill

    thread_id:  long
    signalnum:  int
    /

Send a signal to a thread.
[clinic start generated code]*/

static PyObject *
signal_pthread_kill_impl(PyObject *module, long thread_id, int signalnum)
/*[clinic end generated code: output=2a09ce41f1c4228a input=77ed6a3b6f2a8122]*/
{
    int err;

    err = pthread_kill((pthread_t)thread_id, signalnum);
    if (err != 0) {
        errno = err;
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    /* the signal may have been send to the current thread */
    if (PyErr_CheckSignals())
        return NULL;

    Py_RETURN_NONE;
}

#endif   /* #if defined(HAVE_PTHREAD_KILL) && defined(WITH_THREAD) */



/* List of functions defined in the module -- some of the methoddefs are
   defined to nothing if the corresponding C function is not available. */
static PyMethodDef signal_methods[] = {
    {"default_int_handler", signal_default_int_handler, METH_VARARGS, default_int_handler_doc},
    SIGNAL_ALARM_METHODDEF
    SIGNAL_SETITIMER_METHODDEF
    SIGNAL_GETITIMER_METHODDEF
    SIGNAL_SIGNAL_METHODDEF
    SIGNAL_GETSIGNAL_METHODDEF
    {"set_wakeup_fd",           signal_set_wakeup_fd, METH_VARARGS, set_wakeup_fd_doc},
    SIGNAL_SIGINTERRUPT_METHODDEF
    SIGNAL_PAUSE_METHODDEF
    SIGNAL_PTHREAD_KILL_METHODDEF
    SIGNAL_PTHREAD_SIGMASK_METHODDEF
    SIGNAL_SIGPENDING_METHODDEF
    SIGNAL_SIGWAIT_METHODDEF
    SIGNAL_SIGWAITINFO_METHODDEF
    SIGNAL_SIGTIMEDWAIT_METHODDEF
    {NULL, NULL}           /* sentinel */
};


PyDoc_STRVAR(module_doc,
"This module provides mechanisms to use signal handlers in Python.\n\
\n\
Functions:\n\
\n\
alarm() -- cause SIGALRM after a specified time [Unix only]\n\
setitimer() -- cause a signal (described below) after a specified\n\
               float time and the timer may restart then [Unix only]\n\
getitimer() -- get current value of timer [Unix only]\n\
signal() -- set the action for a given signal\n\
getsignal() -- get the signal action for a given signal\n\
pause() -- wait until a signal arrives [Unix only]\n\
default_int_handler() -- default SIGINT handler\n\
\n\
signal constants:\n\
SIG_DFL -- used to refer to the system default handler\n\
SIG_IGN -- used to ignore the signal\n\
Py_NSIG -- number of defined signals\n\
SIGINT, SIGTERM, etc. -- signal numbers\n\
\n\
itimer constants:\n\
ITIMER_REAL -- decrements in real time, and delivers SIGALRM upon\n\
               expiration\n\
ITIMER_VIRTUAL -- decrements only when the process is executing,\n\
               and delivers SIGVTALRM upon expiration\n\
ITIMER_PROF -- decrements both when the process is executing and\n\
               when the system is executing on behalf of the process.\n\
               Coupled with ITIMER_VIRTUAL, this timer is usually\n\
               used to profile the time spent by the application\n\
               in user and kernel space. SIGPROF is delivered upon\n\
               expiration.\n\
\n\n\
*** IMPORTANT NOTICE ***\n\
A signal handler function is called with two arguments:\n\
the first is the signal number, the second is the interrupted stack frame.");

static struct PyModuleDef signalmodule = {
    PyModuleDef_HEAD_INIT,
    "_signal",
    module_doc,
    -1,
    signal_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__signal(void)
{
    PyObject *m, *d, *x;
    int i;

#ifdef WITH_THREAD
    main_thread = PyThread_get_thread_ident();
    main_pid = getpid();
#endif

    /* Create the module and add the functions */
    m = PyModule_Create(&signalmodule);
    if (m == NULL)
        return NULL;

#if defined(HAVE_SIGWAITINFO) || defined(HAVE_SIGTIMEDWAIT)
    if (!initialized) {
        if (PyStructSequence_InitType2(&SiginfoType, &struct_siginfo_desc) < 0)
            return NULL;
    }
    Py_INCREF((PyObject*) &SiginfoType);
    PyModule_AddObject(m, "struct_siginfo", (PyObject*) &SiginfoType);
    initialized = 1;
#endif

    /* Add some symbolic constants to the module */
    d = PyModule_GetDict(m);

    x = DefaultHandler = PyLong_FromVoidPtr((void *)SIG_DFL);
    if (!x || PyDict_SetItemString(d, "SIG_DFL", x) < 0)
        goto finally;

    x = IgnoreHandler = PyLong_FromVoidPtr((void *)SIG_IGN);
    if (!x || PyDict_SetItemString(d, "SIG_IGN", x) < 0)
        goto finally;

    x = PyLong_FromLong((long)Py_NSIG);
    if (!x || PyDict_SetItemString(d, "NSIG", x) < 0)
        goto finally;
    Py_DECREF(x);

    if (PyModule_AddIntMacro(m, SIG_BLOCK)) goto finally;
    if (PyModule_AddIntMacro(m, SIG_UNBLOCK)) goto finally;
    if (PyModule_AddIntMacro(m, SIG_SETMASK)) goto finally;

    x = IntHandler = PyDict_GetItemString(d, "default_int_handler");
    if (!x)
        goto finally;
    Py_INCREF(IntHandler);

    _Py_atomic_store_relaxed(&Handlers[0].tripped, 0);
    for (i = 1; i < Py_NSIG; i++) {
        void (*t)(int);
        t = PyOS_getsig(i);
        _Py_atomic_store_relaxed(&Handlers[i].tripped, 0);
        if (t == SIG_DFL)
            Handlers[i].func = DefaultHandler;
        else if (t == SIG_IGN)
            Handlers[i].func = IgnoreHandler;
        else
            Handlers[i].func = Py_None; /* None of our business */
        Py_INCREF(Handlers[i].func);
    }
    if (Handlers[SIGINT].func == DefaultHandler) {
        /* Install default int handler */
        Py_INCREF(IntHandler);
        Py_SETREF(Handlers[SIGINT].func, IntHandler);
        PyOS_setsig(SIGINT, signal_handler);
    }

    if (PyModule_AddIntMacro(m, SIGHUP)) goto finally;
    if (PyModule_AddIntMacro(m, SIGINT)) goto finally;
    if (PyModule_AddIntMacro(m, SIGQUIT)) goto finally;
    if (PyModule_AddIntMacro(m, SIGILL)) goto finally;
    if (PyModule_AddIntMacro(m, SIGTRAP)) goto finally;
    if (PyModule_AddIntMacro(m, SIGIOT)) goto finally;
    if (PyModule_AddIntMacro(m, SIGABRT)) goto finally;
    if (PyModule_AddIntMacro(m, SIGFPE)) goto finally;
    if (PyModule_AddIntMacro(m, SIGKILL)) goto finally;
    if (PyModule_AddIntMacro(m, SIGBUS)) goto finally;
    if (PyModule_AddIntMacro(m, SIGSEGV)) goto finally;
    if (PyModule_AddIntMacro(m, SIGSYS)) goto finally;
    if (PyModule_AddIntMacro(m, SIGPIPE)) goto finally;
    if (PyModule_AddIntMacro(m, SIGALRM)) goto finally;
    if (PyModule_AddIntMacro(m, SIGTERM)) goto finally;
    if (PyModule_AddIntMacro(m, SIGUSR1)) goto finally;
    if (PyModule_AddIntMacro(m, SIGUSR2)) goto finally;
    if (PyModule_AddIntMacro(m, SIGCHLD)) goto finally;
    if (PyModule_AddIntMacro(m, SIGPWR)) goto finally;
    if (PyModule_AddIntMacro(m, SIGIO)) goto finally;
    if (PyModule_AddIntMacro(m, SIGURG)) goto finally;
    if (PyModule_AddIntMacro(m, SIGWINCH)) goto finally;
    if (PyModule_AddIntMacro(m, SIGPOLL)) goto finally;
    if (PyModule_AddIntMacro(m, SIGSTOP)) goto finally;
    if (PyModule_AddIntMacro(m, SIGTSTP)) goto finally;
    if (PyModule_AddIntMacro(m, SIGCONT)) goto finally;
    if (PyModule_AddIntMacro(m, SIGTTIN)) goto finally;
    if (PyModule_AddIntMacro(m, SIGTTOU)) goto finally;
    if (PyModule_AddIntMacro(m, SIGVTALRM)) goto finally;
    if (PyModule_AddIntMacro(m, SIGPROF)) goto finally;
    if (PyModule_AddIntMacro(m, SIGXCPU)) goto finally;
    if (PyModule_AddIntMacro(m, SIGXFSZ)) goto finally;
    if (SIGEMT && PyModule_AddIntMacro(m, SIGEMT)) goto finally;
    if (SIGINFO && PyModule_AddIntMacro(m, SIGINFO)) goto finally;
    if (SIGRTMIN && PyModule_AddIntMacro(m, SIGRTMIN)) goto finally;
    if (SIGRTMAX && PyModule_AddIntMacro(m, SIGRTMAX)) goto finally;

#if defined (HAVE_SETITIMER) || defined (HAVE_GETITIMER)
    if (PyModule_AddIntMacro(m, ITIMER_REAL)) goto finally;
    if (PyModule_AddIntMacro(m, ITIMER_VIRTUAL)) goto finally;
    if (PyModule_AddIntMacro(m, ITIMER_PROF)) goto finally;
    ItimerError = PyErr_NewException("signal.ItimerError",
            PyExc_IOError, NULL);
    if (ItimerError != NULL)
        PyDict_SetItemString(d, "ItimerError", ItimerError);
#endif

#ifdef CTRL_C_EVENT
    if (PyModule_AddIntMacro(m, CTRL_C_EVENT))
         goto finally;
#endif

#ifdef CTRL_BREAK_EVENT
    if (PyModule_AddIntMacro(m, CTRL_BREAK_EVENT))
         goto finally;
#endif

#ifdef MS_WINDOWS
    /* Create manual-reset event, initially unset */
    sigint_event = CreateEvent(NULL, TRUE, FALSE, FALSE);
#endif

    if (PyErr_Occurred()) {
        Py_DECREF(m);
        m = NULL;
    }

  finally:
    return m;
}

static void
finisignal(void)
{
    int i;
    PyObject *func;

    for (i = 1; i < Py_NSIG; i++) {
        func = Handlers[i].func;
        _Py_atomic_store_relaxed(&Handlers[i].tripped, 0);
        Handlers[i].func = NULL;
        if (func != NULL && func != Py_None &&
            func != DefaultHandler && func != IgnoreHandler)
            PyOS_setsig(i, SIG_DFL);
        Py_XDECREF(func);
    }

    Py_CLEAR(IntHandler);
    Py_CLEAR(DefaultHandler);
    Py_CLEAR(IgnoreHandler);
}


/* Declared in pyerrors.h */
int
PyErr_CheckSignals(void)
{
    int i;
    PyObject *f;

    if (!_Py_atomic_load(&is_tripped))
        return 0;

#ifdef WITH_THREAD
    if (PyThread_get_thread_ident() != main_thread)
        return 0;
#endif

    /*
     * The is_tripped variable is meant to speed up the calls to
     * PyErr_CheckSignals (both directly or via pending calls) when no
     * signal has arrived. This variable is set to 1 when a signal arrives
     * and it is set to 0 here, when we know some signals arrived. This way
     * we can run the registered handlers with no signals blocked.
     *
     * NOTE: with this approach we can have a situation where is_tripped is
     *       1 but we have no more signals to handle (Handlers[i].tripped
     *       is 0 for every signal i). This won't do us any harm (except
     *       we're gonna spent some cycles for nothing). This happens when
     *       we receive a signal i after we zero is_tripped and before we
     *       check Handlers[i].tripped.
     */
    _Py_atomic_store(&is_tripped, 0);

    if (!(f = (PyObject *)PyEval_GetFrame()))
        f = Py_None;

    for (i = 1; i < Py_NSIG; i++) {
        if (_Py_atomic_load_relaxed(&Handlers[i].tripped)) {
            PyObject *result = NULL;
            PyObject *arglist = Py_BuildValue("(iO)", i, f);
            _Py_atomic_store_relaxed(&Handlers[i].tripped, 0);

            if (arglist) {
                result = PyEval_CallObject(Handlers[i].func,
                                           arglist);
                Py_DECREF(arglist);
            }
            if (!result) {
                _Py_atomic_store(&is_tripped, 1);
                return -1;
            }

            Py_DECREF(result);
        }
    }

    return 0;
}


/* Replacements for intrcheck.c functionality
 * Declared in pyerrors.h
 */
void
PyErr_SetInterrupt(void)
{
    trip_signal(SIGINT);
}

void
PyOS_InitInterrupts(void)
{
    PyObject *m = PyImport_ImportModule("_signal");
    if (m) {
        Py_DECREF(m);
    }
}

void
PyOS_FiniInterrupts(void)
{
    finisignal();
}

int
PyOS_InterruptOccurred(void)
{
    if (_Py_atomic_load_relaxed(&Handlers[SIGINT].tripped)) {
#ifdef WITH_THREAD
        if (PyThread_get_thread_ident() != main_thread)
            return 0;
#endif
        _Py_atomic_store_relaxed(&Handlers[SIGINT].tripped, 0);
        return 1;
    }
    return 0;
}

static void
_clear_pending_signals(void)
{
    int i;
    if (!_Py_atomic_load(&is_tripped))
        return;
    _Py_atomic_store(&is_tripped, 0);
    for (i = 1; i < Py_NSIG; ++i) {
        _Py_atomic_store_relaxed(&Handlers[i].tripped, 0);
    }
}

void
PyOS_AfterFork(void)
{
    /* Clear the signal flags after forking so that they aren't handled
     * in both processes if they came in just before the fork() but before
     * the interpreter had an opportunity to call the handlers.  issue9535. */
    _clear_pending_signals();
#ifdef WITH_THREAD
    /* PyThread_ReInitTLS() must be called early, to make sure that the TLS API
     * can be called safely. */
    PyThread_ReInitTLS();
    _PyGILState_Reinit();
    PyEval_ReInitThreads();
    main_thread = PyThread_get_thread_ident();
    main_pid = getpid();
    _PyImport_ReInitLock();
#endif
}

int
_PyOS_IsMainThread(void)
{
#ifdef WITH_THREAD
    return PyThread_get_thread_ident() == main_thread;
#else
    return 1;
#endif
}

#ifdef MS_WINDOWS
void *_PyOS_SigintEvent(void)
{
    /* Returns a manual-reset event which gets tripped whenever
       SIGINT is received.

       Python.h does not include windows.h so we do cannot use HANDLE
       as the return type of this function.  We use void* instead. */
    return sigint_event;
}
#endif

#ifdef __aarch64__
_Section(".rodata.pytab.1 //")
#else
_Section(".rodata.pytab.1")
#endif
 const struct _inittab _PyImport_Inittab__signal = {
    "_signal",
    PyInit__signal,
};
