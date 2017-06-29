#ifndef UTILS_CALLBACK_H
#define UTILS_CALLBACK_H

#include <algorithm>

#ifdef CALLBACK_MULTITHREADED
#include <process.h>
#include "../../Utils/callbacks/Event.h"
#include "../../Utils/callbacks/Mutex.h"
#endif

#ifdef CALLBACK_MULTITHREADED
#ifndef UTILS_EVENT
#error "Cannot compile Callbacks (Callback.h) without signaling events (Event.h)"
#endif
#ifndef UTILS_MUTEX
#error "Cannot compile Callbacks (Callback.h) without mutual exclusions (Mutex.h)"
#endif
#endif

#include <stdint.h>

#ifndef _WIN32_
#include <time.h>
unsigned int GetTickCount(void)
{
  struct timespec now;

  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
  {
    return 0;
  }

  return (now.tv_sec * 1000) + (now.tv_nsec / 1000);
}
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif

namespace Utils
{
/**
 * constructs a string name for an object based on its memory location.
 * @param buf output buffer to write on
 * @param buflen size of the output buffer in bytes
 * @param container pointer to the object to generate a name from, only the memory location (i.e., pointer value) is used
 * @param param some sequence of characters to append to the memory location for description or discriminative reasons.
 */
static void handleName (char *buf, const unsigned int buflen, void *container, const char *param)
{
  char    prf[14];  ///< event names

#ifdef _WIN32_
  sprintf_s (prf, 14, "%08x\0", reinterpret_cast<unsigned int>(container));
#else
  snprintf (prf, 14, "%08x\0", reinterpret_cast<intptr_t>(container));
#endif
  //memset (buf, 0, buflen);
  std::fill (buf, buf+buflen, 0);
  //memcpy (buf, prf, 14);
  std::copy (prf, prf+14, buf);
#ifdef _WIN32_
  strcat_s (buf, buflen, param);
#else
  // FIXME: this is incorrect, should be buflen - strlen(buf)
  //strncat (buf, param, buflen);

  auto len = std::string (param).length ();
  std::copy (param, param+len, buf+14);
#endif
}
/**
 * converts an unsigned int storage type to a signed int numerical type.
 * @param an unsigned int containing a number which should be interpreted as signed.
 * @returns the signed type representing the value.
 */
#ifdef _WIN64
static __int64 wsgn (const unsigned __int64 i)
{
  return static_cast<int>((i&~0x7FFFFFFFFFFFFFFF)|(i&0x7FFFFFFFFFFFFFFF));
}
#else
static int wsgn (const unsigned int i)
{
  return static_cast<int>((i&~0x7FFFFFFF)|(i&0x7FFFFFFF));
}
#endif
/**
 * Function to obtain the current time for stamping.
 * Call this function so the time source may be changed consistently.
 * Currently uses GetTickCount ().
 * @returns the current time index for stamping (in milliseconds).
 */
static unsigned int timeStamp ()
{
  return GetTickCount ();
}
/**
 * @returns the time to be allowed for a thread to observe a signal (in milliseconds).
 */
static const unsigned int signalObservationTime ()
{
  return 1000;
}
/**
 * Workhorse callback object
 * FIXME: derive this from a MutableSet so we can store strings as well as numbers
 * NB: this is done as Callback_P because MutableSets use callbacks to observe change in values.
 * Although, it would be much nicer to have all callbacks accepting all types of parameters...
 */
class Callback : public IntegerSet
{
  friend class CallbackSet;
public:

#ifdef CALLBACK_MULTITHREADED
  typedef enum
  {
    READY      = 0,  ///< ready for action
    ALLOCATED  = 1,  ///< allocated for action
    EXECUTING  = 2,  ///< executing action
    INVALID    = 3   ///< not to be used
  } State;
#endif
protected:
#ifdef CALLBACK_MULTITHREADED
  Mutex      access;        ///< mutex guarding access to the internals
#endif

