//-----------------------------------------------------------------------------
/** @file RichStereoFitter.cpp
 *
 *  Implementation file for tool : RichStereoFitter
 *
 *  CVS Log :-
 *  $Id: RichStereoFitter.cpp,v 1.1.1.1 2007-09-04 15:59:14 jonrob Exp $
 *
 *  @author Luigi Delbuono   delbuono@in2p3.fr
 *  @date   27/06/2007
 */
//-----------------------------------------------------------------------------

// local
#include "RichStereoFitter.h"

// Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/PhysicalConstants.h"

// namespaces
using namespace LHCb;
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( StereoFitter );

// Standard constructor
StereoFitter::StereoFitter ( const std::string& type,
                             const std::string& name,
                             const IInterface* parent )
  : Rich::Rec::ToolBase ( type, name, parent )
{
  // interface
  declareInterface<IStereoFitter>(this);
}

StatusCode StereoFitter::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = Rich::Rec::ToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  //----------set up some geometric parameters (also done in option file)
  //geometrical data: Z coordinate of entrance and exit windows (cm)
  m_zinWindowRich.push_back( 0 ); // Aerogel
  m_zinWindowRich.push_back( 990 ); // C4F10
  m_zinWindowRich.push_back( 9450 ); // CF4
  declareProperty( "ZinWindowRich",  m_zinWindowRich);
  m_zoutWindowRich.push_back( 0 );       // Aerogel
  m_zoutWindowRich.push_back( 2165 );   // C4F10
  m_zoutWindowRich.push_back( 11900 );   // CF4
  declareProperty( "ZoutWindowRich",  m_zoutWindowRich);

  //geometrical data: windows thickness
  m_thickInterface.push_back( 0 );  // Aerogel
  m_thickInterface.push_back( 5.2e-02 ); // C4F10
  m_thickInterface.push_back( 1.0e-02 ); // CF4
  declareProperty( "ThicknessInterface", m_thickInterface );

  //geometrical data: radiator thickness
  m_thickRadiator.push_back( 3.30e-02 );  // Aerogel
  m_thickRadiator.push_back( 2.60e-02 ); // C4F10
  m_thickRadiator.push_back( 2.04e-02 ); // CF4
  declareProperty( "ThicknessRadiator", m_thickRadiator );

  //geometrical data: mean index of radiators
  m_meanIndexRad.push_back( 1.033 );  // Aerogel
  m_meanIndexRad.push_back( 1.00136 );  // C4F10
  m_meanIndexRad.push_back( 1.000416 ); // CF4
  declareProperty( "MeanIndexRad", m_meanIndexRad );

  //geometrical data: RMS index of radiators
  m_rmsIndexRad.push_back( 488.7e-6 );     // Aerogel
  m_rmsIndexRad.push_back( 39.30e-06 );  // C4F10
  m_rmsIndexRad.push_back( 12.31e-06 );   // CF4
  declareProperty( "RmsIndexRad", m_rmsIndexRad );

  //geometrical data: X curvature in radiators
  m_curvX.push_back( 0 );  // Aerogel
  m_curvX.push_back( 5.770e-03 ); // C4F10
  m_curvX.push_back( 4.702e-03 ); // CF4
  declareProperty( "CurvXRadiator", m_curvX );

  //geometrical data: Y curvature in radiators
  m_curvY.push_back( 0 );  // Aerogel
  m_curvY.push_back( 0.154e-03 ); // C4F10
  m_curvY.push_back( 0.272e-03 ); // CF4
  declareProperty( "CurvYRadiator", m_curvY );

  //Global photon dispersion renormalization factors
  m_globGammmaNorm.push_back( 0 );   // Aerogel
  m_globGammmaNorm.push_back( 1. );       // C4F10
  m_globGammmaNorm.push_back( 1. );       // CF4
  declareProperty( "GlobGammmaNorm", m_globGammmaNorm);

  //Asymptotic resolutions: starting values are from chris external fit
  //Asymptotic resolutions: Forward tracks
  m_asymptoticResFwrd.push_back( 0 );   // Aerogel
  m_asymptoticResFwrd.push_back( 1.376e-03 ); // C4F10
  m_asymptoticResFwrd.push_back( 0.450e-03 ); // CF4
  declareProperty( "AsymptoticResFwrd", m_asymptoticResFwrd );

  //Asymptotic resolutions: Match tracks
  m_asymptoticResMatch.push_back( 0 );       // Aerogel
  m_asymptoticResMatch.push_back( 1.376e-03 ); // C4F10
  m_asymptoticResMatch.push_back( 0.600e-03 ); // CF4
  declareProperty( "AsymptoticResMatch", m_asymptoticResMatch );

  //Asymptotic resolutions: PreDet tracks (i.e: Seed or veloTT)
  m_asymptoticResPreDet.push_back( 0 );        // Aerogel
  m_asymptoticResPreDet.push_back( 1.225e-03 );   // C4F10
  m_asymptoticResPreDet.push_back( 0.710e-03  );   // CF4
  declareProperty( "AsymptoticResPreDet", m_asymptoticResPreDet );

  //Asymptotic resolutions: Ks tracks
  m_asymptoticResKsTrk.push_back( 0 );       // Aerogel
  m_asymptoticResKsTrk.push_back( 1.376e-03 );  // C4F10
  m_asymptoticResKsTrk.push_back( 0.620e-03 );  // CF4
  declareProperty( "AsymptoticResKsTrk", m_asymptoticResKsTrk );

  //FOR PHOTON ITERATION SELECTION
  declareProperty( "NSigmaCut", m_nSigmaCut = 3.00 );

  //Scattering formula constant (P in GeV)
  declareProperty( "CScatt", m_scatt = 13.6e-03 );

  //-------------Acquire instances of tools
  acquireTool( "RichStereoProjection",    m_stereoProj );
  acquireTool( "RichCherenkovAngle",      m_ckAngleTool );
  acquireTool( "RichCherenkovResolution", m_ckResTool );
  acquireTool( "RichParticleProperties",  m_richPartProp );
  acquireTool( "RichGeomEff",             m_geomEffic );
  acquireTool( "RichPhotonSignal",        m_photonSig );

  return sc;
}

