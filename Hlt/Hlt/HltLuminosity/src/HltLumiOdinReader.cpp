// $Id: HltLumiOdinReader.cpp,v 1.4 2008-09-24 18:52:18 graven Exp $
// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
// from LHCb
#include "Event/ODIN.h"
#include "Event/L0DUReport.h"
// local
#include "HltLumiOdinReader.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : HltLumiOdinReader
//
// 2008-07-15 : Jaap Panman
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltLumiOdinReader )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltLumiOdinReader::HltLumiOdinReader( const std::string& name,
                                      ISvcLocator* pSvcLocator )
    : HltAlgorithm( name, pSvcLocator ), m_selection( *this )
{
    //  declareProperty("L0DULocation", m_l0Location = L0DUReportLocation::Default );
    m_selection.declareProperties();
}
//=============================================================================
// Initialization
//=============================================================================
StatusCode HltLumiOdinReader::initialize()
{
    if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;
    StatusCode sc = HltAlgorithm::initialize(); // must be executed first
    if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

    // default configuration
    m_selection.registerSelection();

    return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HltLumiOdinReader::execute()
{
    // get ODIN
    LHCb::ODIN* odin = getIfExists<LHCb::ODIN>( LHCb::ODINLocation::Default );
    ;
    if ( !odin ) {
        return Error( "ODIN cannot be loaded", StatusCode::FAILURE );
    }

    if ( msgLevel( MSG::DEBUG ) ) {
        std::stringstream bxType( "" );
        bxType << (LHCb::ODIN::BXTypes)odin->bunchCrossingType();
        std::stringstream trType( "" );
        trType << (LHCb::ODIN::TriggerType)odin->triggerType();
        debug() << " Trigger Type : " << trType.str() << " BXType : " << bxType.str()
                << endmsg;
    }
    m_selection.output()->setDecision( true );

    return StatusCode::SUCCESS;
}

