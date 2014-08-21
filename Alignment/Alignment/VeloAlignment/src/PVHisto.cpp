// Include files
// from STL
#include <string>

// local
#include "PVHisto.h"
using namespace std; 

//-----------------------------------------------------------------------------
// Implementation file for class : PVHisto
//
// 2002-12-02 : Philippe Charpentier
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PVHisto::PVHisto()
{}

PVHisto::PVHisto( double min, double max, double width )
  : m_Min(min)
  , m_Max(max)
  , m_BinWidth(width)
  , m_Bins (int(0.99 + (max-min)/width))
  , m_Histo(m_Bins, 0)
  , m_MaxVal(0)
  { 
    m_MaxBin = m_Histo.begin();
  }
PVHisto::PVHisto( double min, double max, int bins )
  : m_Min(min)
  , m_Max(max)
  , m_BinWidth((max-min)/bins)
  , m_Bins (bins)
  , m_Histo(bins, 0)
  , m_MaxVal(0)
  { 
    m_MaxBin = m_Histo.begin();
  }
//=============================================================================
PVHisto::iterator PVHisto::fill( double val )
{
  int bin = int((val - m_Min)/m_BinWidth);
  if( bin < 0 || bin >= m_Bins ) return m_Histo.end(); // return an iterator outside the histogram
  int newContent = ++m_Histo[bin];
  if( newContent > m_MaxVal ) {
    m_MaxVal = newContent;
    m_MaxBin = m_Histo.begin() + bin;
  }
  return m_Histo.begin() + bin;
}

PVHisto::iterator PVHisto::getbin( double val )
{
  return (m_Histo.begin() + int((val - m_Min)/m_BinWidth));
}

void PVHisto::reset()
{
  m_Histo = std::vector<int>( m_Bins, 0);
  m_MaxVal = 0.;
  m_MaxBin = m_Histo.begin();  
}



/*
-----------------------------------------------------------
  FINDPVTRACKS:  looking for primary vetices candidates
-----------------------------------------------------------


  ....This method is under development....

-----------------------------------------------------------
*/

StatusCode PVHisto::FindPVTracks(VeloTracks& aPV, double m_z_min,
				 double m_z_max, int m_PV_trackmin, double m_z_sigma, 
				 double m_IPmax, double m_TrIPmax, int& nPV) 
{
  //cout << "" << endl; 
  //cout << "Try to find PV " << endl;
  //cout << "" << endl; 

  VeloTracks::const_iterator itrack;

  double m_Sigma = m_z_sigma;
  double m_Gran = 1.;

  int n_sel_tracks;
  int n_v_tracks;
  int max = 0; 

  int sigmaBins = 1 + int(m_Sigma/m_Gran);

  bool keepgoing = true;

  int iteration = 0;

  PVHisto::iterator start, end, maxBin;
 
  for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
  {
    aPV[iteration].setNPVnumber(-99);  
    iteration++;
  }

  iteration = 0;

  // Histogram for peak finding and parallel vector to pvTracks for caching track info
  PVHisto zCloseHisto( m_z_min, m_z_max, m_Gran);

  while (keepgoing == true)
  {
    n_sel_tracks = 0;
    zCloseHisto.reset();

    for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
    {
      if(aPV[iteration].nPVnumber() == -99 && aPV[iteration].nZclos() > m_z_min && aPV[iteration].nZclos() < m_z_max)  
      {
        n_sel_tracks++;
	zCloseHisto.fill(aPV[iteration].nZclos());
      }

      iteration++;
    }

    iteration = 0;

    // Don't need to find a maximum if there is not enough tracks 

    if (n_sel_tracks < m_PV_trackmin)
    {
      max = -1; 
      keepgoing = false; // end of the quest
    }
    else
    {
      // Find the maximum of the histogram
      max = 0;
      for( PVHisto::iterator bin = zCloseHisto.begin(); zCloseHisto.end() != bin; bin++) 
      {
        if( *bin >= max ) 
	{
	  max = *bin;
	  maxBin = bin;
        }
      }
      
      // Try and make a preliminary vertex out of this bump
      start = maxBin - sigmaBins;
      if( start < zCloseHisto.begin() ) start = zCloseHisto.begin();
      end = maxBin + sigmaBins;
      if( end > zCloseHisto.end() ) end = zCloseHisto.end();
      
      // Now fill in the track vector
      // Count the number of tracks

      nPV++;
      n_v_tracks = 0;
      PVHisto::iterator zclos_bin;

      // First loop just to count the number of tracks in the window

      for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
      {
       	if(aPV[iteration].nPVnumber() == -99)  
	{
	  zclos_bin = zCloseHisto.getbin(aPV[iteration].nZclos());      
	  
	  if (zclos_bin <= end && zclos_bin >= start)
	  {
	    n_v_tracks++;
	    aPV[iteration].setNPVnumber(nPV);      
	  }
	}

	iteration++;
      }

      iteration=0;

      if (n_v_tracks < m_PV_trackmin) // Not enough tracks
      {
	for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
	{
	  if (aPV[iteration].nPVnumber() == nPV) // Throw the track 
	  {
	    aPV[iteration].setNPVnumber(-1);      
	  }

	  iteration++;
	}

	iteration=0;

 	nPV--;
      }
      else // Good peak
      {

	PVHisto::FitPV(aPV,nPV);

	// New loop to suppress outliers

	int n_v_tracks_total = n_v_tracks;

	for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
	{
	  if (aPV[iteration].nPVnumber() == nPV) 
	  {
	    double vp_x = aPV[iteration].nPV_x();
	    double vp_y = aPV[iteration].nPV_y();
	    double vp_z = aPV[iteration].nPV_z();
	    double chisq = aPV[iteration].nPV_chi();

	    double x0 = aPV[iteration].nXo_x()+aPV[iteration].nSlope_x()*vp_z;
	    double y0 = aPV[iteration].nYo_y()+aPV[iteration].nSlope_y()*vp_z;
	    double sig_x = aPV[iteration].nErrX_x();
	    double sig_y = aPV[iteration].nErrY_y();

	    double IParam = sqrt((x0-vp_x)*(x0-vp_x)+(y0-vp_y)*(y0-vp_y));
	    double chiprop = (((x0-vp_x)*(x0-vp_x))/(sig_x*sig_x)+((y0-vp_y)*(y0-vp_y))/(sig_y*sig_y))/chisq;
	    double IP_PV  = sqrt(vp_x*vp_x+vp_y*vp_y);
	    
	    if (chiprop >= 5./float(n_v_tracks_total) || IParam >= m_TrIPmax || IP_PV >= m_IPmax)
	    {
	      aPV[iteration].setNPVnumber(-1);      
	      n_v_tracks--;
	    }
	  }

	  iteration++;
	}

	iteration = 0;
      
	if (n_v_tracks < m_PV_trackmin) // Not enough tracks after rejection
	{
	  for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
	  {
	    if (aPV[iteration].nPVnumber() == nPV) // Make the track non re-usable (!!perfectible!!)  
	    {
	      aPV[iteration].setNPVnumber(-1);      
	    }
	    
	    iteration++;
	  }

	  iteration = 0;

	  nPV--;
	}
	else  
	{
	  // Refit the track
	  PVHisto::FitPV(aPV,nPV);

	}
      }
    }
  }
 
  return StatusCode::SUCCESS;
}


