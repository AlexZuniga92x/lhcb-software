// $Id: P2MCPFunctors.h,v 1.1 2009-03-13 18:06:30 jpalac Exp $
#ifndef P2MCP_P2MCPFUNCTORS_H 
#define P2MCP_P2MCPFUNCTORS_H 1

// Include files
#include "P2MCP/P2MCPTypes.h"
#include "LoKi/BuildMCTrees.h"
#include "LoKi/select.h"
/** @namespace P2MCP::Functors P2MCP/P2MCPFunctors.h
 *  
 *
 *  @author Juan Palacios
 *  @date   2009-03-13
 */
namespace P2MCP{
  namespace Functors {
    
    struct SortInTrees : public std::binary_function<const LHCb::MCParticle*,const LHCb::MCParticle* , bool>
    {
    
      bool operator() ( const LHCb::MCParticle* p1 , 
                        const LHCb::MCParticle* p2 ) const
      {
        LoKi::MCParticles::FromMCDecayTree fromDecay( p1 ) ;
        return fromDecay ( p2 );
      }
    };


    struct InTree : public std::unary_function<const LHCb::MCParticle*, bool>
    {

      InTree(const LHCb::MCParticle* head) : m_head(head) { }
    
      bool operator() (const LHCb::MCParticle* mc) const 
      {
        P2MCP::Functors::SortInTrees sorter;
        return sorter(m_head, mc);
      }
    private:
      InTree() { }
    
    private:
      const LHCb::MCParticle* m_head;
    
    };

    template <class Selector, class Sorter>
     struct SortIntoTrees
     {

       template <class InputContainer>
       P2MCP::Types::FlatTrees operator() (const InputContainer& mcps) const
       {
        
         LHCb::MCParticle::ConstVector heads = LoKi::MCTrees::buildTrees(mcps);

         P2MCP::Types::FlatTrees output;

         LHCb::MCParticle::ConstVector::iterator itHead = heads.begin();

         for ( ; itHead != heads.end() ; ++itHead) {
           Selector selector(*itHead);
           P2MCP::Types::FlatTree tree;
           LoKi::select(mcps.begin(), 
                        mcps.end(), 
                        std::back_inserter(tree), 
                        selector);
           std::stable_sort( tree.begin() , 
                             tree.end() , 
                             Sorter() ) ;
           output.push_back(tree);
         }
         return output;
       }
     };

  }
  
}
#endif // P2MCP_P2MCPFUNCTORS_H
