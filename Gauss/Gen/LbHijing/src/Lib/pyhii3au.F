    
C***********************************************************************    
    
      FUNCTION PYHII3AU(BE,EPS,IREIM) 
      IMPLICIT REAL*8(A-H,O-Z)
C...Calculates real and imaginary parts of the auxiliary function I3;   
C...see R. K. Ellis, I. Hinchliffe, M. Soldate and J. J. van der Bij,   
C...FERMILAB-Pub-87/100-T, LBL-23504, June, 1987    
      COMMON/HILUDAT1/MSTU(200),PARU(200),MSTJ(200),PARJ(200) 
      SAVE /HILUDAT1/ 
    
      IF(EPS.LT.1.) GA=0.5*(1.+SQRT(1.-EPS))    
    
      IF(EPS.LT.0.) THEN    
        F3RE=PYHISPEN((GA-1.)/(GA+BE-1.),0.,1)-
     &  PYHISPEN(GA/(GA+BE-1.),0.,1)+    
     &  PYHISPEN((BE-GA)/BE,0.,1)-PYHISPEN((BE-GA)/(BE-1.),0.,1)+   
     &  (LOG(BE)**2-LOG(BE-1.)**2)/2.+LOG(GA)*LOG((GA+BE-1.)/BE)+   
     &  LOG(GA-1.)*LOG((BE-1.)/(GA+BE-1.))  
        F3IM=0. 
      ELSEIF(EPS.LT.1.) THEN    
        F3RE=PYHISPEN((GA-1.)/(GA+BE-1.),0.,1)-
     &  PYHISPEN(GA/(GA+BE-1.),0.,1)+    
     &  PYHISPEN(GA/(GA-BE),0.,1)-PYHISPEN((GA-1.)/(GA-BE),0.,1)+   
     &  LOG(GA/(1.-GA))*LOG((GA+BE-1.)/(BE-GA)) 
        F3IM=-PARU(1)*LOG((GA+BE-1.)/(BE-GA))   
      ELSE  
        RSQ=EPS/(EPS-1.+(2.*BE-1.)**2)  
        RCTHE=RSQ*(1.-2.*BE/EPS)    
        RSTHE=SQRT(RSQ-RCTHE**2)    
        RCPHI=RSQ*(1.+2.*(BE-1.)/EPS)   
        RSPHI=SQRT(RSQ-RCPHI**2)    
        R=SQRT(RSQ) 
        THE=ACOS(RCTHE/R)   
        PHI=ACOS(RCPHI/R)   
        F3RE=PYHISPEN(RCTHE,RSTHE,1)+PYHISPEN(RCTHE,-RSTHE,1)-  
     &  PYHISPEN(RCPHI,RSPHI,1)-PYHISPEN(RCPHI,-RSPHI,1)+   
     &  (PHI-THE)*(PHI+THE-PARU(1)) 
        F3IM=PYHISPEN(RCTHE,RSTHE,2)+PYHISPEN(RCTHE,-RSTHE,2)-  
     &  PYHISPEN(RCPHI,RSPHI,2)-PYHISPEN(RCPHI,-RSPHI,2)    
      ENDIF 
    
      IF(IREIM.EQ.1) PYHII3AU=2./(2.*BE-1.)*F3RE  
      IF(IREIM.EQ.2) PYHII3AU=2./(2.*BE-1.)*F3IM  
    
      RETURN    
      END   