//Fitter
int StereoFitter::Fit( RichRecSegment *richSegment, const Rich::ParticleIDType pidType,
                       int MinRingPhotons, double nthcPhotSigMax, int ncandsPerPixMax,
                       double maxBkgProb,
                       double &Chi2, double &probChi2, int &ndof, double &thcFit, double &thcFitErr, double &thphotErr) const
{
  if ( msgLevel(MSG::DEBUG) ) debug()<<"Stereo Fitter"<<endreq;

  if(richSegment!=0) {
    //init some internal variables
    m_chi2Prev=1e33;
    m_chi2ExpPrev=-1;
    m_probPrev=-1;
    m_probExpPrev=-1;
    m_dofPrev=-1;
    m_CerenkovPrev=-1;
    m_CerenkovErrPrev=-1;
    m_photonThcSigmaPrev=-1;

    //basic track/segment info
    Gaudi::XYZVector VPTrk=richSegment->trackSegment().bestMomentum();
    double PTrk=VPTrk.R()/Gaudi::Units::GeV;

    //--------------Cerenkov angle and resolution (above threshold?)
    double thcNominal=m_ckAngleTool->avgCherenkovTheta(richSegment,pidType);
    double thcPhotSig=m_ckResTool->ckThetaResolution(richSegment,pidType);
    if(thcNominal>0 && thcPhotSig>0) {
      //----Rich track associated to segment and related info
      const RichRecTrack *rTrack=richSegment->richRecTrack();
      const LHCb::Track *parentTrack=dynamic_cast<const LHCb::Track*>(rTrack->parentTrack());   //Parent Track
      if (!parentTrack) return(0);
      //radiator type
      Rich::RadiatorType richRadiator=richSegment->trackSegment().radiator();

      //------------geometric info
      //Get state closest to entrance window of radiator
      const State *inRadiatorState = &(parentTrack->closestState(m_zinWindowRich[richRadiator]));
      if(!inRadiatorState) return(0);
      //geometric efficiency for current mass hypothesis
      double geomEfficiency = m_geomEffic->geomEfficiency(richSegment, pidType);

      //---------------------Asymptotic resolutions subtracted from chromatics, for each track type
      Rich::Rec::Track::Type trType = rTrack->trackID().trackType();
      bool unique = rTrack->trackID().unique();
      double asymptoticRes(AsymptoticResDefault);
      if(trType==Rich::Rec::Track::Forward && unique) asymptoticRes=m_asymptoticResFwrd[richRadiator];
      else if(trType==Rich::Rec::Track::Match && unique) asymptoticRes=m_asymptoticResMatch[richRadiator];
      else if(trType==Rich::Rec::Track::Seed && unique) asymptoticRes=m_asymptoticResPreDet[richRadiator];    //same
      else if(trType==Rich::Rec::Track::VeloTT && unique) asymptoticRes=m_asymptoticResPreDet[richRadiator];
      else if(trType==Rich::Rec::Track::KsTrack && unique) asymptoticRes=m_asymptoticResKsTrk[richRadiator];

      //------ preliminary error computation
      double lengthEffect,errMom,err_tx2,err_ty2,trkCharge;
      bool trkOK = trkErrStereo(inRadiatorState,richSegment,lengthEffect,errMom,err_tx2,err_ty2,trkCharge);

      double m_photonThcSigma=improvedErrorPerPhoton_index(PTrk, lengthEffect, errMom, err_tx2, err_ty2,
                                                           thcNominal, richRadiator, asymptoticRes, pidType);

      //------------------- iterative fit section: initialization
      g_XCenterGuess=0; g_YCenterGuess=0;

      int n_iter(0);
      double diffChi2(diffChi2Lim);
      bool segUpdateDone(false), photonUpdateDone(false);

      //save segment direction and crossing points before next steps
      m_segEntryDir=richSegment->trackSegment().entryMomentum();
      m_segMidDir=richSegment->trackSegment().middleMomentum();
      m_segExitDir=richSegment->trackSegment().exitMomentum();
      m_segEntryPtn=richSegment->trackSegment().entryPoint();
      m_segMidPtn=richSegment->trackSegment().middlePoint();
      m_segExitPtn=richSegment->trackSegment().exitPoint();


      RichRecPhoton::ConstVector goodPhotons;
      while((diffChi2 >= diffChi2Lim) && (n_iter < nIterMax)) {
        //------------------Select good photons for ring stereo fit
        RichRecRing recRing;

        if(n_iter==0) {   //first iteration

          //global renormalization of individual photon variance
          m_newErrorPerPhotonSelect*=pow(m_globGammmaNorm[richRadiator],2);

          //-------------select photons using thetaC cut (iteration 0)
          const RichRecSegment::Photons &photons=richSegment->richRecPhotons();     //segment photon candidates
          for(RichRecSegment::Photons::const_iterator iPhot=photons.begin(); iPhot != photons.end(); ++iPhot) {
            if(goodPhotonTheta(*iPhot, pidType, thcNominal, m_photonThcSigma, nthcPhotSigMax, ncandsPerPixMax, maxBkgProb)) {
              goodPhotons.push_back(*iPhot);
            }
          }
          int ngoodPhot=goodPhotons.size();

          //stereo projection
          m_stereoProj->Project(goodPhotons,recRing);

          //compute initial (iteration 0) radius guestimation
          double radiusSquareGuess(0);
          for(int iphot=0;iphot<ngoodPhot;iphot++ ) {
            Gaudi::XYZPoint vphot=recRing.ringPoints()[iphot].localPosition();
            radiusSquareGuess+=(vphot.X()-g_XCenterGuess)*(vphot.X()-g_XCenterGuess);
            radiusSquareGuess+=(vphot.Y()-g_YCenterGuess)*(vphot.Y()-g_YCenterGuess);
          }
          radiusSquareGuess/=ngoodPhot;
          m_RadiusGuess=sqrt(radiusSquareGuess);
        }
        else { // other iterations
          //Update segment to account for ring center changes (due to fit)
          if(!segUpdateDone) {
            updateSegmentDirection(richSegment, goodPhotons, XcenterFitted(), YcenterFitted());
          }

          //update all photons associated to segment
          if (!photonUpdateDone) {
            updateAllPhotonAngles(richSegment, goodPhotons);
          }

          //stereo projection
          int stat1=m_stereoProj->Project(goodPhotons,recRing);

          //-------------select photons using distance in stereo plane (iteration > 0)
          //adapt nsigma cut (in radius) to small radii and/or big errors...
          double NsigRcut;
          double RadiusErrorPhot = sqrt(m_newErrorPerPhotonSelect);
          if((radiusFitted()-m_nSigmaCut*RadiusErrorPhot)<0) NsigRcut=fabs(radiusFitted()/RadiusErrorPhot);
          else NsigRcut=m_nSigmaCut;
          m_RadiusGuess=radiusFitted();     //refresh radius estimation

          int index=0;
          RichRecPhoton::ConstVector goodPhotonsTmp;
          for(RichRecPhoton::ConstVector::iterator iPhot = goodPhotons.begin(); iPhot != goodPhotons.end(); ++iPhot) {
            double xphot=recRing.ringPoints()[index].localPosition().X();
            double yphot=recRing.ringPoints()[index].localPosition().Y();
            double sep = radiusFitted()-sqrt(pow(xphot-XcenterFitted(),2) + pow(yphot-YcenterFitted(),2));
            if(fabs(sep)<NsigRcut*RadiusErrorPhot) goodPhotonsTmp.push_back(*iPhot);
            index++;
          }
          goodPhotons = goodPhotonsTmp;
        }

        //filter according to number of photons
        int ngoodPhot=goodPhotons.size();
        if(ngoodPhot>=MinRingPhotons && ngoodPhot<NmaxFitPhot) {

          //------------- begin fit steps
          //compute covariance matrix, solve chi2 equations, get the fitted solution
          bool statusCovarianceMatrix, statusSolveChi2Eq, statusTransferFit;
          if(statusCovarianceMatrix = newCovarianceMatrix(recRing)) {
            if(statusSolveChi2Eq = solveChi2Equations(recRing)) {
              statusTransferFit=transferFitSolution(recRing);
              if(statusTransferFit) {
                diffChi2=m_chi2Prev-m_chi2;
                //--keep 'old solution' if necessary
                if(diffChi2>0) {
                  for(int i=0;i<3;i++) m_solPrev[i]=m_sol[i];
                  m_chi2Prev=m_chi2;
                  m_chi2ExpPrev=m_chi2Exp;
                  m_probPrev=m_prob;
                  m_probExpPrev=m_probExp;
                  m_dofPrev=m_dof;
                  m_CerenkovPrev=m_Cerenkov;
                  m_CerenkovErrPrev=m_CerenkovErr;
                  m_photonThcSigmaPrev=m_photonThcSigma;
                }
              }
              else {
                resetSegmentDirection(richSegment);
                return(0);
              }
            }
            else {
              resetSegmentDirection(richSegment);
              return(0);
            }
          }
          else {
            resetSegmentDirection(richSegment);
            return(0);
          }
        }
        else {
          if(msgLevel(MSG::DEBUG)) debug()<<"Too many or to few photons in stereo fitter"
                                          <<", ngoodPhot="<<ngoodPhot<<endreq;
          break;
        }
        n_iter++;
      }

      //choose solution depending on chi2
      if(m_chi2Prev<m_chi2) {
        Chi2=m_chi2Prev;
        probChi2=m_probPrev;
        ndof=m_dofPrev;
        thcFit=m_CerenkovPrev;
        thcFitErr=m_CerenkovErrPrev;
        thphotErr=m_photonThcSigmaPrev;
      }
      else {
        Chi2=m_chi2;
        probChi2=m_prob;
        ndof=m_dof;
        thcFit=m_Cerenkov;
        thcFitErr=m_CerenkovErr;
        thphotErr=m_photonThcSigma;
      }

      //reset segment direction to what it was at call time...
      resetSegmentDirection(richSegment);

      return(1);
    }
  }

  return(0);
}









