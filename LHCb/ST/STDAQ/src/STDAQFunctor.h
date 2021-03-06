#ifndef _ITDataFunctor_H_
#define _ITDataFunctor_H_

#include <numeric>
#include "Kernel/STTell1ID.h"

namespace STDAQFunctor{

template<class TYPE>
struct Less_by_Tell1Board {

    /** compare the channel of one object with the
     *  channel of another object
     *  @param obj1   first  object
     *  @param obj2   second object
     *  @return  result of the comparision
     */
    bool operator() ( TYPE obj1 , TYPE obj2 ) const {
      return
        ( !obj1 ) ? true  :
        ( !obj2 ) ? false : obj1->boardID() < obj2->boardID() ;
    }
};

template <class TYPE>
class Tell1Board_eq {
   STTell1ID aBoard;
public:
  explicit Tell1Board_eq(const STTell1ID& testBoard) : aBoard(testBoard){}
  inline bool operator() (TYPE obj) const{ return obj->boardID()==aBoard; }
};

template <class TYPE>
struct compByTell1Board_LB {
  bool operator() (const TYPE& obj, const STTell1ID& testID) const{
    return ((!obj) ? false : testID >obj->boardID());
  }
};

template <class TYPE>
struct compByTell1Board_UB {
  bool operator() (const STTell1ID& testID, const TYPE& obj) const{
    return ((!obj) ? false : testID >obj->boardID());
  }
};

}
#endif // STDAQFunctor
