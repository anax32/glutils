#undef CALLBACK_PIPE_DETAILS

namespace Utils
{
/**
 * CALLBACKSET_CREATETHREAD is an important define.
 * If defined, the win32 CreateThread() function is used to spawn threads,
 * otherwise, the CRT _beginthreadex() function is used.
 * According to msdn, there are no longer any issues with using CRT functions
 * in threads created with CreateThread:
 * http://support.microsoft.com/kb/104641/en-us
 * However, I have had some intermittent memory leaks on closing threads
 * and program exit. Using _beginthreadex() solved these.
 */
#ifdef CALLBACK_MULTITHREADED
#define CALLBACKSET_NOTHREADS    0
#define CALLBACKSET_CREATETHREAD  1
#define CALLBACKSET_THREADPOOL    2
#define CALLBACKSET_BEGINTHREADEX  3
//#define CALLBACKSET_THREADMETHOD  CALLBACKSET_THREADPOOL
#define CALLBACKSET_THREADMETHOD  CALLBACKSET_BEGINTHREADEX
#else
#define CALLBACKSET_THREADMETHOD  CALLBACKSET_NOTHREADS
#endif

#undef CALLBACK_STACK_BROADCAST    // walk the stack and pipe the result to a monitor

#ifdef CALLBACK_MUTLITHREADED
#undef CALLBACK_DEBUG_MT_EMULATE_ST  // emulate signle thread execution with mutlithreaded callbacks by waiting on each callback
#endif
/**
 * Once a callback has been created, it can only be invoked or removed.
 * This parameter set invokes the callback object via the operator("name")
 */
class CallbackSet : public ParameterSet<Callback*>
{
public:
#ifdef CALLBACK_MULTITHREADED
  static HANDLE spawnThread (Callback* cb)
  {
    HANDLE  thd;

    // LOCK the callback while we set some things up
    // FIXME: the locks are called inside the callback function and would challenge with this
  //  if (cb->access.lock () == false)                      {Note::notemsg ("%s could not lock\0", pl->first.c_str ());   return;}

    cb->reset ();                      // reset the callback
    Utils::Event::reset (cb->complete ());          // reset the completion event (so the callback is incomplete)
    cb->setState (Callback::ALLOCATED);            // we have started the process and are not available

  //  const_cast<HANDLE>(clr) = GetCurrentThread ();      // pass the ThreadedCallback to CreateThread
#if CALLBACKSET_THREADMETHOD == CALLBACKSET_CREATETHREAD
    const_cast<HANDLE>(thd) = CreateThread (NULL,
                                            0,
                                            threadEntryPoint,        // entry point
                                            reinterpret_cast<LPVOID>(tb),  // pointer to the func
                                            CREATE_SUSPENDED,
                                            NULL);    // create in a suspended state
#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_BEGINTHREADEX
    const_cast<HANDLE>(thd) = reinterpret_cast<HANDLE>(
                          _beginthreadex (NULL,
                                          0,
                                          threadEntryPoint,
                                          reinterpret_cast<void*>(cb),
                                          CREATE_SUSPENDED,
                                          NULL));
#endif
    return thd;
  }
#endif
private:

  /**
   * Thread entry point, do not call directly.
   * This function invokes the overloaded operator() function of the Callback object,
   * albeit from inside another thread.
   * This function also handles the cleanup of the context set by the ThreadedCallback
   * i.e., the state and thread parameters.
   * NB: The return value of this is the return value of ThreadFunction::operator(),
   * but there is no way to communicate this to the caller.
   * Retvals are discarded.
   */
#if CALLBACKSET_THREADMETHOD == CALLBACKSET_NOTHREADS
#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_CREATETHREAD
  static DWORD WINAPI threadEntryPoint (LPVOID lParam)
#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_THREADPOOL
//  PTP_POOL        pool;
//  PTP_CLEANUP_GROUP    cleanupgroup;
  IntegerSet        poolWork;