//--------------------------Auxiliary routines

//----------------------------------------------------------------------------
bool StereoFitter::trkErrStereo(const State *state, RichRecSegment *segment,
                                double &lengthEffect, double &errMom, double &err_tx2,
                                double &err_ty2, double &trkCharge) const {

  // derive the errors on the center coordinates
  // in the Equatorial plane to Stereo Sphere

  // now multiple scatt in the entrance window  and in CF4 is
  // accounted for; one has to check whether the entrance window
  // is not already taken into account in the state covariance matrix
  // IF YES m_coefWindow SHOULD BE SET TO ZERO
  // otherwise, here the track direction error includes
  // 1/measurement error 2/mult. scatt. , 3/curvature

  //-------useful informations
  double PTrk=((RichRecSegment*)segment)->trackSegment().bestMomentum().R()/Gaudi::Units::GeV;
  //Rich radiator
  Rich::RadiatorType radiator=((RichRecSegment*)segment)->trackSegment().radiator();

  // get the direction parameters and construct
  // the associated Unit vector in the lab (direction q_lab)
  const double t_x = state->tx();
  const double t_y = state->ty();
  double den = sqrt(1+t_x*t_x+t_y*t_y);
  const Gaudi::XYZVector q_lab(t_x/den,t_y/den,1/den);

  // construct a vector basis in the lab :
  // a basis in the lab is (v_x,v_y,q_lab)
  den=den*den*den;
  const Gaudi::XYZVector v_x = Gaudi::XYZVector(1+t_y*t_y,-t_x*t_y,-t_x) / den ;
  const Gaudi::XYZVector v_y = Gaudi::XYZVector(-t_x*t_y,1+t_x*t_x,-t_y) / den ;

  // attention  v_x and v_y are linearly independent
  // both are orthogonal to q_lab, but not among themselves
  // nota : d_q = d_t_x *v_x + d_t_y * v_y

  // get the errors associated to this stateP
  err_tx2 = state->errTx2();  //these 2 errors are returned because they are mofified
  err_ty2 = state->errTy2();  //in this routine

  double err_txy = (state->errSlopes())(2,3);
  const Gaudi::SymMatrix5x5 &qCovMatrix = state->covariance();
  //####
  double err_txp = qCovMatrix(3,5)*pow(PTrk,2)/Gaudi::Units::GeV;   //(x,y,tx,ty,q/p) matrix used to
  double err_typ = qCovMatrix(4,5)*pow(PTrk,2)/Gaudi::Units::GeV;   //get approximately (tx,p), (ty,p) with |q|=1

  //rms error on PTrk (GeV)
  errMom=sqrt(state->errP2())/Gaudi::Units::GeV;
  trkCharge=state->qOverP();
  if(trkCharge>0) trkCharge=1;
  else if(trkCharge<0) trkCharge=-1;
  else trkCharge=0;

  // compute multiple scattering error, mult scatt for err_txy is neglected
  lengthEffect = sqrt(1+t_x*t_x+t_y*t_y);   //also returned for angular error per photon
  //longitudinal thickness in total for improved scatt. mult. computation
  double totalThicknessOfMaterial=m_thickInterface[radiator]+m_thickRadiator[radiator];
  // (stricly correct in the zero field approximation)
  double thicknessOfMaterial=totalThicknessOfMaterial*lengthEffect;
  double multScattCoeff=m_scatt *sqrt(thicknessOfMaterial)*(1+0.038*log(thicknessOfMaterial));
  double MultScattRes = multScattCoeff*multScattCoeff/(PTrk*PTrk);
  err_tx2 += MultScattRes;
  err_ty2 += MultScattRes;

  // Get the rotation matrix to the unit sphere
  Gaudi::Rotation3D toUnitSphere;
  m_stereoProj->toUnitSphere(segment, toUnitSphere);
  Gaudi::XYZVector rot_q_lab = toUnitSphere * q_lab;
  Gaudi::XYZVector rot_v_x   = toUnitSphere * v_x;
  Gaudi::XYZVector rot_v_y   = toUnitSphere * v_y;

  // the error functions in the stereo frame are
  // errors in the stereo plane are to be divided by 2 (4 for squares)
  //    d_x= delta_t_x * rot_v_x.x()+ delta_t_y * rot_v_y.x()
  //    d_y= delta_t_x * rot_v_x.y()+ delta_t_y * rot_v_y.y()
  //    normally d_z=    delta_t_x * rot_v_x.z()+ delta_t_y * rot_v_y.z() == 0
  // squared track errors at north pole of the stero frame
  // squared track errors in the equatorial plane are quarter of these

  m_err_x2 =    err_tx2   * rot_v_x.x() * rot_v_x.x()
    + err_ty2   * rot_v_y.x() * rot_v_y.x()
    + 2*err_txy *  rot_v_x.x() * rot_v_y.x();
  m_err_x2 /=4;
  if(m_err_x2 < 1.e-12) m_err_x2 = 1.e-12;

  m_err_y2 =    err_tx2   * rot_v_x.y() * rot_v_x.y()
    + err_ty2   * rot_v_y.y() * rot_v_y.y()
    + 2*err_txy *  rot_v_x.y() * rot_v_y.y();
  m_err_y2 /=4;
  if(m_err_y2 < 1.e-12) m_err_y2 = 1.e-12;

  m_err_xy =    err_tx2 * rot_v_x.x() * rot_v_x.y()
    + err_ty2 * rot_v_y.x() * rot_v_y.y()
    + err_txy * (rot_v_x.x() * rot_v_y.y()+rot_v_x.y() * rot_v_y.x()) ;
  m_err_xy /=4;

  //needs to be renormalized to (m^2/E^2) :done in newTrackAndPhotonParams
  m_err_xp = err_txp * rot_v_x.x() + err_typ * rot_v_y.x();
  m_err_xp /= 4*PTrk;
  m_err_yp = err_typ * rot_v_x.y() + err_txp * rot_v_y.y();
  m_err_yp /= 4*PTrk;

  return true;
}




