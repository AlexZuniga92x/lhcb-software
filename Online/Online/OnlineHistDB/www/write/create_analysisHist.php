<HTML>
 <HEAD>
<LINK REL=STYLESHEET TYPE="text/css" HREF="../styles_screen.css">
 </HEAD>
<body>
<?php 
include '../util.php';
$conn=HistDBconnect(1);
$nin=$_POST["nin"];
$np=$_POST["np"];
$mypars=array();
if ($gs && $_POST["SOURCESET"]) {
  $sourceh="theSet=>".$_POST["SOURCESET"].",";
}
else {
  $sourceh="theSet=>0,";
}

$sh=array();
$sss=0;
for ($ih=1;$ih<=$nin;$ih++) {
  if($_POST["SOURCEH${ih}"] > 0) {
    $sss=1;
    $sh[$ih]=SingleHist($_POST["SOURCEH${ih}"]);
  }
}
if( $sss ) {
  $sourceh .= "theSources=>OnlineHistDB.sourceh('".implode("','",$sh)."')";
}
else {
  $sourceh .= "theSources=>OnlineHistDB.sourceh()";
}
for ($ip=1;$ip<=$np;$ip++) {
 $mypars[$ip]=$_POST["a1_p${ip}"];
}
$parameters="thresholds(".implode(",",$mypars).")";
$command="begin OnlineHistDB.DeclareAnalysisHistogram(theAlg=>'".$_POST["alg"].
  "',theTitle=>'".sqlstring($_POST["hctitle"])."',${sourceh},thePars=>${parameters},".
  "theName=>:name); end;";
//echo "command is $command <br>";
$stid = OCIParse($conn,$command);
ocibindbyname($stid,":name",$name,130);
$r=OCIExecute($stid,OCI_DEFAULT);
if ($r) {
  ocicommit($conn);
  ocifreestatement($stid);
  $stid = OCIParse($conn,"select HID from histogram where NAME='${name}'");
  $r=OCIExecute($stid,OCI_DEFAULT);
  if ($r) $r=OCIFetchInto($stid, $h, OCI_ASSOC);
} 
if (!$r)
  echo "<font color=red> <B>Got errors from create_AnalysisHist.php </B></font><br>Writing aborted<br>Check that your input Histograms exist and that the histogram name is not already in use<br>\n";
else {
  echo "Analysis Histogram created successfully<br>";
  echo "<a href=../Histogram.php?hid=".$h["HID"]."> Go to its record </a>";
}
ocifreestatement($stid);
ocilogoff($conn);
?>
</body></html>