StatusCode PVHisto::FitPV(VeloTracks& aPV, int PV_number)
{
  VeloTracks::const_iterator itrack;

  double sum_zclos = 0.0;

  double a_x = 0.0;
  double b_x = 0.0;
  double c_x = 0.0;
  double d_x = 0.0;
  double e_x = 0.0;

  double a_y = 0.0;
  double b_y = 0.0;
  double c_y = 0.0;
  double d_y = 0.0;
  double e_y = 0.0;

  int iteration = 0;

  for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
  {
    if (aPV[iteration].nPVnumber() == PV_number) 
    {
      sum_zclos += aPV[iteration].nZclos();
	      
      double sX = aPV[iteration].nSlope_x();
      double sY = aPV[iteration].nSlope_y();
      double x0 = aPV[iteration].nXo_x();
      double y0 = aPV[iteration].nYo_y();
      double sig_x = aPV[iteration].nErrX_x();
      double sig_y = aPV[iteration].nErrY_y();

      a_x += x0/(sig_x*sig_x);
      b_x += 1/(sig_x*sig_x);
      c_x += sX/(sig_x*sig_x);
      d_x += (sX*sX)/(sig_x*sig_x);
      e_x += (sX*x0)/(sig_x*sig_x);
      
      a_y += y0/(sig_y*sig_y);
      b_y += 1/(sig_y*sig_y);
      c_y += sY/(sig_y*sig_y);
      d_y += (sY*sY)/(sig_y*sig_y);
      e_y += (sY*y0)/(sig_y*sig_y); 	    
    }

    iteration++;

  }

  iteration = 0;
	
  double vp_z = ((c_x*a_x)/b_x+(c_y*a_y)/b_y-e_x-e_y)/(d_x+d_y-(c_x*c_x)/b_x-(c_y*c_y)/b_y); 
  double vp_y = c_y*vp_z/b_y+a_y/b_y;
  double vp_x = c_x*vp_z/b_x+a_x/b_x;
	
  // Then compute the chisquare

  double chisq = 0.;
  int n_dof = 0;

  for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
  {
    if (aPV[iteration].nPVnumber() == PV_number) // Track into the vertex
    {
      double x0 = aPV[iteration].nXo_x()+aPV[iteration].nSlope_x()*vp_z;
      double y0 = aPV[iteration].nYo_y()+aPV[iteration].nSlope_y()*vp_z;
      double sig_x = aPV[iteration].nErrX_x();
      double sig_y = aPV[iteration].nErrY_y();

      chisq += ((x0-vp_x)/sig_x)*((x0-vp_x)/sig_x);
      chisq += ((y0-vp_y)/sig_y)*((y0-vp_y)/sig_y);
      n_dof += 2;
    }

    iteration++;
  }

  iteration=0;

  for (itrack = aPV.begin(); itrack != aPV.end(); ++itrack ) 
  {
    if (aPV[iteration].nPVnumber() == PV_number) // Track into the vertex
    {
      aPV[iteration].setNPV_x(vp_x); 
      aPV[iteration].setNPV_y(vp_y); 
      aPV[iteration].setNPV_z(vp_z); 
      aPV[iteration].setNPV_chi(chisq); 
    }

    iteration++;
  }

  iteration=0;

  
  //cout << " " << endl;
  //cout << "Primary vertex coordinates (in mm): " << endl;
  //cout << "x_v = " << vp_x << endl;
  //cout << "y_v = " << vp_y << endl;
  //cout << "z_v = " << vp_z << endl;
  //cout << " " << endl;
  
  //cout << " Final Chisquare is " << chisq << endl;
  //cout << " Final ndof is " << n_dof << endl;
  //cout << " " << endl;  
  
 
  return StatusCode::SUCCESS;
} 