//----------------------------------------------------------------------------
double  StereoFitter::improvedErrorPerPhoton_index(double PTrk, double lengthEffect,
                                                   double errMom, double err_tx2, double err_ty2, double ckExp,
                                                   Rich::RadiatorType radiator, double asymptoticRes,
                                                   const Rich::ParticleIDType pidType) const {

  //   COMPUTING THE RECONSTRUCTION ERRORS
  //   HERE WE LEAVE THE CHROMATIC ERROR VARY WITH THE CERENKOV ANGLE
  //   AND ACCORDINGLY THE CHROMATIC ERROR IS SUBTRACTED FROM ASYMPTOTIC ERROR
  //   numbers valid for rich2 ONLY and already loaded
  //
  // as we have to take the error relative to a given track direction
  // we should include multiple scattering and curvature errors
  // curvature errors are optimum if using the chord definition for the track

  //compute kinematic stuff
  double massPid=m_richPartProp->mass(pidType)/Gaudi::Units::GeV;
  double massFactor=pow(massPid,2)/(PTrk*PTrk+pow(massPid,2));
  double betaPid = m_richPartProp->beta(PTrk*Gaudi::Units::GeV, pidType);  //be careful with units!!!
  double EPid = sqrt(pow(massPid,2) + pow(PTrk,2));

  //estimating multiple scattering effects:
  double totalThicknessOfMaterial=m_thickInterface[radiator]+m_thickRadiator[radiator];
  double thicknessOfMaterial=totalThicknessOfMaterial*lengthEffect;
  double multScattCoeff=m_scatt *sqrt(thicknessOfMaterial)*(1+0.038*log(thicknessOfMaterial));   //####
  double MultScattRes2 = 2*multScattCoeff*multScattCoeff/(PTrk*PTrk);

  //curvature effects :
  double CurvatureRes2 = (pow(m_curvX[radiator],2) + pow(m_curvY[radiator],2))/(PTrk*PTrk) ;

  //multiple scattering +curvature effects (PTrk in GeV)
  double smearedTotal2=MultScattRes2+CurvatureRes2;

  //compute asymptotic and non asymptotic chromatic error
  double chromaticErrCoeff=m_rmsIndexRad[radiator]/m_meanIndexRad[radiator];
  double asymptoticChromaticErr=chromaticErrCoeff/sqrt(pow(m_meanIndexRad[radiator],2)-1);
  double chromaticErr2 = pow(chromaticErrCoeff,2)/(tan(ckExp)*tan(ckExp));

  double errMomThc = errMom * massFactor/PTrk/tan(ckExp);

  // starting values are rounded from last cerenkov bins of Chris
  double asymptoticResSub=sqrt(pow(asymptoticRes,2)-pow(asymptoticChromaticErr,2));
  double resolutionA2=pow(asymptoticResSub,2)+smearedTotal2;

  // also take into account error on the averaged charged track direction
  // what done below is strictly valid at zero magnetic field
  // this correction is small (5-15%) except at low momenta (5 a 10 GeV/c)
  // where it can amounts to 20-50%
  double errAvrgDir2=err_tx2+err_ty2;

  // ===>>  total variance is :
  double resolution2 = resolutionA2 + errAvrgDir2 + chromaticErr2 + pow(errMomThc,2);

  //error per photon in selection
  m_newErrorPerPhotonSelect = pow(asymptoticResSub,2) + CurvatureRes2 + MultScattRes2 + errAvrgDir2 + chromaticErr2;
  m_var_nocor=pow(asymptoticResSub,2) + CurvatureRes2;

  //compute track induced Cerenkov radius and variance
  //approximate variance only due to PTrk error(neglect corr. with track dir)
  m_radiusSquaredPid = (betaPid * m_meanIndexRad[radiator]-1)/(betaPid * m_meanIndexRad[radiator]+1);
  double errMomMf = errMom * massFactor/PTrk;
  double var_n = pow(m_rmsIndexRad[radiator]/m_meanIndexRad[radiator],2);
  double var_b=pow(errMomMf,2);
  double m_var_c = var_n + var_b; //db/b+dn/n no longer divided by 4

  m_radiusSquaredPid -= m_var_c/4; //#### //unbiased
  if(m_radiusSquaredPid < 0) m_radiusSquaredPid=1.0e-15;   // m_radiusSquaredPid <0 problem

  //renormalize the covariance terms to mass dependant factors
  m_err_xp *= pow(massPid/EPid,2);
  m_err_yp *= pow(massPid/EPid,2);
  m_var_beta  = var_b;
  m_var_index = var_n;

  return sqrt(resolution2);
}