  static void CALLBACK threadEntryPoint (PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
  {
    Callback  *cb = NULL;

    if ((cb = reinterpret_cast<Callback*>(context)) == NULL)
    {
      return;
    }

  //  tpc->idx = InterlockedIncrement (&tpc->cur)-1;
    // execute the thread function
    if (cb->canceled () == false)
    {
      (*cb) ();
    }

    // signal that execution is complete
    cb->completed ();

    // reset the execution context contained in the ThreadedCallback object
    // and let the thread end and be collected by the OS
    cb->access.lock ();
    cb->setState (Callback::READY);
    cb->access.unlock ();
  }
#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_BEGINTHREADEX
  static unsigned int __stdcall threadEntryPoint (void *lParam)
  {
    Callback    *cb;
    IntegerSetType  retval;

    if ((cb = reinterpret_cast<Callback*>(lParam)) == NULL)
    {
      Note::notemsg ("Invalid thread params\0");
      return FALSE;
    }

    // execute the thread function
    if (cb->canceled () == true)
    {
      retval = FALSE;
    }
    else
    {
      (*cb) ();
      retval = cb->returnValue ();
    }

    // signal that execution is complete
    cb->completed ();

    // reset the execution context contained in the Callback object
    // and let thread termination be collected by the OS
    if (cb->access.lock () == true)
    {
      cb->set ("thread\0", NULL);
    } cb->access.unlock ();

    cb->setState (Callback::READY);

    // retval
    return static_cast<unsigned int>(retval);
  }
#endif
  /**
   * runCallback invokes execution of the callback object, after checking the 
   * state and initialising the object correctly.
   * FIXME: run threaded callbacks in a thread pool.
   * if the callback is a waiter (isWaiter ()) the callback is executed by the
   * current thread, otherwise a new thread is spawned for execution.
   */
  void runCallback (ConstParamList& pl) const
  {
    Utils::Callback  *cb = pl->second;

#ifdef CALLBACK_MULTITHREADED
  //  if ((((*tb) && "delcancel\0") == true) && (tb->cancelThreadConfirm () == false))    // cancel the previous invokation
    if (Utils::Event::signaled (cb->complete ()) == false)
    {
      if ((((*cb) && "delcancel\0") == false) ||
           ((*cb)["delcancel\0"] == 0))
      {
        // previous is not complete and it doesn't have its own cancel signal,
        // so we don't know what to do here, just return and accept failure.
        return;
      }
      else
      {
        if (cb->cancelThreadConfirm () == false)
        {
#ifdef _DEBUG
          const unsigned int  N = 5;
          unsigned int    i;
        
          // test the delay
          for (i=0;i<N;i++)
          {
            if (Utils::Event::waitOn (cb->complete (), Utils::signalObservationTime ()) == true)
              break;
          }

          if (i<N)
          {
            std::cout << pl->first << " canceled after " << i << " wait cycles" << std::endl;
          }
          else
          {
            std::cout << pl->first << " would not cancel and confirm" << std::endl;
            return;
          }
#else
          // could not cancel, so exit
          return;
#endif
        }
      }

      // all good, previous is dismissed, continue
      std::cout << pl->first << " canceled, restarting" << std::endl;
    }

    if (cb->isWaiter () == true)
    {
      cb->setState (Callback::EXECUTING);          // now we are in the executing state
#else  // IF NOT MULTITHREADED
    // NOTHING HAPPENS. NB: CONDITIONAL BRACKET NOT PRESENT
#endif  // ENDIF-IF MULTITHREADED

#ifdef CALLBACK_STACK_BROADCAST
      Note::walkStack ();
#endif

#ifdef CALLBACK_PIPE_DETAILS
      STACK_INFO  si;

      si.fnLocation = reinterpret_cast<STACK_INFO::MEM_LOC>(pl->second);
      strcpy_s (si.funcname, MAX_PATH, pl->first.c_str ());

      std::ostringstream  os;
      os << si;
      Utils::Pipe::write (Note::pipe (), os.str ().c_str (), static_cast<unsigned int>(os.str ().length ()));

      Note::notemsg ("%s\0", os.str ().c_str ());
#endif

      // execute the callback in this thread
      (*cb) ();
#ifdef CALLBACK_MULTITHREADED
#ifdef CALLBACK_DEBUG_MT_EMULATE_ST
      // ideally we would use this method, but...
      ResumeThread (cb->thread ());
      WaitForSingleObject (cb->thread (), INFINITE);
#endif
    }
#endif

#ifdef CALLBACK_MULTITHREADED
    else
    {
#if CALLBACKSET_THREADMETHOD == CALLBACKSET_THREADPOOL
      if ((poolWork && pl->first) == true)
      {
        SubmitThreadpoolWork (reinterpret_cast<PTP_WORK>(poolWork[pl->first]));
      }
      else
      {
        std::cout << "No work object for '" << pl->first << "'" << std::endl;
      }
#else
      HANDLE  thd;

      thd = spawnThread (cb);
#endif

      if (thd == NULL)                    // check we have a thread
      {
        std::cout << "CreateThread () failed" << std::endl;
        Event::signal (cb->complete ());            // signal the complete event (for whatever reason,
                                  // this computation is complete)
        cb->setState (Callback::READY);
      //  cb->access.unlock ();
      }

      cb->set ("thread\0", (IntegerSetType)thd);      // store the thread handle incase we need to close
      
      // execute in another thread
      cb->setState (Callback::EXECUTING);
      ResumeThread (cb->thread ());            // resume the thread
    }
#endif

#if 0
// OLD STYLE had two forms of callback object
    Utils::Callback      *cb;
    Utils::ThreadedCallback  *tb;

    // get the callback associated with that name
    if ((cb = dynamic_cast<Callback*>(pl->second)) == NULL)
    {
      return;
    }

    if ((tb = dynamic_cast<ThreadedCallback*>(pl->second)) != NULL)
    {
    //  if ((((*tb) && "delcancel\0") == true) && (tb->cancelThreadConfirm () == false))    // cancel the previous invokation
      if (Utils::Event::signaled (tb->complete ()) == false)
      {
        if ((((*tb) && "delcancel\0") == false) || ((*tb)["delcancel\0"] == 0))
        {
          // previous is not complete and it doesn't have its own cancel signal,
          // so we don't know what to do here, just return and accept failure.
          return;
        }
        else
        {
          if (tb->cancelThreadConfirm () == false)
          {
#ifdef _DEBUG
            const unsigned int  N = 5;
            unsigned int    i;
        
            // test the delay
            for (i=0;i<N;i++)
            {
              if (Utils::Event::waitOn (tb->complete (), tb->cancelTime ()) == true)
                break;
            }

            if (i<N)
            {
              Note::notemsg ("%s canceled after %i wait cycles\0", pl->first.c_str (), i);
            }
            else
            {
              Note::notemsg ("%s would not cancel and confirm\0", pl->first.c_str ());
              return;
            }
#else
            // could not cancel, so exit
            return;
#endif
          }
        }

        // all good, previous is dismissed, continue
        Note::notemsg ("%s canceled, restarting\0", pl->first.c_str ());
      }

      HANDLE  thd;

      // lock the tcb while we set some things up
      if (tb->access.lock () == false)
      {
        Note::notemsg ("%s could not lock\0", pl->first.c_str ());
        return;
      }

      tb->reset ();
      Utils::Event::reset (tb->complete ());
      tb->setState (Callback::ALLOCATED);            // we have started the process and are not available

    //  const_cast<HANDLE>(clr) = GetCurrentThread ();      // pass the ThreadedCallback to CreateThread
#if CALLBACKSET_THREADMETHOD == CALLBACKSET_CREATETHREAD
      const_cast<HANDLE>(thd) = CreateThread (NULL, 0,
                    threadEntryPoint,        // entry point
                    reinterpret_cast<LPVOID>(tb),  // pointer to the func
                    CREATE_SUSPENDED, NULL);    // create in a suspended state
#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_BEGINTHREADEX
      const_cast<HANDLE>(thd) = reinterpret_cast<HANDLE>(
                    _beginthreadex (NULL, 0,
                            threadEntryPoint,
                            reinterpret_cast<void*>(tb),
                            CREATE_SUSPENDED,
                            NULL));
#endif

      if (thd == NULL)                    // check we have a thread
      {
        Note::notemsg ("CreateThread () failed\0");
        Event::signal (tb->complete ());            // signal the complete event (for whatever reason,
                                  // this computation is complete)
        tb->setState (Callback::READY);
        tb->access.unlock ();
      }
      else
      {
        tb->set ("thread\0", (IntegerSetType)thd);      // store the thread handle incase we need to close
        tb->setState (Callback::EXECUTING);          // now we are in the executing state
        tb->access.unlock ();

#ifdef CALLBACK_MULTITHREADED
        ResumeThread (tb->thread ());            // resume the thread
#else
        ResumeThread (tb->thread ());            // test single threaded logic
        WaitForSingleObject (tb->thread (), INFINITE);
#endif
      }
    }
    else  // STANDARD IN-THREAD CALLBACK
    {
      cb->setReturnValue (FALSE);
      // execute in this thread, much simpler
      cb->setState (Callback::ALLOCATED);
      cb->setState (Callback::EXECUTING);
      (*cb) ();
      cb->setState (Callback::READY);
    }
#endif
  }
public:
#if CALLBACKSET_THREADMETHOD == CALLBACKSET_NOTHREADS

#elif CALLBACKSET_THREADMETHOD == CALLBACKSET_THREADPOOL
  void set (const Param& name, Callback* newValue)
  {
    if (newValue->isWaiter () == false)
    {
      if (poolWork.find (name) != poolWork.end ())
      {
        remove (name);
      }

      poolWork.set (name, reinterpret_cast<IntegerSetType>(CreateThreadpoolWork (threadEntryPoint, newValue, NULL)));

      Note::notemsg ("Callback for '%s' pooled in %08x\0", name.c_str (), poolWork[name]);
    }

    ParameterSet<Callback*>::set (name, newValue);
  }
  void remove (const Param& name)
  {
    if ((poolWork && name) == true)
    {
      CloseThreadpoolWork (reinterpret_cast<PTP_WORK>(poolWork[name]));
      poolWork.remove (name);
    }

    ParameterSet<Callback*>::remove (name);
  }
#endif
  /**
   * Invokes a callback by "name" operator by finding the functor
   * and passing it to runCallback.
   * @returns FALSE if callback is not found, or returnValue () of functor otherwise.
   */
  const IntegerSetType operator() (const Param& name) const
  {
    ConstParamList  pl;

    if ((pl = ParameterSet::find (name)) != end ())
    {
#if 0
      Note::tmmark ();
      runCallback (pl);
      Note::tmfunc ("%s\0", pl->first.c_str ());
#else
      (*pl->second) ();
#endif
      return pl->second->returnValue ();
    }

    return FALSE;
  }
  /**
   * Invokes a callback by the hashcode of the "name" operator by finding
   * the functor with findHash () and passing the result to runCallback ()
   * @returns FALSE if callback is not found, or returnValue () of functor otherwise.
   */
  const IntegerSetType operator() (const unsigned int& hashcode) const
  {
    ConstParamList pl;

    if ((pl = ParameterSet::findHash (hashcode)) != end ())
    {
      runCallback (pl);
      return pl->second->returnValue ();
    }

    return FALSE;
  }
};
};