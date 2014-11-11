if ( $?CMTROOT == 0 ) then
  setenv CMTROOT /afs/cern.ch/sw/contrib/CMT/v1r20p20090520
endif
source ${CMTROOT}/mgr/setup.csh
set tempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt cleanup -csh -pack=B2DDphi -version=v1r0 -path=/afs/cern.ch/user/m/mamartin/cmtuser/Urania_v3r0/Phys $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}

