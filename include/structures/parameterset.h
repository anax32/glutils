#ifndef STRUCTURES_PARAMETERSET_H
#define STRUCTURES_PARAMETERSET_H
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <deque>
#include <functional>
#include <limits>
#include <algorithm>
#include <initializer_list>

/**
 * Parameter type
 *  key for referencing values
 */
typedef    char            param_base_type;

// NB: we can't store 64bit hash values in windows context menus (using AppendMenu command ids)
// so we have to use 32 bit values and hash accordingly.
#ifdef _WIN64
//typedef    __int64       HashType;
typedef    unsigned int    HashType;
//typedef      size_t      HashType;    // wrong size type prevents matches in the parameterset findhash function
#else
typedef    unsigned int    HashType;
#endif
class Param : public std::basic_string<param_base_type>
{
public:
// CONSTRUCTORS
  Param (const unsigned int i)
    : std::basic_string<param_base_type> (reinterpret_cast<const param_base_type *>(&i),
                        reinterpret_cast<const param_base_type *>(&i) + sizeof (unsigned int))
  {}
  Param (const param_base_type *c)              : std::basic_string<param_base_type> (c)    {}
  Param (const std::basic_string<param_base_type> s)      : std::basic_string<param_base_type> (s)    {}
  Param (const param_base_type *s, const param_base_type *e)  : std::basic_string<param_base_type> (s, e)    {}
  Param (const Param& p)                    : std::basic_string<param_base_type> (p)    {}
// COMPARISION
  bool operator== (const Param& p) const
  {
    return (compare (p) == 0);
  }
  bool operator== (const Param* p) const
  {
    return (compare (*p) == 0);
  }
  bool operator== (const param_base_type *s) const
  {
    return (compare (s) == 0);
  }
  bool operator!= (const Param& p) const
  {
    return (!(*this == p));
  }
  bool operator!= (const Param* p) const
  {
    return (!(*this == p));
  }
  bool operator!= (const param_base_type *s) const
  {
    return (!(*this == s));
  }
// UTILS
  /**
   * @returns a number identifying this param value
   * //  return std::hash<std::basic_string<char> >() (this->c_str ());  // FIXME: compiler warning, does the cast truncate?
   */
  HashType hash () const
  {
    return static_cast<HashType>(std::hash<std::basic_string<char> >() (this->c_str ()));
  }
};
/**
 * List of Params
 */
#if 1
//#ifdef _VECTOR_
typedef std::vector<Param>  ParameterList;  ///< confusing name?
//#endif
#else
//#ifdef _DEQUE_
typedef std::deque<Param>  ParameterList;
//#endif
#endif

class ChangeHistory
{
protected:
  Param     _lcv_nm;    // name of the variable which changed
  clock_t   _lcv_t;      // time of change (in clock () ticks)
public:
  ChangeHistory ()
    : _lcv_nm ("unknown\0"), _lcv_t (0)
  {}
// MUTATORS
  virtual void setLCV (const Param& paramName)
  {
    _lcv_nm = paramName;
    _lcv_t = clock ();
  }
// ACCESSORS
  const Param& lastChangedParameter () const
  {
    return _lcv_nm;
  }
  unsigned int lastChangedTime () const
  {
    return _lcv_t;
  }
};
template<class T>
class ChangeHistoryValue : public ChangeHistory
{
protected:
  T    _lcv_val;    // previous value of the variable
public:
  ChangeHistoryValue ()
    : ChangeHistory ()
  {}
  virtual void setLCV (const Param& paramName, const T& paramValue)
  {
    ChangeHistory::setLCV (paramName);
    _lcv_val = paramValue;
  }
  const T& lastChangedValue () const
  {
    return _lcv_val;
  }
};