//----------------------------------------------------------------------------
// selects the best photons in a window around a given theta Cerenkov angle
bool StereoFitter::goodPhotonTheta( RichRecPhoton *photon, const Rich::ParticleIDType pidHypo,
                                    double thcCentral, double thcPhotSig, double nthcPhotSigMax,
                                    unsigned int ncandsPerPixMax, double maxBkgProb ) const {

  //Check multiplicity of this pixel i.e. number of photons made from it.
  if(photon->richRecPixel()->richRecPhotons().size() > ncandsPerPixMax) return false;

  // check background prob
  if(photon->richRecPixel()->currentBackground() > maxBkgProb) return false;

  double thetaExpRes=m_ckResTool->ckThetaResolution(photon->richRecSegment(),pidHypo);
  if(thetaExpRes < MinthetaExpRes) return false;

  double thetaPhot=photon->geomPhoton().CherenkovTheta();
  if(thcPhotSig>0) {
    double nSig=fabs((thetaPhot-thcCentral)/thcPhotSig);
    if(nSig > nthcPhotSigMax) return false;
  }
  else return false;

  return true;
}





//-----------------------covariance computation routine
bool StereoFitter::newCovarianceMatrix( RichRecRing &recRing ) const {

  std::vector<double> radius;
  radius.clear();
  int nphot=recRing.ringPoints().size();

  double C2(0),S2(0),CS(0),C(0),S(0);
  for(int i=0;i<nphot; ++i ) {
    double xphot=recRing.ringPoints()[i].localPosition().X();
    double yphot=recRing.ringPoints()[i].localPosition().Y();
    double rr2=pow(xphot,2)+pow(yphot,2);
    double rr=sqrt(rr2);
    radius.push_back(rr);
    C2 += pow(xphot,2)/rr2;
    S2 += pow(yphot,2)/rr2;
    CS += xphot*yphot/rr2;
    C  += xphot/rr;
    S  += yphot/rr;
  }

  //get track errors
  double p2 = (m_err_x2 > 2.0e-09 ? m_err_x2 : 2.0e-09);
  double q2 = (m_err_y2 > 4.0e-09 ? m_err_y2 : 4.0e-09);
  double cv=0;

  double RadiusGuess2=m_RadiusGuess*m_RadiusGuess;
  double fc=1;
  double ff=(1-RadiusGuess2)/2/m_RadiusGuess;

  //========>>  work out possibility # 2
  double lambda= RadiusGuess2*(m_var_nocor + ff * ff * m_var_index);
  double mu=RadiusGuess2 * ff * ff *m_var_beta;
  double scale= 4 * RadiusGuess2;   //scale *= zero;
  double CA=m_err_xp*2*m_RadiusGuess;   //presently vanishing
  double CB=m_err_yp*2*m_RadiusGuess;
  double newCov[3][3];

  newCov[0][0]= (lambda   + scale * p2 * C2 + scale *cv * CS  + CA * C)/lambda ;
  newCov[0][1]= (scale * cv* S2   + scale * p2 * CS           + CA * S)/lambda ;
  newCov[0][2]= (scale * cv* S    + scale * p2 * C            + CA * nphot)/lambda;

  newCov[1][0]= (scale * cv * C2  + scale * q2 * CS           + CB * C)/lambda;
  newCov[1][1]= (lambda   + scale * q2 * S2 + scale * cv * CS + CB * S)/lambda;
  newCov[1][2]= (scale * cv * C   + scale * q2 * S            + CB * nphot)/lambda;

  newCov[2][0]= (mu * C   + CB * CS           + CA * C2)/lambda;
  newCov[2][1]= (mu * S   + CA * CS           + CB * S2)/lambda;
  newCov[2][2]= (lambda   + mu * nphot + CA * C + CB * S)/lambda;

  double newCovInv[3][3];
  bool status = invert3x3Matrix(newCov,newCovInv);
  if(!status) return status;

  //renormalize
  for(int i =0;i<3; i++) {
    for(int j =0;j<3; j++) newCovInv[i][j] /= lambda;
  }

  //test 1
  if(nphot > NmaxFitPhot) return !status;

  double eta=1./lambda;

  double W1[3];
  W1[0] =  -eta * CA;
  W1[1] =  -eta * CB;
  W1[2] =  -eta * mu;

  double W2[3];
  W2[0] =  -eta * scale * p2;
  W2[1] =  -eta * scale * cv;
  W2[2] =  -eta * CA;

  double W3[3];
  W3[0] =  -eta * scale * cv;
  W3[1] =  -eta * scale * q2;
  W3[2] =  -eta * CB;

  double sol1[3];
  double sol2[3];
  double sol3[3];

  for(int i =0;i<3;i++) {
    sol1[i]=0;
    sol2[i]=0;
    sol3[i]=0;
    for(int j =0;j<3;j++) {
      sol1[i] += newCovInv[i][j]*W1[j];
      sol2[i] += newCovInv[i][j]*W2[j];
      sol3[i] += newCovInv[i][j]*W3[j];
    }
  }

  double ci; double si; double gi =1;
  double cj; double sj; double gj =1;

  for(int i=0; i < nphot; i++) {
    for(int j=0; j < nphot; j++) m_invCov[i][j]=0;
  }

  for(int i=0; i < nphot; i++)  m_invCov[i][i] = eta;

  for(int i=0; i < nphot; i++) {
    double xphoti=recRing.ringPoints()[i].localPosition().X();
    double yphoti=recRing.ringPoints()[i].localPosition().Y();
    ci=xphoti/radius[i];
    si=yphoti/radius[i];
    for(int j=0; j < nphot; j++) {
      if(j>=i) {
        double xphotj=recRing.ringPoints()[j].localPosition().X();
        double yphotj=recRing.ringPoints()[j].localPosition().Y();
        cj=xphotj/radius[j];
        sj=yphotj/radius[j];

        m_invCov[i][j] += (sol2[0]*ci*cj+sol3[1]*si*sj+sol3[0]*(ci*sj+cj*si) +
                           sol1[0]*(ci*gj+cj*gi)+sol1[1]*(si*gj+sj*gi)+ sol1[2]*gi*gj)/fc;

      }
      else m_invCov[i][j]=m_invCov[j][i];
    }
  }
  return status;
}





