// $Id$
// ============================================================================
#ifndef LOKI_VOIDPRIMITIVES_H 
#define LOKI_VOIDPRIMITIVES_H 1
// ============================================================================
// Include files
// ============================================================================
/** @file 
 *  The template specialization of basic primitives for "void" arguments 
 *  @author Vanya BELYAEV Ivan.BElyaev@nikhef.nl
 *  @date 2008-10-17
 */
// ============================================================================
namespace LoKi
{
  // ==========================================================================
  template <class TYPE2>
  class TwoFunctors<void,TYPE2>
  {
  private:
    // ========================================================================
    /// the actual type of underlying functor 
    typedef LoKi::Functor<void,TYPE2> functor ;
    // ========================================================================
  public:
    // ========================================================================
    TwoFunctors ( const functor& f1 , 
                  const functor& f2 ) 
      : m_fun1  ( f1 ) 
      , m_fun2  ( f2 ) 
    {}
    /// destructor 
    virtual ~TwoFunctors() {}
    // ========================================================================
  public:
    // ========================================================================
    /// evaluate the first functor 
    typename functor::result_type fun1 () const { return m_fun1.fun ( /* a */ ) ; }
    /// evaluate the first functor 
    typename functor::result_type fun2 () const { return m_fun2.fun ( /* a */ ) ; }
    // ========================================================================
  public:
    // ========================================================================
    /// get the first functor 
    const functor& func1 ()           const { return m_fun1.func () ; }
    /// get the second functor 
    const functor& func2 ()           const { return m_fun2.func () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// no default constructor 
    TwoFunctors () ;                                  // no default constructor 
    // ========================================================================
  private:
    // ========================================================================
    /// the first functor 
    LoKi::FunctorFromFunctor<void,TYPE2> m_fun1 ;          // the first functor 
    /// the second functor 
    LoKi::FunctorFromFunctor<void,TYPE2> m_fun2 ;         // the second functor 
    // ========================================================================
  } ;
  // ==========================================================================  
  template<class TYPE2> 
  class And<void,TYPE2> : public LoKi::Functor<void,bool> 
  {
  private:
    // ========================================================================
    typedef void TYPE  ;
    /// the actual type of functor
    typedef LoKi::Functor<TYPE,TYPE2> functor ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    And ( const functor& f1 , 
          const functor& f2 ) 
      : LoKi::Functor<TYPE,bool>()
      , m_two ( f1 , f2 ) 
    {}
    /// virtual constructor
    virtual ~And(){}
    // ========================================================================
  public:
    // ========================================================================
    /// MANDATORY: clone method ("")
    virtual And* clone() const { return new And(*this) ; }
    /// MANDATORY: the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { return this->fun1 ( /* a */ ) ? this->fun2 ( /* a */ )  : false ; }  
    /// OPTIONAL: the nice printout 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "&" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default contructor is disabled 
    And () ; // the default contructor is disabled 
    // ========================================================================
  protected:
    // ========================================================================
    /// evaluate the first functor 
    typename functor::result_type fun1 () const { return m_two.fun1 ( /* a */ ) ; }
    /// evaluate the first functor 
    typename functor::result_type fun2 () const { return m_two.fun2 ( /* a */ ) ; }
    // ========================================================================
    /// get the first functor 
    const functor& func1 ()           const { return m_two.func1 () ; }
    /// get the second functor 
    const functor& func2 ()           const { return m_two.func2 () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the storage of two functors 
    LoKi::TwoFunctors<TYPE,TYPE2> m_two ;        // the storage of two fucntors 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Or<void,TYPE2> : public LoKi::And<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;    
    /// the actual type of functor
    typedef LoKi::Functor<TYPE,TYPE2> functor ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Or ( const functor& f1 , 
         const functor& f2 ) 
      : LoKi::And<void,TYPE2> ( f1 , f2 ) 
    {}
    /// virtual constructor
    virtual ~Or(){}
    // ========================================================================
  public:
    // ========================================================================
    /// MANDATORY: clone method ("")
    virtual Or* clone() const { return new Or(*this) ; }
    /// MANDATORY: the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { return this->fun1 ( /* a */ ) ? true : this->fun2 ( /* a */ ) ; }  
    /// OPTIONAL: the nice printout 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "|" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default contructor is disabled 
    Or() ; // the default contructor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Not<void,TYPE2> : public LoKi::Functor<void,bool>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from the functor  
    Not ( const Functor<TYPE,TYPE2>& fun ) 
      : Functor<TYPE,bool>() 
      , m_fun ( fun ) 
    {} 
    /// virtual destructor 
    virtual ~Not() {}
    /// clone method (mandatory)
    virtual  Not* clone() const { return new Not( *this ); }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument a */ ) const 
    { 
      std::logical_not<TYPE2> lnot ;
      return lnot ( m_fun.fun ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << "(~"  << m_fun << ")" ; };
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    Not() ;                              // the default constrictor is disabled 
    // ========================================================================
  private:
    // ========================================================================
    /// the functor to be negated 
    LoKi::FunctorFromFunctor<TYPE,TYPE2> m_fun ; // the functor to be negated 
    // ========================================================================
  };  
  // ==========================================================================
  template<class TYPE2> 
  class Negate<void,TYPE2> : public LoKi::Functor<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from the functor  
    Negate ( const LoKi::Functor<TYPE,TYPE2>& fun ) 
      : LoKi::Functor<TYPE,TYPE2>() 
      , m_fun ( fun ) 
    {} 
    /// virtual destructor 
    virtual ~Negate() {}
    /// clone method (mandatory)
    virtual  Negate* clone() const { return new Negate ( *this ) ; }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument  a */ ) const 
    { 
      std::negate<TYPE2> negator ;
      return negator ( m_fun.fun ( /* a */ ) ) ;
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << "(-"  << m_fun << ")" ; };
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    Negate() ; // the default constrictor is disabled 
    // ========================================================================
  private:
    // ========================================================================
    /// the functor to be negated 
    LoKi::FunctorFromFunctor<TYPE,TYPE2> m_fun ; // the functor to be negated 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Less<void,TYPE2> : public LoKi::Functor<void,bool> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Less ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
           const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Functor<TYPE,bool>()
      , m_two ( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Less ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Functor<TYPE,bool>()
      , m_two ( f1 , LoKi::Constant<TYPE,TYPE2>( f2 )  ) 
    {}
    /// constructor from the functor and constant 
    Less ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Functor<TYPE,bool>()
      , m_two ( LoKi::Constant<TYPE,TYPE2>( f1 ) , f2  ) 
    {}
    /// virtual destructor 
    virtual ~Less() {}
    /// clone method (mandatory)
    virtual  Less* clone() const { return new Less( *this ); }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument  a */ ) const 
    { 
      std::less<TYPE2> _less ;
      return _less ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "<" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constrictor is disabled
    Less() ; // the default constrictor is disabled 
    // ========================================================================
  protected:
    // ========================================================================
    typedef LoKi::Functor<TYPE,TYPE2>                                 functor ;
    /// evaluate the first functor 
    typename functor::result_type fun1 () const { return m_two.fun1 ( /* a */ ) ; }
    /// evaluate the first functor 
    typename functor::result_type fun2 () const { return m_two.fun2 ( /* a */ ) ; }
    // ========================================================================
    /// get the first functor 
    const functor& func1 ()           const { return m_two.func1 () ; }
    /// get the second functor 
    const functor& func2 ()           const { return m_two.func2 () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the storage of two functors 
    LoKi::TwoFunctors<TYPE,TYPE2> m_two ; // the storage of two functors 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Equal<void,TYPE2> : public LoKi::Less<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Equal ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
            const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Less<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from two functors 
    Equal ( T2                               f1 , 
            const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Less<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from two functors 
    Equal ( const LoKi::Functor<TYPE,TYPE2>& f1 ,
            T2                               f2 )
      : LoKi::Less<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// virtual destructor 
    virtual ~Equal() {}
    /// clone method (mandatory)
    virtual  Equal* clone() const { return new Equal ( *this ) ; }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { return equal ( /* a */ ) ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "==" << this->func2() << ") " ; }
    // ========================================================================
  public:
    // ========================================================================
    /// the actual comparison:
    inline result_type equal ( /* argument a */ ) const 
    { 
      LHCb::Math::Equal_To<TYPE2> _cmp ;                      // the comparison    
      return _cmp ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    // ========================================================================
    /// the actual comparison:
    inline result_type not_equal ( /* argument a */ ) const 
    {return ! this->equal( /* argument a*/ ) ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constrictor is disabled
    Equal() ;                            // the default constrictor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class LessOrEqual<void,TYPE2> : public LoKi::Equal<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from twho functors 
    LessOrEqual ( const Functor<TYPE,TYPE2>& f1 , 
                  const Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    LessOrEqual ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 )
    {}
    /// constructor from the functor and constant 
    LessOrEqual ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 )
    {}
    /// virtual destructor 
    virtual ~LessOrEqual() {}
    /// clone method (mandatory)
    virtual  LessOrEqual* clone() const { return new LessOrEqual ( *this ) ; }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { 
      std::less_equal<TYPE2> _cmp ;
      return _cmp ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " ( " << this->func1() << "<=" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    LessOrEqual() ; // the default constrictor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class NotEqual<void,TYPE2> : public LoKi::Equal<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from twho functors 
    NotEqual ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
               const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from twho functors 
    NotEqual ( T2                               f1 , 
               const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from twho functors 
    NotEqual ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
               T2                               f2 ) 
      : LoKi::Equal<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// virtual destructor 
    virtual ~NotEqual() {}
    /// clone method (mandatory)
    virtual  NotEqual* clone() const { return new NotEqual ( *this ) ; }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument  a */ ) const 
    { return this->not_equal ( /* a */ )  ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() 
               << "!=" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    NotEqual() ; // the default constrictor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<> 
  class Plus<void,double> : public LoKi::Functor<void,double> 
  {
  private:
    // ========================================================================
    typedef void   TYPE ;
    typedef double TYPE2 ;
    /// the constant type 
    typedef LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Plus ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
           const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Plus ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( f1 , LoKi::Constant<TYPE,TYPE2> ( f2 )  ) 
    {}
    /// constructor from the functor and constant 
    Plus ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( LoKi::Constant<TYPE,TYPE2> ( f1 ) , f2 ) 
    {}
    /// virtual destructor 
    virtual ~Plus() {}
    /// clone method (mandatory)
    virtual  Plus* clone() const { return new Plus ( *this ) ; }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument a */ ) const 
    { 
      std::plus<TYPE2> _plus ;
      return _plus ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "+" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    Plus() ;                             // the default constructor is disabled 
    // ========================================================================
  protected:
    // ========================================================================
    typedef LoKi::Functor<TYPE,TYPE2>                                 functor ;
    /// evaluate the first functor 
    functor::result_type fun1  () const { return m_two.fun1 ( /* a */ ) ; }
    /// evaluate the first functor 
    functor::result_type fun2  () const { return m_two.fun2 ( /* a */ ) ; }
    // ========================================================================
    /// get the first functor 
    const functor&       func1 ()           const { return m_two.func1 () ; }
    /// get the second functor 
    const functor&       func2 ()           const { return m_two.func2 () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the storage of two functors 
    LoKi::TwoFunctors<TYPE,TYPE2> m_two ; // the storage of two functors 
    // ========================================================================
  };
  // ==========================================================================
  template<> 
  class Minus<void,double> : public LoKi::Plus<void,double> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    typedef double TYPE2 ;
    /// the constant type 
    typedef LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Minus ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
            const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Plus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Minus ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Plus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Minus ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Plus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// virtual destructor 
    virtual ~Minus() {}
    /// clone method (mandatory)
    virtual  Minus* clone() const { return new Minus ( *this ) ; }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument  a */ ) const 
    { 
      std::minus<TYPE2> _minus;
      return _minus ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "-" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constrictor is disabled
    Minus() ; // the default constrictor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<> 
  class Divide<void,double> : public LoKi::Minus<void,double> 
  { 
  private:
    // ========================================================================
    typedef void   TYPE ;
    typedef double TYPE2 ;
    /// the constant type 
    typedef LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Divide ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
             const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Minus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor adn the constant 
    Divide ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Minus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor adn the constant 
    Divide ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2) 
      : LoKi::Minus<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// virtual destructor 
    virtual ~Divide() {}
    /// clone method (mandatory)
    virtual  Divide* clone() const { return new Divide ( *this ) ; }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument  a */ ) const 
    { 
      std::divides<TYPE2> oper ;
      return oper ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "/" << this->func2() << " )" ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    Divide() ;                          // the default constructor is disabled 
    // ========================================================================
  } ;
  // ==========================================================================
  template<> 
  class Multiply<void,double> : public LoKi::Divide<void,double> 
  {
  private:
    // ========================================================================
    typedef void   TYPE  ;
    typedef double TYPE2 ;
    /// the constant type 
    typedef LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// constructor from two functors 
    Multiply ( const LoKi::Functor<TYPE,TYPE2>& f1 , 
               const LoKi::Functor<TYPE,TYPE2>& f2 ) 
      : LoKi::Divide<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Multiply ( const LoKi::Functor<TYPE,TYPE2>& f1 , T2 f2 ) 
      : LoKi::Divide<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// constructor from the functor and constant 
    Multiply ( T2 f1 , const LoKi::Functor<TYPE,TYPE2>& f2) 
      : LoKi::Divide<TYPE,TYPE2>( f1 , f2 ) 
    {}
    /// virtual destructor 
    virtual ~Multiply() {}
    /// clone method (mandatory)
    virtual  Multiply* clone() const { return new Multiply ( *this ) ; }
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument a */ ) const 
    { 
      std::multiplies<TYPE2> oper ;
      return oper ( this->fun1 ( /* a */ ) , this->fun2 ( /* a */ ) ) ; 
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " (" << this->func1() << "*" << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constrictor is disabled
    Multiply() ; // the default constrictor is disabled 
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Min<void,TYPE2> : public LoKi::Functor<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:    
    // ========================================================================
    /** constructor from 2 functions 
     *  @param fun1 the first  function
     *  @param fun2 the second function
     */
    Min ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( fun1 , fun2 ) 
    {}
    /// constructor from the function and constant 
    Min ( const LoKi::Functor<TYPE,TYPE2>& fun1 , T2 fun2 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( fun1 , LoKi::Constant<TYPE,TYPE2> ( fun2 )  ) 
    {}
    /// constructor from the function and constant 
    Min ( T2 fun1 , const LoKi::Functor<TYPE,TYPE2>& fun2 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( LoKi::Constant<TYPE,TYPE2> ( fun1 ) , fun2 ) 
    {}
    /** constructor from 3 functions 
     *  @param fun1 the first  function
     *  @param fun2 the second function
     *  @param fun3 the third  function
     */
    Min ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ,
          const LoKi::Functor<TYPE,TYPE2>& fun3 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( Min ( fun1 , fun2 ) , fun3 ) 
    {}
    /** constructor from 4 functions 
     *  @param fun1 the first function
     *  @param fun2 the second function
     *  @param fun3 the third function
     *  @param fun4 the fourth function
     */
    Min ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ,
          const LoKi::Functor<TYPE,TYPE2>& fun3 , 
          const LoKi::Functor<TYPE,TYPE2>& fun4 ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_two ( Min ( Min ( fun1 , fun2 ) , fun3 ) , fun4 ) 
    {}
    /// virtual destructor 
    virtual ~Min() {}
    /// clone method (mandatory)
    virtual  Min* clone() const { return new Min ( *this ) ; }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument  a */ ) const 
    { 
      return std::min ( this->fun1( /* a */ ) , 
                        this->fun2( /* a */ ) , std::less<TYPE2>() ) ;
    }    
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " min( " << this->func1() << "," << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    Min () ; // the default constructor is disabled
    // ========================================================================
  protected:
    // ========================================================================
    typedef LoKi::Functor<TYPE,TYPE2>                                 functor ;
    /// evaluate the first functor 
    typename functor::result_type fun1 () const { return m_two.fun1 ( /* a */ ) ; }
    /// evaluate the first functor 
    typename functor::result_type fun2 () const { return m_two.fun2 ( /* a */ ) ; }
    // ========================================================================
    /// get the first functor 
    const functor& func1 ()           const { return m_two.func1 () ; }
    /// get the second functor 
    const functor& func2 ()           const { return m_two.func2 () ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the storage of two functors 
    LoKi::TwoFunctors<TYPE,TYPE2> m_two ; // the storage of two functors
    // ========================================================================
  };
  // ==========================================================================
  template<class TYPE2> 
  class Max<void,TYPE2> : public LoKi::Min<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:    
    // ========================================================================
    /** constructor from 2 functions 
     *  @param fun1 the first  function
     *  @param fun2 the second function
     */
    Max ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ) 
      : LoKi::Min<TYPE,TYPE2>( fun1 , fun2 ) 
    {}
    /// constructor from the function and constant 
    Max ( const LoKi::Functor<TYPE,TYPE2>& fun1 , T2 fun2 ) 
      : LoKi::Min<TYPE,TYPE2>( fun1 , fun2 ) 
    {}
    /// constructor from the function and constant 
    Max ( T2 fun1 , const LoKi::Functor<TYPE,TYPE2>& fun2 ) 
      : LoKi::Min<TYPE,TYPE2>( fun1 , fun2 ) 
    {}
    /** constructor from 3 functions 
     *  @param fun1 the first  function
     *  @param fun2 the second function
     *  @param fun3 the third  function
     */
    Max ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ,
          const LoKi::Functor<TYPE,TYPE2>& fun3 ) 
      : LoKi::Min<TYPE,TYPE2>( Max ( fun1 , fun2 ) , fun3 ) 
    {}
    /** constructor from 4 functions 
     *  @param fun1 the first function
     *  @param fun2 the second function
     *  @param fun3 the third function
     *  @param fun4 the fourth function
     */
    Max ( const LoKi::Functor<TYPE,TYPE2>& fun1 , 
          const LoKi::Functor<TYPE,TYPE2>& fun2 ,
          const LoKi::Functor<TYPE,TYPE2>& fun3 , 
          const LoKi::Functor<TYPE,TYPE2>& fun4 ) 
      : LoKi::Min<TYPE,TYPE2>( Max ( Max( fun1 , fun2 ) , fun3 ) , fun4 ) 
    {}
    /// virtual destructor 
    virtual ~Max() {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  Max* clone() const { return new Max ( *this ) ; }
    /// MANDATORY: the only one essential method ("function")      
    virtual  result_type operator() ( /* argument a */ ) const 
    { 
      return std::max ( this->fun1( /* a */ ) , 
                        this->fun2( /* a */ ) , std::less<TYPE2>() ) ;
    }    
    /// OPTIONAL: the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " max(" << this->func1() << "," << this->func2() << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    Max () ;                            // the default constructor is disabled
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE2>
  class SimpleSwitch<void,TYPE2> : public LoKi::Functor<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the predicate and 2 constants 
     *
     *  The function return a value of the first constant  
     *  if predicate evaluated to "true", otherwise the 
     *  second constant is returned 
     *
     *  @param cut predicate for branch
     *  @param val1 the first constant 
     *  @param val2 the second constant 
     *
     *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
     *  @date 2005-02-11
     */
    SimpleSwitch
    ( const LoKi::Functor<TYPE,bool>& cut  , T2 val1 , T2 val2 )
      : LoKi::Functor<TYPE,TYPE2> (      ) 
      , m_cut  ( cut  ) 
      , m_val1 ( val1 )
      , m_val2 ( val2 )
    {}
    /// copy constructor 
    SimpleSwitch ( const SimpleSwitch& right ) 
      : LoKi::AuxFunBase          ( right )
      , LoKi::Functor<TYPE,TYPE2> ( right ) 
      , m_cut               ( right.m_cut  ) 
      , m_val1              ( right.m_val1 ) 
      , m_val2              ( right.m_val2 ) 
    {}
    /// destructor 
    virtual ~SimpleSwitch() {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  SimpleSwitch* clone() const { return new SimpleSwitch ( *this ) ; }
    /// MANDATORY: the only one essential method:
    virtual result_type operator() ( /* argument a */ ) const 
    { return m_cut ( /* a */ ) ? m_val1 : m_val2 ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " switch("  << m_cut << ","  << m_val1 << "," << m_val2 << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled    
    SimpleSwitch() ; // the default constructor is disabled    
    // ========================================================================
  private:
    // ========================================================================
    /// the condition 
    LoKi::FunctorFromFunctor <TYPE,bool> m_cut  ; // the condiftion 
    /// the first value 
    TYPE2                                m_val1 ; // the first value 
    /// the second value 
    TYPE2                                m_val2 ; // the second value 
    // ========================================================================
  };  
  // ==========================================================================
  template<class TYPE2>
  class Switch<void,TYPE2> : public LoKi::Functor<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// the constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from predicate and 2 functions 
     *
     *  The function return a value of the first function 
     *  if predicate evaluates to "true", otherwise the 
     *  value of second function is returned 
     *
     *  @param cut predicate for branch
     *  @param fun1 the first  fuction
     *  @param fun2 the second function
     *
     *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
     *  @date 2005-02-11
     */
    Switch
    ( const LoKi::Functor<TYPE,bool>&   cut  , 
      const LoKi::Functor<TYPE,TYPE2>&  fun1 , 
      const LoKi::Functor<TYPE,TYPE2>&  fun2 )
      : Functor<TYPE,TYPE2>  (      ) 
      , m_cut  ( cut  ) 
      , m_two  ( fun1 , fun2 )  
    {}    
    /** constructor from predicate ,function and constant
     *
     *  The function return a value of the function 
     *  if predicate evaluates to "true", otherwise the 
     *  value of constant is returned 
     *
     *  @param cut predicate for branch
     *  @param fun1 the function
     *  @param fun2 the constant 
     *
     *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
     *  @date 2005-02-11
     */
    Switch
    ( const LoKi::Functor<TYPE,bool>&   cut  , 
      const LoKi::Functor<TYPE,TYPE2>&  fun1 , T2 fun2 )
      : LoKi::Functor<TYPE,TYPE2>  (      ) 
      , m_cut  ( cut  ) 
      , m_two  ( fun1 , LoKi::Constant<TYPE,TYPE2>( fun2 ) )  
    {}    
    /** constructor from predicate ,function and constant 
     *
     *  The function return a value of the constant  
     *  if predicate evaluates to "true", otherwise the 
     *  value of function is returned 
     *
     *  @param cut predicate for branch
     *  @param fun1 the constant 
     *  @param fun2 the function
     *
     *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
     *  @date 2005-02-11
     */
    Switch
    ( const LoKi::Functor<TYPE,bool>& cut  , 
      T2                              fun1 , 
      const Functor<TYPE,TYPE2>&      fun2 ) 
      : LoKi::Functor<TYPE,TYPE2>  (      ) 
      , m_cut  ( cut  ) 
      , m_two  ( LoKi::Constant<TYPE,TYPE2>( fun1 ) , fun2 )  
    {}
    /** constructor from predicate and 2 constants 
     *
     *  The function return a value of the first constant  
     *  if predicate evaluates to "true", otherwise the 
     *  second constant is returned 
     *
     *  The constructed fuction object has the same 
     *  functionality as SimpleSwitch, but a bit 
     *  less performant. 
     *  @see SimpleSwitch 
     *  
     *  @param cut predicate for branch
     *  @param fun1 the first constant 
     *  @param fun2 the second constant 
     *
     *
     *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
     *  @date 2005-02-11
     */
    Switch
    ( const LoKi::Functor<TYPE,bool>&   cut  , T2 fun1 , T2 fun2 ) 
      : LoKi::Functor<TYPE,TYPE2>  (      ) 
      , m_cut  ( cut  ) 
      , m_two  ( LoKi::Constant<TYPE,TYPE2>( fun1 ) ,
                 LoKi::Constant<TYPE,TYPE2>( fun2 ) )
    {}
    /// copy constructor 
    Switch ( const Switch& right ) 
      : LoKi::AuxFunBase          ( right ) 
      , LoKi::Functor<TYPE,TYPE2> ( right ) 
      , m_cut               ( right.m_cut ) 
      , m_two               ( right.m_two ) 
    {}
    /// destructor 
    virtual ~Switch() {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  Switch* clone() const { return new Switch ( *this ) ; }
    /// MANDATORY: the only one essential method:
    virtual  result_type operator() ( /* argument a */ ) const 
    { return m_cut.fun  ( /* a */ ) ? 
        m_two.fun1 ( /* a */ ) : m_two.fun2 ( /* a */ ) ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " switch("     
               << m_cut          << "," 
               << m_two.func1()  << "," 
               << m_two.func2()  << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default contructor is disabled 
    Switch () ; // the defautl contructor is disabled 
    // ========================================================================
  private:
    // ========================================================================
    /// the condition 
    LoKi::FunctorFromFunctor<TYPE,bool> m_cut  ;              // the condition 
    /// the actual storage of two functors 
    LoKi::TwoFunctors<TYPE,TYPE2>       m_two ;      // the storage of functors 
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE2>
  class ComposeFunction<void,TYPE2> : public LoKi::Functor<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    // ========================================================================
  protected:
    // ========================================================================
    /// the actual type for the function
    typedef double (*Func) ( double ) ;
    // ========================================================================
  public:
    // ========================================================================
    /// constructor 
    ComposeFunction ( Func                             func           ,  
                      const LoKi::Functor<TYPE,TYPE2>& fun            , 
                      const std::string&               desc = "'fun'" ) 
      : LoKi::Functor<TYPE,TYPE2>()
      , m_fun  ( fun  ) 
      , m_func ( func ) 
      , m_desc ( desc ) 
    {}     
    /// copy contructor
    ComposeFunction ( const ComposeFunction& right ) 
      : LoKi::AuxFunBase          ( right )      
      , LoKi::Functor<TYPE,TYPE2> ( right )
      , m_fun                     ( right.m_fun  ) 
      , m_func                    ( right.m_func ) 
      , m_desc                    ( right.m_desc ) 
    {}
    /// virtual destructor 
    virtual ~ComposeFunction(){}
    /// clone method (mandatory!)
    virtual ComposeFunction*  clone () const 
    { return new ComposeFunction ( *this ) ; }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { return (*m_func) ( m_fun . fun ( /* a */ ) ) ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " " << m_desc << "("  << m_fun << ") " ; };
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    ComposeFunction() ; // the default constructor is disabled
    // ========================================================================
  private:
    // ========================================================================
    /// the functor 
    LoKi::FunctorFromFunctor<TYPE,TYPE2> m_fun  ; // the functor 
    /// the function:
    Func                                 m_func ; // the function:
    /// the function description:
    std::string                          m_desc ; // description 
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE2>
  class ComposeFunction2<void,TYPE2> : public LoKi::Functor<void,TYPE2> 
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,TYPE2>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,TYPE2>::result_type result_type ; 
    /// the actual type of the function 
    typedef double (*Func) ( double , double )      ;
    /// constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    // ========================================================================
  public:
    // ========================================================================
    /// constructor 
    ComposeFunction2 
    ( Func                             func           ,  
      const LoKi::Functor<TYPE,TYPE2>& fun1           , 
      const LoKi::Functor<TYPE,TYPE2>& fun2           , 
      const std::string&               desc = "'fun'" )
      : LoKi::Functor<TYPE,TYPE2>() 
      , m_func ( func ) 
      , m_two  ( fun1 ,fun2 )
      , m_desc ( desc ) 
    {}
    /// constructor 
    ComposeFunction2 
    ( Func                             func            ,  
      const LoKi::Functor<TYPE,TYPE2>& fun1            , 
      T2                               val2            , 
      const std::string&               desc  = "'fun'" )
      : LoKi::Functor<TYPE,TYPE2>() 
      , m_func ( func ) 
      , m_two  ( fun1 , LoKi::Constant<TYPE,TYPE2> ( val2 ) )
      , m_desc ( desc ) 
    {}
    /// constructor 
    ComposeFunction2 
    ( Func                             func            ,  
      T2                               val1            , 
      const LoKi::Functor<TYPE,TYPE2>& fun2            , 
      const std::string&               desc  = "'fun'" )
      : LoKi::Functor<TYPE,TYPE2>() 
      , m_func ( func ) 
      , m_two  ( LoKi::Constant<TYPE,TYPE2> ( val1 ) , fun2 )
      , m_desc ( desc ) 
    {}
    /// constructor 
    ComposeFunction2 
    ( Func                             func            ,  
      T2                               val1            , 
      T2                               val2            , 
      const std::string&               desc  = "'fun'" )
      : LoKi::Functor<TYPE,TYPE2>() 
      , m_func ( func ) 
      , m_two  ( LoKi::Constant<TYPE,TYPE2> ( val1 ) ,
                 LoKi::Constant<TYPE,TYPE2> ( val2 ) )
      , m_desc ( desc ) 
    {}
    /// copy constructor 
    ComposeFunction2 ( const ComposeFunction2& right ) 
      : LoKi::AuxFunBase ( right ) 
      , LoKi::Functor<TYPE,TYPE2>( right ) 
      , m_func ( right.m_func ) 
      , m_two  ( right.m_two  ) 
      , m_desc ( right.m_desc ) 
    {}
    /// virtual destructor 
    virtual ~ComposeFunction2() {}
    /// clone method (mandatory!)
    virtual ComposeFunction2*  clone   () const 
    { return new ComposeFunction2( *this ); }
    /// the only one essential method ("function")      
    virtual result_type operator() ( /* argument a */ ) const 
    { return (*m_func) ( m_two. fun1 ( /* a */ ) , m_two.fun2( /* a */ ) ) ; }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << " " 
               << m_desc         << "("  
               << m_two.func1 () << "," 
               << m_two.func2 () << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the function itself 
    Func m_func        ; // the function itself 
    /// the storage of functors 
    LoKi::TwoFunctors<TYPE,TYPE2> m_two ; // the storage of functors 
    /// the funtion descrition
    std::string m_desc ; // the funtion descrition
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE2>
  class EqualToValue<void,TYPE2> : public LoKi::Functor<void,bool>
  {
  private:
    // ========================================================================
    typedef  void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param val the reference value 
     *  @param eps the relative precision
     */
    EqualToValue 
    ( const LoKi::Functor<TYPE,TYPE2>&  fun , 
      T2                                val )  
      : LoKi::Functor<TYPE,bool>() 
      , m_fun ( fun ) 
      , m_val ( val ) 
    {}
    // ========================================================================
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param val the reference value 
     *  @param eps the relative precision
     */
    EqualToValue 
    ( T2 val , const LoKi::Functor<TYPE,TYPE2>&  fun ) 
      : LoKi::Functor<TYPE,bool>() 
      , m_fun ( fun ) 
      , m_val ( val ) 
    {}
    // ========================================================================
    /// MANDATORY: virtual destructor 
    virtual ~EqualToValue(){} ;
    // ========================================================================
    /// MANDATORY: clone method ("virtual construcor")
    virtual  EqualToValue* clone() const { return new EqualToValue(*this); }
    /// MANDATORY: the only one essential method :
    virtual  result_type operator() ( /* argument a */ ) const
    { return equal_to ( /* a */ ) ; }
    /// OPTIONAL: the specific printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << " (" << this->func() << "==" << this->val() << ") " ; }
    // ========================================================================
  public:
    // ========================================================================
    inline result_type equal_to ( /* argument a */ ) const
    { 
      LHCb::Math::Equal_To<TYPE2>  _cmp ;
      return _cmp ( m_fun.fun ( /* a */ ) , m_val ) ; 
    }    
    // ========================================================================
    inline result_type not_equal_to ( /* argument a */ ) const
    { return !this->equal_to ( /* a */ ) ; }    
    // ========================================================================
  public:
    // ========================================================================
    const LoKi::Functor<TYPE,TYPE2>& func () const { return m_fun.func() ; }
    const TYPE2&                     val  () const { return m_val        ; }
    // ========================================================================
  private:
    // ========================================================================
    /// The default constructor is disabled 
    EqualToValue();
    // ========================================================================
  private:
    // ========================================================================
    /// the functor 
    LoKi::FunctorFromFunctor<TYPE,TYPE2> m_fun ;                 // the functor 
    /// the value 
    TYPE2  m_val ;                                                 // the value 
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE2>
  class NotEqualToValue<void,TYPE2> : public LoKi::EqualToValue<void,TYPE2>
  {
  private:
    // ========================================================================
    typedef void TYPE ;
    /// argument type
    typedef typename LoKi::Functor<TYPE,bool>::argument argument  ; 
    /// result type 
    typedef typename LoKi::Functor<TYPE,bool>::result_type result_type ; 
    // constant type 
    typedef typename LoKi::Constant<TYPE,TYPE2>::T2 T2 ;
    // ========================================================================
  public:
    // ========================================================================
    /** constructor for the function and the value 
     *  @param fun the function
     *  @param val the reference value 
     *  @param eps the relative precision 
     */
    NotEqualToValue
    ( const LoKi::Functor<TYPE,TYPE2>&  fun , T2 val ) 
      : LoKi::EqualToValue<void,TYPE2> ( fun , val ) 
    {}
    /** constructor from the function and the value 
     *  @param val the reference value 
     *  @param fun the function
     *  @param eps the relative precision 
     */
    NotEqualToValue
    ( T2 val , const LoKi::Functor<TYPE,TYPE2>&  fun ) 
      : LoKi::EqualToValue<void,TYPE2> ( val , fun ) 
    {}
    /// MANDATORY: virtual destructor 
    virtual ~NotEqualToValue(){} ;
    /// MANDATORY: clone method ("virtual constructor")
    virtual  NotEqualToValue* clone() const { return new NotEqualToValue(*this); }
    /// MANDATORY: the only one essential method :
    virtual  result_type operator() ( /* argument a */ ) const
    { return this->not_equal_to ( /* a */ ) ; }
    /// OPTIONAL: the specific printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << " (" << this->func () 
               << "!=" << this->val  () << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// The default constructor is disabled 
    NotEqualToValue();
    // ========================================================================
  };
  // ==========================================================================
  template <class TYPE1, class TYPE2, class TYPE3>
  class Compose<void,TYPE1,TYPE2,TYPE3> : public LoKi::Functor<void,TYPE2>
  {
  private:
    // ========================================================================
    /// result type 
    typedef typename LoKi::Functor<void,TYPE2>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /// contructor
    Compose
    ( const LoKi::Functor<void,TYPE1>&  fun1 , 
      const LoKi::Functor<TYPE3,TYPE2>& fun2 )
      : LoKi::Functor<void,TYPE2> () 
      , m_fun1 ( fun1 ) 
      , m_fun2 ( fun2 )
    {}
    /// copy constructor
    Compose ( const Compose& right ) 
      : LoKi::AuxFunBase ( right ) 
      , LoKi::Functor<void,TYPE2> ( right ) 
      , m_fun1 ( right.m_fun1 ) 
      , m_fun2 ( right.m_fun2 )
    {}
    /// MANDATORY: virtual destructor
    virtual ~Compose() {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  Compose* clone() const { return new Compose ( *this ) ; }    
    /// the only one essential method ("function")      
    virtual  result_type operator() ( /* argument */ ) const 
    { 
      const LoKi::Apply<TYPE3,TYPE2> f2 ( &m_fun2.func() ) ;
      return f2.eval ( m_fun1.fun() ) ;
    }
    /// the basic printout method 
    virtual std::ostream& fillStream( std::ostream& s ) const 
    { return s << "(" << m_fun1 << ">>" << m_fun2  << ")" ; }
    // ========================================================================
  public:
    // ========================================================================
    /// the first functor 
    LoKi::FunctorFromFunctor<void,TYPE1>  m_fun1 ;         // the first functor 
    /// the second functor 
    LoKi::FunctorFromFunctor<TYPE3,TYPE2> m_fun2 ;        // the second functor 
    // ========================================================================
  } ;
  // ==========================================================================
  template<> 
  class InRange<void>: public LoKi::Functor<void,bool> 
  {
  private:
    // ========================================================================
    /// result type 
    typedef LoKi::Functor<void,bool>::result_type                 result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the functor and edges 
     *  @param fun the functor 
     *  @param low the low edge  
     *  @param high the high edge 
     */
    InRange
    ( const double                      low  , 
      const LoKi::Functor<void,double>& fun  , 
      const double                      high )
      : LoKi::Functor<void,bool> () 
      , m_low  ( low  ) 
      , m_fun  ( fun  ) 
      , m_high ( high ) 
    {}
    /// MANDATORY: virtual destructor 
    virtual ~InRange() {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  InRange* clone() const { return new InRange ( *this ) ; }
    /// MANDATORY: the only one essential method 
    virtual result_type operator() () const 
    {
      const double r = m_fun.fun() ;
      return m_low <= r && r <= m_high ;
    }
    /// OPTIONAL: the nice printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    {
      return s << " in_range( " << m_low
               << ","           << m_fun 
               << ","           << m_high << " )" ;
      
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the default contructor is disabled 
    InRange() ;                           // the default contructor is disabled 
    // ========================================================================
  private:
    // ========================================================================
    /// the low edge 
    double                                m_low  ;        //       the low edge 
    /// the functor itself 
    LoKi::FunctorFromFunctor<void,double> m_fun  ;        // the functor itself
    /// the high edge 
    double                                m_high ;        //       the low edge 
    // ========================================================================
  } ;
  // ==========================================================================
  template<> 
  class InRange2<void>: public LoKi::Functor<void,bool> 
  {
  private:
    // ========================================================================
    /// result type 
    typedef LoKi::Functor<void,bool>::result_type                 result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the functor and edges 
     *  @param fun the functor 
     *  @param low the low edge  
     *  @param high the high edge 
     */
    InRange2
    ( const LoKi::Functor<void,double>& low  , 
      const LoKi::Functor<void,double>& fun  , 
      const LoKi::Functor<void,double>& high )
      : LoKi::Functor<void,bool> () 
      , m_low  ( low  ) 
      , m_fun  ( fun  ) 
      , m_high ( high ) 
    {}
    /** constructor from the functor and edges 
     *  @param fun the functor 
     *  @param low the low edge  
     *  @param high the high edge 
     */
    InRange2
    ( const double                      low  , 
      const LoKi::Functor<void,double>& fun  , 
      const LoKi::Functor<void,double>& high )
      : LoKi::Functor<void,bool> () 
      , m_low  ( LoKi::Constant<void,double> ( low ) ) 
      , m_fun  ( fun  ) 
      , m_high ( high ) 
    {}
    /** constructor from the functor and edges 
     *  @param fun the functor 
     *  @param low the low edge  
     *  @param high the high edge 
     */
    InRange2
    ( const LoKi::Functor<void,double>& low  ,
      const LoKi::Functor<void,double>& fun  , 
      const double                      high ) 
      : LoKi::Functor<void,bool> () 
      , m_low  ( low )
      , m_fun  ( fun  ) 
      , m_high ( LoKi::Constant<void,double> ( high ) ) 
    {}
    /// MANDATORY: virtual destructor 
    virtual ~InRange2 () {}
    /// MANDATORY: clone method ("virtual constructor")
    virtual  InRange2* clone() const { return new InRange2 ( *this ) ; }
    /// MANDATORY: the only one essential method 
    virtual result_type operator() () const 
    {
      const double low  = m_low .fun () ;
      const double res  = m_fun .fun () ;
      return  
        low <= res           ? 
        res <= m_high.fun () : false ;
    }
    /// OPTIONAL: the nice printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    {
      return s << " in_range( " << m_low
               << ","           << m_fun 
               << ","           << m_high << ") " ;
      
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the default contructor is disabled 
    InRange2() ;                          // the default contructor is disabled 
    // ========================================================================
  private:
    // ========================================================================
    /// the low edge 
    LoKi::FunctorFromFunctor<void,double> m_low  ;        //       the low edge 
    /// the functor itself 
    LoKi::FunctorFromFunctor<void,double> m_fun  ;        // the functor itself
    /// the high edge
    LoKi::FunctorFromFunctor<void,double> m_high ;        //       the low edge 
    // ========================================================================
  } ;
  // ==========================================================================
  template <>
  class EqualToList<void> : public LoKi::Functor<void,bool>
  {
  private:
    // ========================================================================
    /// result type 
    typedef LoKi::Functor<void,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    EqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<double>&         vct ) 
      : LoKi::Functor<void,bool>() 
      , m_fun ( fun ) 
      , m_vct ( vct ) 
    {
      adjust () ;
    }
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    EqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<int>&            vct ) 
      : LoKi::Functor<void,bool>() 
      , m_fun ( fun ) 
      , m_vct ( vct.begin() , vct.end() ) 
    {
      adjust () ;
    }
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    EqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<unsigned int>&            vct ) 
      : LoKi::Functor<void,bool>() 
      , m_fun ( fun ) 
      , m_vct ( vct.begin() , vct.end() ) 
    {
      adjust () ;
    }
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    template <class ITERATOR>
    EqualToList
    ( const LoKi::Functor<void,double>&  fun   , 
      ITERATOR                           first , 
      ITERATOR                           last  ) 
      : LoKi::Functor<void,bool>() 
      , m_fun ( fun ) 
      , m_vct ( first , last ) 
    {
      adjust () ;
    }
    // ========================================================================
    /// MANDATORY: virtual destructor 
    virtual ~EqualToList(){} ;
    // ========================================================================
    /// MANDATORY: clone method ("virtual construcor")
    virtual  EqualToList* clone() const { return new EqualToList(*this); }
    /// MANDATORY: the only one essential method :
    virtual  result_type operator() ( /* argument a */ ) const
    { return equal_to ( /* a */ ) ; }
    /// OPTIONAL: the specific printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << " ( " << this->func() << " == " 
               << Gaudi::Utils::toString ( m_vct ) << " ) " ; }
    // ========================================================================
  public:
    // ========================================================================
    inline result_type equal_to ( /* argument a */ ) const
    {
      if ( m_vct.empty() ) { return  false ; }
      //
      const double r = m_fun.fun () ;
      //
      LHCb::Math::Equal_To<double> cmp ;
      for  ( std::vector<double>::const_iterator item = m_vct.begin() ;
             m_vct.end() != item ; ++item ) 
      { if ( cmp ( *item , r ) ) { return  true ; } } // RETURN 
      //
      return false ;
    }    
    // ========================================================================
    inline result_type not_equal_to ( /* argument a */ ) const
    { return !this->equal_to ( /* a */ ) ; }
    // ========================================================================
  public:
    // ========================================================================
    const LoKi::Functor<void,double>& func () const { return m_fun.func() ; }
    /// get the vector 
    const std::vector<double>&        vect() const { return m_vct ; }
    // ========================================================================
  private:
    // ========================================================================
    size_t  adjust () 
    {
      std::sort ( m_vct.begin() , m_vct.end() ) ;
      std::vector<double>::iterator ilast = 
        std::unique ( m_vct.begin() , m_vct.end() ) ;
      if ( m_vct.end() != ilast ) { m_vct.erase  ( ilast , m_vct.end() ) ; }
      return m_vct.size() ;
    }
    // ========================================================================
  private:
    // ========================================================================
    /// The default constructor is disabled 
    EqualToList ();
    // ========================================================================
  private:
    // ========================================================================
    /// the functor 
    LoKi::FunctorFromFunctor<void,double> m_fun ;                // the functor 
    /// the list 
    std::vector<double>  m_vct ;                                 //    the list 
    // ========================================================================
  };  
  // ==========================================================================
  /** @class NotEqualToList
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-12-06
   */
  template <>
  class NotEqualToList<void> : public LoKi::EqualToList<void>
  {
  private:
    // ========================================================================
    /// result type 
    typedef LoKi::Functor<void,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    NotEqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<double>&         vct ) 
      : LoKi::EqualToList<void>( fun , vct ) 
    {}
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    NotEqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<int>&            vct ) 
      : LoKi::EqualToList<void>( fun , vct ) 
    {}
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    NotEqualToList
    ( const LoKi::Functor<void,double>&  fun , 
      const std::vector<unsigned int>&            vct ) 
      : LoKi::EqualToList<void>( fun , vct ) 
    {}
    /** constructor from the function and the value 
     *  @param fun the function
     *  @param vct the vector of values 
     */
    template <class ITERATOR>
    NotEqualToList
    ( const LoKi::Functor<void,double>&  fun   , 
      ITERATOR                           first , 
      ITERATOR                           last  ) 
      : LoKi::EqualToList<void>( fun , first , last ) 
    {}
    // ========================================================================
    /// MANDATORY: virtual destructor 
    virtual ~NotEqualToList(){} ;
    // ========================================================================
    /// MANDATORY: clone method ("virtual construcor")
    virtual  NotEqualToList* clone() const 
    { return new NotEqualToList(*this); }
    /// MANDATORY: the only one essential method :
    virtual  result_type operator() ( /* argument a */ ) const
    { return not_equal_to ( /* a */ ) ; }
    /// OPTIONAL: the specific printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << " (" << this->func() << "!=" 
               << Gaudi::Utils::toString ( this->vect() ) << ") " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// The default constructor is disabled 
    NotEqualToList ();
    // ========================================================================
  };  
  // ==========================================================================
  template <>
  class XScaler<void> : public LoKi::Functor<void,bool>
  {
  private:
    // ========================================================================
    /// result type 
    typedef LoKi::Functor<void,bool>::result_type result_type ; 
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from predicate and scale
     *  @param cut the predicate 
     *  @param scale the scaler 
     */
    XScaler 
    ( const LoKi::Functor<void,bool>& cut   , 
      const LoKi::Functor<void,bool>& scale ) 
      : LoKi::Functor<void,bool>()
      , m_cut    ( cut   ) 
      , m_scaler ( scale ) 
    {}
    /// MANDATORY: virtual destructor 
    virtual ~XScaler() {}
    /// MANDATORY: clone method ("virtual constructor") 
    virtual  XScaler* clone() const { return new XScaler ( *this ) ; }
    /// MANDATORY: the only one essential method 
    virtual result_type operator() ( /* argument a */ ) const 
    {
      return m_cut.fun ( /* a */ ) && m_scaler.fun( /* void */ ) ;
    }
    /// OPTIONAL: nice printout 
    virtual std::ostream& fillStream ( std::ostream& s ) const 
    { return s << " scale ( " << m_cut << " , " << m_scaler << " ) " ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    XScaler() ;                          // the default constructor is disabled    
    // ========================================================================
  private:
    // ========================================================================
    /// the predicate 
    LoKi::FunctorFromFunctor<void,bool>  m_cut    ;            // the predicate 
    /// the scaler 
    LoKi::FunctorFromFunctor<void,bool>  m_scaler ;            // the scaler 
    // ========================================================================
  };
  // ==========================================================================
} //                                                      end of namespace LoKi
// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_VOIDPRIMITIVES_H
// ============================================================================