class SelectedName
{
protected:
  Param    __sel;
public:
  SelectedName ()
    : __sel ("?\0")
  {}
  const Param& selected () const
  {
    return __sel;
  }
  void select (const Param& newSelection)
  {
    __sel.assign (newSelection);
  }
  void deselect ()
  {
    __sel.assign ("?\0");
  }
  bool interest () const
  {
    return (__sel != "?\0");
  }
  void clear ()
  {
    deselect ();
  }
  bool isSelected (const Param& testValue) const
  {
    return ((__sel != Param ("?\0")) && (__sel == testValue));
  }
};

/**
 * A wrapper of the std::map class which links a
 * name (parameter label) to a value (parameter).
 * The key is the Param type (named identifier), the value is of type T in the template.
 * Values are stored in a stl::map member variable, and accessed transparently
 * through this wrapper class.
 * FIXME: Use templated functions for clearing values.
 */
template<class T>
class NoOp
{
public:
  void operator() (typename T::iterator::value_type pl)
  {}
};

template<class T, class CLR = NoOp<std::map<Param, T> > >
class ParameterSet : //public Patterns::Persistable,
                     public SelectedName,
                     public ChangeHistoryValue<T>
{
public:
  typedef std::map<Param, T>      ParamMap;  ///< map for key to values

  typedef typename ParamMap::iterator                 ParamList;      ///< iterator for all keys
  typedef typename ParamMap::const_iterator           ConstParamList; ///< const iterator
  typedef typename ParamMap::reverse_iterator         ParamListR;
  typedef typename ParamMap::const_reverse_iterator   ConstParamListR;

private:
  ParamMap  m;        ///< key-value pairs
  T      NO_ATTR_VAL;  ///< bad/null value indicator

public:
  ParameterSet (const T UNSET_DATA_VALUE)
    : NO_ATTR_VAL (UNSET_DATA_VALUE)
  {}
  ParameterSet ()                          {}
  ParameterSet (std::initializer_list<typename ParamMap::value_type> c)
    : m (c)
  {
    //std::cout << "initializer_list" << std::endl;
  }
  ParameterSet (const ParameterSet<T>& ps)
  {
#ifdef _DEBUG
    std::cout << "COPY&" << std::endl;
#endif
    (*this) = ps;
  }
  ParameterSet (const ParameterSet<T> *ps)
  {
#ifdef _DEBUG
    std::cout << "COPY*\0" << std::endl;
#endif

    ParameterSet::ConstParamList  pl;

    for (pl=ps->begin (); pl!=ps->end (); pl++)
    {
      (*this)[pl->first] = pl->second;
    }
  }
  virtual ~ParameterSet ()
  {
    clear ();
  }
// STATE
  /**
   * @returns true if there are no keys in the parameter set
   */
  const bool empty () const
  {
    return m.empty ();
  }
  /**
   * @returns the number of keys in the parameter set
   */
  const size_t parameterCount () const
  {
    return m.size ();
  }
// ITERATORS
  /**
   * @returns an iterator at the first key of the parameter set
   */
  ParamList begin ()
  {
    return m.begin ();
  }
  ParamListR rbegin ()
  {
    return m.rbegin ();
  }
  ConstParamList begin () const
  {
    return m.begin ();
  }
  ConstParamListR rbegin () const
  {
    return m.rbegin ();
  }
  /**
   * @returns an iterator at the last+1 key of the parameter set
   */
  ParamList end ()
  {
    return m.end ();
  }
  ParamListR rend ()
  {
    return m.rend ();
  }
  ConstParamList end () const
  {
    return m.end ();
  }
  ConstParamListR rend () const
  {
    return m.rend ();
  }
  const ParameterSet<T>& operator= (const ParameterSet<T>& ps)
  {
    this->set (ps);
    return (*this);
  }
  /**
   * @returns the parameter value associated with the label
   */
  const T& operator[] (const Param& name) const
  {
    ConstParamList  pl;

    if ((pl = m.find (name)) != m.end ())
    {
      return pl->second;
    }
    else
    {
      std::cout << "PARAMETERSET(&)::find ('" << name << "')" << std::endl;

      for (pl=begin ();pl!=end ();pl++)
      {
        std::cout << "[" << indexOf (pl->first) << "] '" << pl->first << "'" << std::endl;
      }
      return noValue ();
    }
  }
  T& operator[] (const Param& name)
  {
#if _DEBUG
    if (((*this) && name) == false)
    {
    //  Note::notemsg ("Creating: '%s'\0", name.c_str ());
    //  setLCV (name);
    }
#endif
    // non-const objects allow creation of parameters
    return m[name];
  }
  /**
   * @returns true if there is a parameter value associated with the label.
   */
  const bool operator&& (const Param& name) const
  {
    return (m.find (name) != m.end ());
  }
  void copy (const ParameterSet<T>& ps)
  {
    for (auto& pl : ps)
    {
      (*this)[pl.first] = pl->second;    // FIXME LCV
    }
  }
// MUTATORS
  virtual void set (const ParameterSet<T>& pm)
  {
    clear ();
    
    for (auto& pl : pm)
    {
      set (pl.first, pl.second);    // FIXME LCV
    }
  }
  /**
   * set a parameter to a new value,
   * the parameter will be created if it does not exist.
   * @param name parameter key (label)
   * @param newValue new value of the parameter, will be copied.
   */
  virtual void set (const Param name, const T newValue)
  {
    this->setLCV (name, m[name]);
    m[name] = newValue;
  }
//  virtual void push_back (const Param name)            {set (name, NO_ATTR_VAL);  setLCV (name);}
  /**
   * removes a parameter from the set
   * @param name parameter label of the parameter to delete
   */
  virtual void remove (const Param& name)
  {
    this->setLCV (name, m[name]);
    m.erase (name);
  }
  /**
   * removes all key/value pairs from the parameter set
   */
  virtual void clear ()
  {
  //  std::for_each (begin (), end (), CLR ());
    SelectedName::clear ();
    m.clear ();
    this->setLCV ("::all\0", NO_ATTR_VAL);
  }
  /**
   * @returns a refernce to the null data indicator set for this object
   */
  const T& noValue () const
  {
#ifdef _DEBUG
    std::cout << "noValue () accessed" << std::endl;
#endif
    return NO_ATTR_VAL;
  }
// FIND
  ParamList find (const T& value)
  {
#if 1
    ParamList  pl;

    for (pl=begin ();pl!=end()&&pl->second!=value;pl++)
      ;;

    return pl;
#else
    // FIXME: migrate to this usage, requires a change to the return type
    for (const auto& p : m)
    {
      if (p.second == value)
        return p;
    }
#endif
  }
  ConstParamList find (const T& value) const
  {
    ConstParamList  pl;

    for (pl=begin ();pl!=end ()&&pl->second!=value;pl++)
      ;;

    return pl;
  }
  ParamList find (const Param& name)
  {
    return m.find (name);
  }
  ConstParamList find (const Param& name) const
  {
    return m.find (name);
  }
  ParamList findHash (const HashType hash)
  {
    ParamList  pl;

    for (pl=begin ();pl!=end ();pl++)
    {
      if (pl->first.hash () == hash)
        break;
    }
    return pl;
  }
  ConstParamList findHash (const HashType hash) const
  {
    ConstParamList pl;

    for (pl=begin ();pl!=end ();pl++)
    {
      if (pl->first.hash () == hash)
        break;
    }
    return pl;
  }
  const unsigned int indexOf (const Param& name) const
  {
    return static_cast<unsigned int>(std::distance (begin (), find (name)));
  }
  ParamList byIndex (const unsigned int index)
  {
    ParamList pl = begin (); std::advance (pl, index); return pl;
  }
  ConstParamList byIndex (const unsigned int index) const
  {
    ConstParamList pl = begin (); std::advance (pl, index); return pl;
  }
#if 0
// PERSISTANCE
  virtual const char *persistableNodeID () const    {return persistableTypename<T> ();}
  virtual const char *persistableNodeLabelID () const  {return "name\0";}
  virtual const char *persistableNodeValueID () const  {return "value\0";}
  virtual void write (std::ostream& os) const
  {
    ConstParamList  pl;

  //  Patterns::Persistable::pushObject<ParameterSet<T> > (os);

    for (pl=begin (); pl!=end (); pl++)
    {
    //  Patterns::Persistable::pushParameter<T> (os, pl->first.c_str (), pl->second);

      Patterns::Persistable::pushObject<T> (os, "name\0", pl->first.c_str ());  /// persistableNodeLabelID?
      os << pl->second;
      Patterns::Persistable::popObject<T> (os);
    }

  //  Patterns::Persistable::popObject<ParameterSet<T> > (os);
  }
  virtual void read (std::istream& is)
  {

  }
  friend std::ostream& operator<< (std::ostream& os, const ParameterSet<T>& ps)
  {
    ps.write (os);
    return os;
  }
/*
template<class T>
std::ostream&  operator<< (std::ostream& os, const ParameterSet<T>& ps)
{
  ps.write (os);
  return os;
}*/
#endif

};

