namespace Patterns
{
class Touch
{
private:
  // monitoring
  unsigned long   _tchcd;    ///< touch code
  bool            _tched;    ///< touch flag

protected:
  const unsigned long touchCD () const
  {
    return _tchcd;
  }
  const bool touchFlag () const
  {
    return _tched;
  }
  const bool setTouchCD (const unsigned long t)
  {
    // NB: if the same parameter is modified (with different values)
    // this method does NOT highlight that (future changes are masked).
    // The hash value needs to be H(PARAM, VALUE) not H(PARAM)
  //  if (t != touchCD ())
    {
      _tchcd = t;
      _tched = true;
      return true;
    }

    return false;
  }
public:
  /**
   * This is the function that should be implemented by subclasses.
   * It is required to test all internal variables of the subclass and set the
   * touch flag to true if values have changed.
   * It is also acceptable to update this function during mutators (set(), etc)
   * and allow the value to carry over.
   * Subclasses should call the superclass function.
   * This function (the root of the class chain) will set the flag to true.
   * The return value indicates whether the change resulted in any actual modification
   * of values.
   * If this is not possible (or too expensive) to test the implementation should
   * always return true.
   * @returns true if the touch flag was previously unset, false otherwise.
   */
  virtual bool touch ()
  {
    return (_tched = true);
  }
  /**
   * clears the touched flag
   */
  void untouch ()
  {
    _tched = false;
  }
  /**
   * tests if the touched flag is set.
   * @returns true if the touch flag is set, false otherwise
   */
  const bool touched () const
  {
    return _tched;
  }
};
};