//----------------- Solve chi2 equations
bool StereoFitter::solveChi2Equations(RichRecRing &recRing) const {

  //computing moments
  int nphot=recRing.ringPoints().size();
  double xVx(0),yVy(0),xVy(0),xVg(0),yVg(0),gVg(0),xVr2(0),yVr2(0),gVr2(0);
  {for(int i=0; i<nphot; ++i) {
    for(int j=0; j<nphot; ++j) {
      double xphoti=recRing.ringPoints()[i].localPosition().X();
      double yphoti=recRing.ringPoints()[i].localPosition().Y();
      double xphotj=recRing.ringPoints()[j].localPosition().X();
      double yphotj=recRing.ringPoints()[j].localPosition().Y();

      xVx+=xphoti*m_invCov[i][j]*xphotj;
      xVy+=xphoti*m_invCov[i][j]*yphotj;
      yVy+=yphoti*m_invCov[i][j]*yphotj;
      xVg+=xphoti*m_invCov[i][j];
      yVg+=yphoti*m_invCov[i][j];
      gVg+=m_invCov[i][j];
      double r2=xphotj*xphotj+yphotj*yphotj;
      xVr2+=xphoti*m_invCov[i][j]*r2;
      yVr2+=yphoti*m_invCov[i][j]*r2;
      gVr2+=m_invCov[i][j]*r2;
    }
  }} //Fix scope for Windows

  //defining the moment matrix (also the inverse error matrix)
  //in presence of the center fixing term
  double p2 = (m_err_x2 > 2.0e-09 ? m_err_x2 : 2.0e-09);
  double q2 = (m_err_y2 > 4.0e-09 ? m_err_y2 : 4.0e-09);
  double cv=0;
  ////  if(p2 == 2.0e-09 || q2 == 4.0e-09) cv=0;
  ////  else cv = m_err_xy;

  double discr=q2*p2-cv*cv;

  //creates the minimization of chi2 equations
  double fgc(1);
  double fgg=4/(m_var_beta + m_var_index);

  m_mat[0][0]=4*xVx + fgc * q2/discr ;
  m_mat[0][1]=4*xVy - fgc * cv/discr ;
  m_mat[1][0]=4*xVy - fgc * cv/discr ;
  m_mat[1][1]=4*yVy + fgc * p2/discr ;
  m_mat[2][0]=2*xVg;
  m_mat[0][2]=2*xVg;
  m_mat[2][1]=2*yVg;
  m_mat[1][2]=2*yVg;
  m_mat[2][2]=gVg+fgg;

  m_right[0]=2*xVr2;
  m_right[1]=2*yVr2;
  m_right[2]=gVr2 + fgg * m_radiusSquaredPid ;

  //solve the system
  m_errPar[0][0]=m_mat[1][1]*m_mat[2][2]-m_mat[1][2]*m_mat[2][1];
  m_errPar[1][0]=m_mat[1][2]*m_mat[2][0]-m_mat[1][0]*m_mat[2][2];
  m_errPar[2][0]=m_mat[1][0]*m_mat[2][1]-m_mat[1][1]*m_mat[2][0];

  m_errPar[0][1]=m_mat[0][2]*m_mat[2][1]-m_mat[0][1]*m_mat[2][2];
  m_errPar[1][1]=m_mat[0][0]*m_mat[2][2]-m_mat[0][2]*m_mat[2][0];
  m_errPar[2][1]=m_mat[0][1]*m_mat[2][0]-m_mat[0][0]*m_mat[2][1];

  m_errPar[0][2]=m_mat[0][1]*m_mat[1][2]-m_mat[0][2]*m_mat[1][1];
  m_errPar[1][2]=m_mat[0][2]*m_mat[1][0]-m_mat[0][0]*m_mat[1][2];
  m_errPar[2][2]=m_mat[0][0]*m_mat[1][1]-m_mat[0][1]*m_mat[1][0];

  double det;
  det =m_mat[0][0]*m_errPar[0][0];
  det+=m_mat[0][1]*m_errPar[1][0];
  det+=m_mat[0][2]*m_errPar[2][0];

  //fabs not needed because matrix should be positive definite
  if(det < 1.0e-17) {
    std::cout << " ---<> from solveMB() BAD FIT , det = "<< det << std::endl;
    std::cout << " singular matrix = " << std::endl;
    return false;
  }

  //the error matrix
  for(int i=0;i<3;++i) for(int j=0;j<3;++j) m_errPar[i][j] /= det;

  //the fit solution
  for(int i=0;i<3;++i) {
    m_sol[i] = 0;
    for(int j=0;j<3;++j) m_sol[i] += m_errPar[i][j]*m_right[j];
  }

  return true;
}