// specialised template for pointer types which deletes the item when remove () or clear () is fired.
template<class T>
class ParameterSet<T*> : //public Patterns::Persistable,
                         public SelectedName,
                         public ChangeHistory
{
public:
  typedef std::map<Param, T*>        ParamMap;  ///< map for key to values
  typedef typename ParamMap::iterator               ParamList;    ///< iterator for all keys
  typedef typename ParamMap::reverse_iterator       ParamListR;
  typedef typename ParamMap::const_iterator         ConstParamList;  ///< const iterator
  typedef typename ParamMap::const_reverse_iterator ConstParamListR;

private:
  ParamMap  m;        ///< key-value pairs
  void*    NO_ATTR_VAL;  ///< bad/null value indicator
                ///< FIXME: The badvalue is a sink for all operations which may be
                ///< performed on the accessed type.  It should be created here
                ///< as part of the ParameterSet constructor, but we usually need
                ///< to specialise the type to notify us when it is in use.
public:
  ParameterSet ()
    : NO_ATTR_VAL (NULL)
  {}
  ParameterSet (const ParameterSet<T*>& ps)
    : NO_ATTR_VAL (ps.NO_ATTR_VAL)
  {
#ifdef _DEBUG
    std::cout << "!!!!!*(&)" << std::endl;
#endif
    m = ps.m;
  }
  ParameterSet (const ParameterSet<T*>* ps)
    : NO_ATTR_VAL (ps->NO_ATTR_VAL)
  {
#ifdef _DEBUG
    std::cout << "!!!!!*(*)" << std::endl;
#endif
    m = ps->m;
  }
  /**
   * Desctructor: clears the map deleting all pointers and
   * also deletes the noValue () object if one has been set
   */
  virtual ~ParameterSet ()
  {
    clear ();
  }
  const ParameterSet<T*>& operator= (const ParameterSet<T*>& ps)
  {
    this->set (ps);
    return (*this);
  }
  /**
   * @param name the key associated with the pointer item
   * @returns the parameter value associated with the label
   */
  const T& operator[] (const Param& name) const
  {
    ConstParamList  pl;

    if ((pl = m.find (name)) != m.end ())
    {
      return (*pl->second);
    }
    else
    {
      std::cout << "const PARAMETERSET(*)::find ('" << name << "')" << std::endl;

      for (pl=begin ();pl!=end ();pl++)
      {
        std::cout << "\t[" << indexOf (pl->first) << "] " << (void*)pl->second << " '" << pl->first << "'" << std::endl;
      }

      throw std::exception ("");
    }
  }
  /**
   * the non-const accessor for pointer types behaves exactly as the const
   * accessor; It does NOT create new objects (which may allow previous
   * pointers to be overwritten and cause memory leaks).
   * To add new parameters always use the set () function.
   * If non-const (i.e., creational) access to a non-extant pointer object
   * is attempted a note is issued and noValue () is returned.
   * To allow valid failures the noValue () should be set to an object
   * which performs bottom semantics (generally, this behaviour should be avoided).
   * @param name key of the pointer to access
   * @returns pointer to the stored object or noValue () if there is no pairing.
   */
  T& operator[] (const Param& name)
  {
    ConstParamList  pl;

    if ((pl = m.find (name)) != m.end ())
    {
      return (*pl->second);
    }
    else
    {
      std::cout << "PARMETERSET(*)::find ('" << name << "')" << std::endl;

      for (pl=begin ();pl!=end ();pl++)
      {
        std::cout << "\t[" << indexOf (pl->first) << "] " << (void*)pl->second << " '" << pl->first << "'" << std::endl;
      }

      throw std::exception ();
    }
  }
  /**
   * @returns true if there is a parameter value associated with the label.
   */
  const bool operator&& (const Param& name) const
  {
    return ((m.find (name) == m.end ()) == false);
  }
  void set (const ParameterSet<T*>& ps)
  {
    for (auto& pl : ps)
    {
      this->set (pl.first, pl.second);
    }
  }
  /**
   * set a parameter to a new value,
   * the parameter will be created if it does not exist.
   * If it a pointed-to object exists it will be deleted via the remove () function,
   * UNLESS! the pointed-to object has the same memory location as the newValue (via the == operator)
   * Otherwise, the new object will cause deletion of the old object, which is in fact,
   * the new object, and we have a dangling pointer problem.
   * Dangling pointers attack the space between your buttocks.
   * @param name parameter key (label)
   * @param newValue new value of the parameter, will be copied.
   */
  void set (const Param& name, T* newValue)
  {
    ConstParamList pl;
    
    if ((pl = m.find (name)) != m.end ())
    {
      if (pl->second == newValue)
      {
        std::cout << "Duplicate, original preserved" << std::endl;
      }
      else
      {
        remove (name);
      }
    }

    if (newValue != NULL)
    {
      m[name] = newValue;
    }

    setLCV (name);
  }
  void push_back (const Param& name)                {set (name, NULL);}
  /**
   * removes a parameter from the set, deleting the pointed-to object.
   * @param name parameter label of the parameter to delete
   */
  void remove (const Param& name)
  {
    ConstParamList pl;

    if ((pl = m.find (name)) != m.end ())
    {
#ifdef _DEBUG
      std::endl << "DEL " << pl->first << std::endl;
#endif
      // FIXME: why is this here?
      if (Param ("ColourMap\\Black\0") == pl->first)
      {
        std::cout << "CLRM" << std::endl;
      }

      delete pl->second;
    }

    setLCV (name);
    m.erase (name);
  }
  /**
   * WEAK-REMOVE
   * removes a paramter from the set WITHOUT deleting the pointed-to object.
   * This allows objects which are not primarily stored in the ParameterSet
   * to be removed without borking the automatic stack clean-up performed
   * during object destruction.
   * @param name parameter label of the parameter to delete
   */
  void wemove (const Param& name)
  {
    if (((*this) && name) == false)
    {
      std::cout << "Could not remove non-existant param" << std::endl;
      return;
    }

    setLCV (name);
    m.erase (name);
  }
  /**
   * removes all key/value pairs from the parameter set
   * deleteing the pointed-to object (without using remove () function)
   */
  void clear ()
  {
    ConstParamList pl;
  //  Note::notemsg ("CLR %08x", reinterpret_cast<unsigned int>(this));

    while ((pl=begin ()) !=end ())
    {
      remove (pl->first);
      pl=begin ();
    }

    setLCV ("::all\0");
    m.clear ();
    SelectedName::clear ();
  }
  /**
   * @returns true if there are no keys in the parameter set
   */
  const bool empty () const
  {
    return m.empty ();
  }
  /**
   * @returns the number of keys in the parameter set
   */
  const size_t parameterCount () const
  {
    return m.size ();
  }
  /**
   * @returns an iterator at the first key of the parameter set
   */
  ParamList begin ()
  {
    return m.begin ();
  }
  ParamListR rbegin ()
  {
    return m.rbegin ();
  }
  /**
   * @returns an iterator at the last+1 key of the parameter set
   */
  ParamList end ()
  {
    return m.end ();
  }
  ParamListR rend ()
  {
    return m.rend ();
  }
  ConstParamList begin () const
  {
    return m.begin ();
  }
  ConstParamListR rbegin () const
  {
    return m.rbegin ();
  }
  ConstParamList end () const
  {
    return m.end ();
  }
  ConstParamListR rend () const
  {
    return m.rend ();
  }
// FIND
  ParamList find (const T* value)
  {
    ParamList  pl;

    for (pl=begin ();pl!=end()&&pl->second!=value;pl++)
      ;;

    return pl;
  }
  ConstParamList find (const T* value) const
  {
    ConstParamList  pl;

    for (pl=begin ();pl!=end ()&&pl->second!=value;pl++)
      ;;

    return pl;
  }
  ParamList find (const Param& name)
  {
    return m.find (name);
  }
  ConstParamList find (const Param& name) const
  {
    return m.find (name);
  }
  ParamList findHash (const HashType hash)
  {
    ParamList  pl;

    for (pl=begin ();pl!=end ();pl++)
    {
      if (pl->first.hash () == hash)
        break;
    }
    return pl;
  }
  ConstParamList findHash (const HashType hash) const
  {
    ConstParamList pl;

    for (pl=begin ();pl!=end ();pl++)
    {
      if (pl->first.hash () == hash)
        break;
    }
    return pl;
  }
  const size_t indexOf (const Param& name) const
  {
    return std::distance (begin (), find (name));
  }
#if 0
// PERSISTANCE
  virtual const char *persistableNodeID () const          {return persistableTypename<T> ();}
  virtual const char *persistableNodeLabelID () const        {return "name\0";}
  virtual const char *persistableNodeValueID () const        {return "value\0";}
  virtual void write (std::ostream& os) const
  {
    ConstParamList  pl;

  //  Patterns::Persistable::pushObject<ParameterSet<T> > (os);

    for (pl=begin (); pl!=end (); pl++)
    {
    //  Patterns::Persistable::pushParameter<T> (os, pl->first.c_str (), pl->second);

      Patterns::Persistable::pushObject<T> (os, "name\0", pl->first.c_str ());  ///FIXME: persistableNodeLabelID ()?
      os << (*pl->second);
      Patterns::Persistable::popObject<T> (os);
    }

  //  Patterns::Persistable::popObject<ParameterSet<T> > (os);
  }
  virtual void read (std::istream& is)
  {
    Note::notemsg ("read is not implemented for this type of ParameterSet\0");
  }
  friend inline std::ostream& operator<< (std::ostream& os, const ParameterSet<T*>& ps)
  {
    ps.write (os);
    return os;
  }
#endif
};