  /**
   * Utility function to set the return value of the callback
   */
  void setReturnValue (const IntegerSetType newReturnValue)
  {
#ifdef CALLBACK_MULTITHREADED
    if (access.wait () == true)
    {
#endif
      set ("returnValue\0", newReturnValue);
#ifdef CALLBACK_MULTITHREADED
      access.unlock ();
    }
#endif
  }
#ifdef CALLBACK_MULTITHREADED
  /**
   * Utility function to set the current state of the callback.
   * This uses the guard mutex to control access
   */
  void setState (const IntegerSetType newState)
  {
    if (access.wait () == true)
    {
      set ("state\0", newState);

      switch (newState)
      {
      case READY:
      case ALLOCATED:
        break;
      case EXECUTING:
        set ("lastRunTime\0", Utils::timeStamp ());
        break;
      case INVALID:
        set ("lastRunTime\0", 0);
        break;
      }
      access.unlock ();
    }
  }
#endif
public:
#ifdef CALLBACK_MULTITHREADED
// STRUCTORS
  Callback (const bool waitOn = true, HANDLE cancelSignal = NULL, HANDLE beepSignal = NULL, HANDLE completeSignal = NULL)
    : access ("TCBK\0", 100)
  {
    // CALLBACK PARAMS
    setReturnValue (0);
    setState (READY);

    if (access.wait () == false)
    {
      Note::notemsg ("MAJOR B0RK. Could not lock callback for signal creation\0");
      return;
    }

    if (cancelSignal == NULL)
    {
      set ("cancel\0", (IntegerSetType)Utils::Event::create (this, "cancel\0"));
      set ("delcancel\0", 1);
    }
    else
    {
      set ("cancel\0", (IntegerSetType)cancelSignal);
      set ("delcancel\0", 0);
    }

    if (completeSignal == NULL)
    {
      set ("complete\0", (IntegerSetType)Utils::Event::create (this, "complete\0"));
      set ("delcomplete\0", 1);
    }
    else
    {
      set ("complete\0", (IntegerSetType) completeSignal);
      set ("delcomplete\0", 0);
    }

    if (beepSignal == NULL)
    {
      set ("beep\0", (IntegerSetType)Utils::Event::create (this, "beep\0"));
      set ("delbeep\0", 1);
    }
    else
    {
      set ("beep\0", (IntegerSetType) beepSignal);
      set ("delbeep\0", 0);
    }

    set ("thread\0", NULL);
    set ("waitOn\0", (waitOn==true)?1:0);
    set ("lastRunTime\0", 0);

    access.unlock ();

    // cycle the events
    Utils::Event::signal (cancel ());
    Utils::Event::reset (cancel ());
    Utils::Event::signal (complete ());
    Utils::Event::reset (complete ());
    Utils::Event::signal (beep ());
    Utils::Event::reset (beep ());

    // reset the callback to its initial state
    reset ();
  }
  virtual ~Callback ()
  {
    setState (INVALID);

    if (access.wait () == true)
    {
      if (Utils::Event::signaled (complete ()) == false)
      {
        Utils::Event::signal (cancel ());
        Utils::Event::waitOn (complete (), Utils::signalObservationTime ());
      }

      if ((((*this) && "delcancel\0") == true) && ((*this)["delcancel\0"] == 1))
      {
        Utils::Event::close (cancel ());
      }

      if ((((*this) && "delcomplete\0") == true) && ((*this)["delcomplete\0"] == 1))
      {
        Utils::Event::close (complete ());
      }

      access.unlock ();
    }
    else
    {
      Note::notemsg ("FATAL: Could not obtain access to release events\0");
    }

    CloseHandle (thread ());
  }
#else
  Callback ()
  {
    // CALLBACK PARAMS
    setReturnValue (FALSE);
  }
#endif
// ACCESSORS
  /**
   * @returns the current return value of the callback
   */
  const IntegerSetType returnValue () const
  {
    IntegerSetType  s = FALSE;
#ifdef CALLBACK_MULTITHREADED
    if (access.wait () == true)
    {
#endif
      s = operator[] ("returnValue\0");
#ifdef CALLBACK_MULTITHREADED
      access.unlock ();
    }
#endif

    return s;
  }
// IMPL
  /**
   * This is the function that is executed when the callback is invoked.
   */
  virtual void operator () () = 0;
#ifdef CALLBACK_MULTITHREADED
  /**
   * Access the current state of the callback.
   * NB: this accesses the state mutex to prevent crazy, if the mutex
   * is locked and cannot be attained, the function will return INVALID
   * @returns the current state of the mutex, or INVALID if the mutex could not be locked.
   */
  const IntegerSetType state () const
  {
    IntegerSetType  s = INVALID;

    if (access.wait () == true)
    {
      s = operator[] ("state\0");
      access.unlock ();
    }

    return s;
  }
  /**
   * @returns true if the callback is to be waited on, false otherwise
   */
  const bool isWaiter () const
  {
    bool  w = false;

    if (access.wait () == true)
    {
      if (operator[] ("waitOn\0") == 1)
      {
        w = true;
      }

      access.unlock ();
    }

    return w;
  }
  /**
   * @returns a handle to the thread executing this callback, will be NULL if it is not executing
   */
  HANDLE thread () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("thread\0"));
      access.unlock ();
    }

    return h;
  }
  /**
   * @returns a handle to the cancel event for this callback, NULL if access mutex cannot be obtained
   */
  HANDLE cancel () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("cancel\0"));
      access.unlock ();
    }

    return h;
  }
  /**
   * @returns handle to the completion event, NULL if access mutex cannot be obtained.
   */
  HANDLE complete () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("complete\0"));
      access.unlock ();
    }

    return h;
  }
  /**
   * @returns a handle to beep event, or INVALID_HANDLE_VALUE if access cannot be obtained.
   */
  HANDLE beep () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("beep\0"));
      access.unlock ();
    }

    return h;
  }
  /**
   * returns true if the cancel event has been signaled
   */
  const bool canceled () const
  {
    bool t = false;

    if (access.wait () == true)
    {
      if (((*this) && "cancel\0") == true)
      {
        t = Utils::Event::signaled (cancel ());
      }

      access.unlock ();
    }

    return t;
  }
