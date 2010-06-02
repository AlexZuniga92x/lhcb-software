if test "${CMTROOT}" = ""; then
  CMTROOT=/afs/cern.ch/sw/contrib/CMT/v1r20p20090520; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
tempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if test ! $? = 0 ; then tempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt cleanup -sh -pack=DaVinciTests -version=v1r0 -path=/afs/cern.ch/lhcb/group/davinci/vol1/jpalac/DaVinci_v25r5 $* >${tempfile}; . ${tempfile}
/bin/rm -f ${tempfile}