// basic sets
typedef bool                BoolSetType;
#ifdef _WIN64
//typedef __int64           IntegerSetType;
typedef unsigned int        IntegerSetType;
#else
#ifdef _WIN32_
typedef unsigned int        IntegerSetType;
#else
typedef uintptr_t           size_t;
typedef uintptr_t           IntegerSetType;
#endif
#endif
typedef float                         FloatSetType;
typedef ParameterSet<BoolSetType>     BoolSet;
typedef ParameterSet<FloatSetType>    FloatSet;
typedef ParameterSet<Param>           ParamSet;    // nice name!


#if 0
class IntegerSet : public ParameterSet<IntegerSetType>
{
public:
// OVERLOADS
  virtual void push_back (const Param& name)  {set (name, parameterCount ());}
#if 0
// PERSISTANCE
  const char *persistableNodeID () const    {return "property\0";}
  const char *persistableNodeLabelID () const  {return "name\0";}
  const char *persistableNodeValueID () const  {return "value\0";}
  void read (std::istream& is, const XMLNode& xn)
  {
    std::vector<XMLNode>::const_iterator  it;
    std::string    p, n, nv, v, vv;
    unsigned int  vx;

    for (it=xn.children.begin ();it!=xn.children.end ();it++)
    {
      is.seekg (it->offset ()+1);
      p = Patterns::Persistable::fetchNextToken (is);

      if (p.compare (persistableNodeID ()) != 0)
      {
        Note::notemsg ("Incorrect NodeID '%s'\0", p.c_str ());
        continue;
      }

      Patterns::Persistable::fetchNextAttributeValuePair (is, n, nv);

      if (n.compare (persistableNodeLabelID ()) != 0)
      {
        Note::notemsg ("Incorrect LabelID '%s'\0", n.c_str ());
        continue;
      }

      Patterns::Persistable::fetchNextAttributeValuePair (is, v, vv);

      if (v.compare (persistableNodeValueID ()) != 0)
      {
        Note::notemsg ("Incorrect ValueID '%s'\0", v.c_str ());
        continue;
      }

      std::stringstream (vv) >> vx;

      this->set (nv, vx);
    }
  }
  void write (std::ostream& os) const
  {
    IntegerSet::ConstParamList  pl;

    for (pl=begin (); pl!=end (); pl++)
    {
      Patterns::Persistable::writeXMLBeginAttributeHang<const char *> (os, persistableNodeID ());
      Patterns::Persistable::writeXMLAttributeValuePair (os, persistableNodeLabelID (), pl->first.c_str ());
      Patterns::Persistable::writeXMLAttributeValuePair (os, persistableNodeValueID (), pl->second);
      Patterns::Persistable::writeXMLBeginAttributeUnhangTerm (os);
    }
  }
#endif
};
#else
typedef ParameterSet<IntegerSetType>  IntegerSet;
#endif