//----------------------------------------------------------------------
int StereoFitter::transferFitSolution(RichRecRing &recRing) const {

  //transfer of parameter errors
  double err_R;
  double radius2=m_sol[2]+m_sol[0]*m_sol[0]+m_sol[1]*m_sol[1];
  err_R=m_errPar[2][2];
  err_R+=4*m_sol[0]*m_sol[0]*m_errPar[0][0];
  err_R+=4*m_sol[1]*m_sol[1]*m_errPar[1][1];
  err_R+=4*m_sol[0]*m_errPar[0][2];
  err_R+=4*m_sol[1]*m_errPar[1][2];
  err_R+=8*m_sol[0]*m_sol[1]*m_errPar[0][1];
  err_R=err_R/(4*radius2);
  m_err_R=sqrt(err_R);

  //statistical info
  double radiusSquareFitted=m_sol[2]+m_sol[0]*m_sol[0]+m_sol[1]*m_sol[1];
  m_chi2=chiSquare(recRing,radiusSquareFitted);
  m_chi2Exp=chiSquare(recRing, m_radiusSquaredPid);

  if(m_chi2<0 || m_chi2Exp<0) {   //bad chi2 //####DBL
    std::cout<<"---FastRingFitter::transferFitSolutionNormal: Chi2<0: "<<m_chi2<<" "<<m_chi2Exp<<std::endl;
    return(0);
  }

  m_dof=recRing.ringPoints().size();
  m_prob=Proba(m_chi2,m_dof);
  m_probExp=Proba(m_chi2Exp,m_dof);

  // Cerenkov Angle and its error
  double tgTheta=2*radiusFitted()/(1-m_sol[2]);
  double Theta_Crk=atan(tgTheta);
  m_Cerenkov=Theta_Crk;

  double UnPlusTg2 = 1+tgTheta*tgTheta;
  double cf_a = 2*m_sol[0]/radiusFitted()/(1-m_sol[2]);
  double cf_b = 2*m_sol[1]/radiusFitted()/(1-m_sol[2]);
  double cf_S = (1-2*radiusFitted()*radiusFitted()/(1-m_sol[2]));
  cf_S *= 1/radiusFitted()/(1-m_sol[2]);
  double errTheta=cf_S*cf_S*m_errPar[2][2]+cf_a*cf_a*m_errPar[0][0]+cf_b*cf_b*m_errPar[1][1];
  errTheta+=2*cf_S*cf_a*m_errPar[0][2]+2*cf_S*cf_b*m_errPar[1][2]+2*cf_a*cf_b*m_errPar[0][1];
  errTheta=sqrt(errTheta)/UnPlusTg2;
  m_CerenkovErr=errTheta;

  return(1);
}



