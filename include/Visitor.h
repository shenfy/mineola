#ifndef MINEOLA_VISITOR_H
#define MINEOLA_VISITOR_H

namespace mineola {

// templated visitor pattern
class Traverser {
public:
  Traverser() {}
  virtual ~Traverser() {}
};

template <typename T, typename TR = void>
class Visitor {
public:
  virtual TR Visit(T &) = 0;
};

template <typename TR = void>
class Visitable {
public:
  typedef TR visitor_return_type;
  Visitable() {}
  virtual ~Visitable() {}
  virtual TR Accept(Traverser &visitor) = 0;

protected:
  template <typename T>
  static TR AcceptImpl(T &me, Traverser &visitor)
  { 
    if (Visitor<T, TR> *p = dynamic_cast<Visitor<T, TR>*>(&visitor))
      return p->Visit(me);
    return visitor_return_type();
  }
};

#ifndef MINEOLA_VISITOR_ACCEPT_FUNC
#define MINEOLA_VISITOR_ACCEPT_FUNC\
  virtual visitor_return_type Accept(Traverser &visitor) {return AcceptImpl(*this, visitor);}
#endif

} //namespace

#endif