#if 0
inline std::ostream& operator<< (std::ostream& os, const IntegerSet& is)
{
  //IntegerSet::ConstParamList  pl;

  //for (pl=is.begin (); pl!=is.end (); pl++)
  //{
  //  Patterns::Persistable::writeXMLBeginAttributeHang<const char *> (os, "property\0");
  //  Patterns::Persistable::writeXMLAttributeValuePair (os, "name\0", pl->first.c_str ());
  //  Patterns::Persistable::writeXMLAttributeValuePair (os, "value\0", pl->second);
  //  Patterns::Persistable::writeXMLBeginAttributeUnhangTerm (os);
  //}

  is.write (os);
  return os;
}
#endif

/**
 * Returns a list of attributes which are in one set but not the other.
 * -1 values indicate the attribute is in A but not B, +1 values indicate B not A.
 */
template<class T>
static void difference (const ParameterSet<T>& a, const ParameterSet<T>& b, IntegerSet dst)
{
  typename ParameterSet<T>::ConstParamList  pl;

  // false values are those in A not B
  for (pl=a.begin (); pl!=a.end (); pl++)
  {
    if (b.find (pl->first) == b.end ())
    {
      continue;
    }

    dst.set (pl->first, -1);
  }

  // true values: b not a
  for (pl=b.begin (); pl!=b.end (); pl++)
  {
    if (a.find (pl->first) == a.end ())
    {
      continue;
    }

    dst.set (pl->first, +1);
  }
}
static void difference (const ParameterList& a, const ParameterList& b, IntegerSet dst)
{

}
template<class T>
static bool contains (const ParameterSet<T>& ps, const Param& p)
{
  return (std::find (ps.begin (), ps.end (), p) != ps.end ());
}
static bool contains (const ParameterList& pl, const Param& p)
{
  return (std::find (pl.begin (), pl.end (), p) != pl.end ());
}
template<class T>
static size_t indexOf (const ParameterSet<T>& ps, const Param& p)
{
#if 0
  ParameterSet<T>::ConstParamList  pl;

  if ((pl == std::find (ps.begin (), ps.end (), p)) == ps.end ())
  {
    return ps.parameterCount ();
  }

  return std::distance (ps.begin (), pl);
#else
  return std::distance (ps.begin (), std::find (ps.begin (), ps.end (), p));
#endif
}
static size_t indexOf (const ParameterList& pl, const Param& p)
{
  return std::distance (pl.begin (), std::find (pl.begin (), pl.end (), p));
}
#endif