// MISC
  /**
   * resets the state of the callback to ready, resets the cancel event
   * and signals the complete event.
   * NB: This is not used to cancel execution
   */
  void reset ()
  {
    setState (READY);
    Utils::Event::reset (cancel ());
    Utils::Event::reset (beep ());
    Utils::Event::signal (complete ());
  }
// SIGNALING
  /**
   * Signals the cancelation event for a thread executing this callback.
   * Does not confirm reciept, nor reset the event.
   */
  void cancelThread ()        {if (access.wait () == true) {if (((*this)&&"cancel\0") == true)  {Event::signal (cancel ());}}  access.unlock ();}
  /**
   * Signals the complete event for this thread.
   * NB: This is called by the threadEntryPoint after operator() returns,
   * so a thread signals completion regardless of whether the computation
   * was decided.
   * Completion indicates a thread exited smoothly (or is in the process of exiting).
   * FIXME: these semantics are incorrect. completed () mutates the object, cancelled tests
   */
  void completed ()          {if (access.wait () == true) {if (((*this)&&"complete\0") == true)  {Event::signal (complete ());}}  access.unlock ();}
  void beepThread ()          {if (access.lock () == true) {if (((*this)&&"beep\0") == true)    {Event::signal (beep ());}}    access.unlock ();}
  /**
   * Signals the cancelation event for a thread.
   * Confirms receipt by waiting cancelTime () ms for the completeEvent () to signal.
   * If this does not signal, completion is not confirmed.
   * Regardless, the cancelEvent () is reset (with a cancelTime () period allowed
   * for its signalling to be detected).
   * @returns true if the completeEvent () was signaled, false otherwise.
   */
  bool cancelThreadConfirm ()
  {
    // make sure we aren't in the same thread as the thing we are trying to signal (deadlock)
    if (((*this)&&"thread\0") && ((HANDLE)((*this)["thread\0"])) != GetCurrentThread ())
    {
      // signal the cancel event for the execution thread
    //  Note::notemsg ("%s cancelling...\0", prefix ().c_str ());
      if (Utils::Event::signaled (cancel ()) == false)
      {
        Utils::Event::signal (cancel ());
      }

      if (Utils::Event::waitOn (complete (), Utils::signalObservationTime ()) == false)
      {
    //    Note::notemsg ("%s failed to cancel\0", prefix ().c_str ());
    //    Utils::Event::reset (cancel ());
        return false;
      }
      else
      {
    //    Note::notemsg ("%s cancelled\0", prefix ().c_str ());
        // reset the cancel signal
        Utils::Event::reset (cancel ());
        return true;
      }
    }
    else
    {
      return false;
    }
  }