// Update the segment direction
void StereoFitter::updateSegmentDirection(RichRecSegment *richSegment, RichRecPhoton::ConstVector &photons,
                                          double Xcenter, double Ycenter) const {

  //form rotation transformation to update segment directions
  Gaudi::XYZVector newDir(Xcenter, Ycenter, 1);
  double angle = ROOT::Math::VectorUtil::Angle( newDir, Gaudi::XYZVector(0,0,1) );
  Gaudi::XYZVector axis = newDir.Cross( Gaudi::XYZVector(0,0,1) );
  Gaudi::AxisAngle axisangle(axis, angle);
  Gaudi::Transform3D trans(axisangle);

  // Update tracking information using average photon emission point
  Gaudi::XYZPoint avgEpoint;
  m_stereoProj->avgEmissionPnt(photons,avgEpoint);
  richSegment->trackSegment().updateState(avgEpoint,trans);
}



// Recomputes cherenkov angles for all photons associated to the parent segment
void StereoFitter::updateAllPhotonAngles(RichRecSegment *richSegment, RichRecPhoton::ConstVector &photons) const {

  for(RichRecPhoton::ConstVector::iterator iPhot=photons.begin(); iPhot!=photons.end(); ++iPhot) {
    //photon direction
    RichGeomPhoton &gPhot = (RichGeomPhoton&)((*iPhot)->geomPhoton());   //discard "const"
    Gaudi::XYZVector photonDir = gPhot.sphMirReflectionPoint() - gPhot.emissionPoint();

    // Update angles using updated segment
    double thetaC, phiC;
    richSegment->trackSegment().angleToDirection( photonDir, thetaC, phiC );
    gPhot.setCherenkovTheta(thetaC);
    gPhot.setCherenkovPhi(phiC);

    //reset photon to force recalculation of derived quantites in proceding algorithms  (discard "const")
    RichRecPhoton *iPhotNonConst=(RichRecPhoton*)(*iPhot);
    iPhotNonConst->reset();
  }
}


void StereoFitter::resetSegmentDirection(RichRecSegment *richSegment) const {
  //reset segment direction to what it was...
  richSegment->trackSegment().setEntryState(m_segEntryPtn, m_segEntryDir);
  richSegment->trackSegment().setMiddleState(m_segMidPtn, m_segMidDir);
  richSegment->trackSegment().setExitState(m_segExitPtn, m_segExitDir);
}


double StereoFitter::chiSquare(RichRecRing &recRing, double radiusSquare) const {
  std::vector<double> func;
  func.clear();

  double xc=m_sol[0];
  double yc=m_sol[1];

  int nphot=recRing.ringPoints().size();
  for(int i=0;i<nphot;i++) {
    double xp = recRing.ringPoints()[i].localPosition().X();
    double yp = recRing.ringPoints()[i].localPosition().Y();
    double ff = (xp-xc)*(xp-xc) + (yp-yc)*(yp-yc) - radiusSquare;
    func.push_back(ff);
  }

  double chi2(0);
  for(int i=0;i<nphot;i++) for(int j=0;j<nphot;j++) chi2+=func[i]*func[j]*m_invCov[i][j];

  double det=m_err_x2*m_err_y2-pow(m_err_xy,2);
  if(det<1e-18) det=1e-18;
  double uu = (m_sol[0]*m_sol[0]*m_err_y2 + m_sol[1]*m_sol[1]*m_err_x2-2*m_sol[0]*m_sol[1]*m_err_xy)/det;
  chi2 += uu;

  double den=(m_var_beta+m_var_index)/4 ;
  chi2+=(m_sol[2]-m_radiusSquaredPid)*(m_sol[2]-m_radiusSquaredPid)/den;

  return chi2;
}


//more utility functions
double StereoFitter::radiusFitted() const {
  if(m_sol[2]+m_sol[0]*m_sol[0]+m_sol[1]*m_sol[1>=0]) return(sqrt(m_sol[2]+m_sol[0]*m_sol[0]+m_sol[1]*m_sol[1]));
  else return(0);
};

double StereoFitter::XcenterFitted() const { return(m_sol[0]); };
double StereoFitter::YcenterFitted() const { return(m_sol[1]); };


double StereoFitter::Proba(double chi2,double ndl) const {
  if(ndl>0 && chi2>=0) return gsl_sf_gamma_inc_Q(ndl/2.0,chi2/2.0);
  else return 0;
}


// inversion of a 3 by 3 matrix ; successfull if returned bool is  = true
// matrix is not necessarily symmetric ; tests OK
bool StereoFitter::invert3x3Matrix(double mat[3][3], double invMat[3][3]) const {

  invMat[0][0]=mat[1][1]*mat[2][2]-mat[1][2]*mat[2][1];
  invMat[1][0]=mat[1][2]*mat[2][0]-mat[1][0]*mat[2][2];
  invMat[2][0]=mat[1][0]*mat[2][1]-mat[1][1]*mat[2][0];

  invMat[0][1]=mat[0][2]*mat[2][1]-mat[0][1]*mat[2][2];
  invMat[1][1]=mat[0][0]*mat[2][2]-mat[0][2]*mat[2][0];
  invMat[2][1]=mat[0][1]*mat[2][0]-mat[0][0]*mat[2][1];

  invMat[0][2]=mat[0][1]*mat[1][2]-mat[0][2]*mat[1][1];
  invMat[1][2]=mat[0][2]*mat[1][0]-mat[0][0]*mat[1][2];
  invMat[2][2]=mat[0][0]*mat[1][1]-mat[0][1]*mat[1][0];

  double det;
  det =mat[0][0]*invMat[0][0];
  det+=mat[1][0]*invMat[0][1];
  det+=mat[2][0]*invMat[0][2];

  if(fabs(det)<1.0e-15) {
    std::cout << " ---<> from RichStereoFitter::invert3x3Matrix , det = " << det << std::endl;
    std::cout << " singular matrix = " << std::endl;
    return false;
  }

  // the inverse matrix
  for(int i=0;i<3;++i) for(int j=0;j<3;++j) invMat[i][j]/=det;
  return true;
}

