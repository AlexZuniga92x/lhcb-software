#ifndef HLT2MONITORING_CHUNK_H
#define HLT2MONITORING_CHUNK_H 1

#include <ostream>

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "Common.h"

namespace Monitoring {

/** A serialize-able chunk of data that spans DELTA_T seconds.
 */
struct Chunk {
   Chunk() = default;

   Chunk(RunNumber rn, TCK t, HistId id)
      : runNumber{rn}, tck{t}, histId{id} {}

   // Copy constructor
   Chunk(const Chunk& other)
      : runNumber{other.runNumber},
        tck{other.tck},
        histId{other.histId},
        data{other.data}
   {
   }

   // Move constructor
   Chunk(Chunk&& other)
      : runNumber{0},
        tck{0},
        histId{0}
   {
      std::swap(runNumber, other.runNumber);
      std::swap(tck, other.tck);
      std::swap(histId, other.histId);
      std::swap(data, other.data);
   }

   // Assignment operator
   Chunk& operator=(const Chunk& other) {
      if (&other == this) {
         return *this;
      }
      runNumber = other.runNumber;
      tck = other.tck;
      histId = other.histId;
      data = other.data;
      return *this;
   }

   // Move assignment operator
   Chunk& operator=(Chunk&& other) {
      if (&other == this) {
         return *this;
      }
      std::swap(runNumber, other.runNumber);
      std::swap(tck, other.tck);
      std::swap(histId, other.histId);
      std::swap(data, other.data);
      return *this;
   }

   // Find highest bin
   unsigned int highestBin() const {
      auto it = std::max_element(begin(data), end(data),
                                 [] (const decltype(data)::value_type& first,
                                     const decltype(data)::value_type& second) {
                                    return first.first < second.first;
                                 });
      return it == end(data) ? 0 : it->first;
   }

   RunNumber runNumber;
   TCK tck;
   HistId histId;
   std::unordered_map<unsigned int, BinContent> data;
};
}

#endif // HLT2MONITORING_CHUNK_H