#endif
};

#ifdef CALLBACK_MULTITHREADED
/**
 * Signal listener is a callback which runs forever in its own thread.
 * When the signal it's listening for is fired, it runs the operator() function,
 * resets the signal and returns to its original waiting state.
 */
class SignalListener : public IntegerSet
{
protected:
  Mutex    access;

  static unsigned int __stdcall threadEntryPoint (void *lParam)
  {
    SignalListener  *sl;
    HANDLE      handles[2] = {0};
    unsigned int  sid;

    if ((sl = reinterpret_cast<SignalListener*>(lParam)) == NULL)    {Note::notemsg ("Invalid thread params\0"); return FALSE;}

    handles[0] = sl->signal ();
    handles[1] = sl->cancel ();

    // wait
    while ((sid = WaitForMultipleObjects (2, handles, FALSE, INFINITE) - WAIT_OBJECT_0 ) != 1)
    {
      (*sl) ();    // execute the listener
    };

    return 1;
  }

public:
  SignalListener (HANDLE signal)
    : access ("siglst\0", 100)
  {
    set ("signal\0", (IntegerSetType)signal);
    set ("cancel\0", (IntegerSetType)Utils::Event::create (this, "cancel\0"));
    set ("thread\0", 0);

    // set the thread running
    HANDLE  thd;

    Utils::Event::reset (cancel ());    // reset the completion event (so the callback is incomplete)

    const_cast<HANDLE>(thd) = reinterpret_cast<HANDLE>(
                  _beginthreadex (NULL, 0,
                          threadEntryPoint,
                          reinterpret_cast<void*>(this),
                          CREATE_SUSPENDED,
                          NULL));

    if (thd == NULL)                  // check we have a thread
    {
      Note::notemsg ("_beginthreadex failed\0");
    }
    else
    {
      set ("thread\0", (IntegerSetType)thd);      // store the thread handle incase we need to close
      ResumeThread (thread ());            // resume the thread
    }
  }
  virtual ~SignalListener ()
  {
    Utils::Event::signal (cancel ());
    Sleep (100);
    Utils::Event::close (cancel ());
  }
  /**
   * @returns a handle to the thread executing this callback, will be NULL if it is not executing
   */
  HANDLE thread () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("thread\0"));
      access.unlock ();
    }

    return h;
  }
  /**
   * @returns a handle to the cancel event for this callback, NULL if access mutex cannot be obtained
   */
  HANDLE cancel () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("cancel\0"));
      access.unlock ();
    }

    return h;
  }
  HANDLE signal () const
  {
    HANDLE  h = INVALID_HANDLE_VALUE;

    if (access.wait () == true)
    {
      h = (HANDLE)(operator[] ("signal\0"));
      access.unlock ();
    }

    return h;
  }
  virtual void operator() (void) = NULL;
};

typedef ParameterSet<SignalListener*>  ListenerSet;
#endif

// MISC
class ProgressCallback : public Callback
{
public:
  virtual void reset () = 0;          // resets the progress bar to initial state
  virtual void setsize (int size) = 0;    // sets the size of the progress bar
  virtual void setincrement (int inc) = 0;  // sets the increment advancement
  virtual void exit () = 0;          // completion
};
};
#endif
