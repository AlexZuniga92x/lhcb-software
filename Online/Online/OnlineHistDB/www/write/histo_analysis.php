<HTML>
 <HEAD>
<LINK REL=STYLESHEET TYPE="text/css" HREF="../styles_screen.css">
 </HEAD>
<body>
<?php 
include '../util.php'; include '../dbforms.php'; 
$conn=HistDBconnect();
if (!$conn) {
  $e = ocierror();
  print htmlentities($e['message']);
  exit;
}

function update_histo_analysis() {
  global $conn,$id;
  $ia=$_POST["Iana"];
  $aid=$_POST["a${ia}_id"];
  $warn=$alr=$inps=array();
  $out=0;
  for ($ip=1;$ip<=$_POST["a${ia}_np"];$ip++) {
    $warn[$ip]=$_POST["a${ia}_p${ip}_w"];
    $alr[$ip]=$_POST["a${ia}_p${ip}_a"];
  }
  for ($ip=1;$ip<=$_POST["a${ia}_ni"];$ip++) {
    $inps[$ip]=$_POST["a${ia}_i${ip}_v"];
  }
  $warnings="thresholds(".implode(",",$warn).")";
  $alarms="thresholds(".implode(",",$alr).")";
  $inputs= "thresholds(".implode(",",$inps).")";

  if (! $aid)  // new entry
    $command="begin :out := OnlineHistDB.DeclareAnalysis(".$_POST["id"].",'".$_POST["a${ia}_alg"]."',$warnings,$alarms,999,$inputs); end;";
  else 
    $command= "update ANASETTINGS set WARNINGS=$warnings,ALARMS=$alarms,INPUTPARS=$inputs where ANA=$aid and ".
      ( $_POST["htype"] == "HID" ? "HISTO='$id'" : "REGEXP_REPLACE(HISTO,'^(.*)/.*\$','\\1')=$id");
  
  //echo "command is $command<br>\n";
  $stid = OCIParse($conn,$command);
  if (! $aid)
    ocibindbyname($stid,":out",$out,10);
  $r=OCIExecute($stid,OCI_DEFAULT);
  if (!$aid && !$out) return 0;
  ocicommit($conn);
  ocifreestatement($stid);
  return 1;  
}

function mask_histo_analysis($action) {
  global $conn,$id;
  $newmask= ($action=="Unmask") ? 0 : 1;
  $ia=$_POST["Iana"];
  $aid=$_POST["a${ia}_id"];
  $command= "update ANASETTINGS set MASK=$newmask where ANA=$aid and ".
      ( $_POST["htype"] == "HID" ? "HISTO='$id'" : "REGEXP_REPLACE(HISTO,'^(.*)/.*\$','\\1')=$id");

  $stid = OCIParse($conn,$command);
  $r=OCIExecute($stid,OCI_DEFAULT);
  if (!$r) return 0;
  ocicommit($conn);
  ocifreestatement($stid);   
  return 1;  
}

function remove_histo_analysis($aid,$hset) {
  global $conn;
  $stid = OCIParse($conn,"delete from ANALYSIS where AID=$aid and HSET=$hset");
  $r=OCIExecute($stid,OCI_DEFAULT);
  if (!$r) return 0;
  ocifreestatement($stid);
  $stid = OCIParse($conn,"UPDATE HISTOGRAMSET set NANALYSIS=NANALYSIS-1 where HSID=$hset");
  $r=OCIExecute($stid,OCI_DEFAULT);
  if (!$r) return 0;
  ocicommit($conn);
  ocifreestatement($stid);
  return 1;  
}

$id=$_POST["id"];
$ia=$_POST["Iana"];
?>
<H2 ALIGN="CENTER">Add/Update Automatic Analysis for histogram <?php echo $id ?></H2><hr>

<?php

if (array_key_exists("New",$_POST)) {
  histo_analysis($id,$_POST["htype"],"new");
}
else if (array_key_exists("SetMask_Analysis",$_POST)) {
  if (mask_histo_analysis($_POST["SetMask_Analysis"]))
    echo "Histogram Analysis ".$_POST["a${ia}_alg"]." ".$_POST["SetMask_Analysis"]."ed successfully<br><br>\n";
  else
    echo "<font color=red> <B>Got errors from mask_histo_analysis() </B></font><br><br>\n";
}
else if (array_key_exists("Remove_Analysis",$_POST)) {
  if ($_POST["htype"] == 'HSID') { // be sure command comes from a set
    echo "If you confirm, Analysis ".$_POST["a${ia}_alg"]." will be removed and settings of parameters will be lost<br>\n";
    if($_POST["NHS"]>1)
      echo "<B> for all the ".$_POST["NHS"]." histograms of set<br>\n";
    echo "<form action='$_SERVER[PHP_SELF]' method='post'>\n";
    echo "<input type='hidden' name='id' value='${id}'>\n";
    echo "<input type='hidden' name='htype' value='".$_POST["htype"],"'>\n";    
    echo "<input type='hidden' name='algo' value='".$_POST["a${ia}_alg"],"'>\n";
    echo "<input type='hidden' name='aid' value='".$_POST["a${ia}_id"],"'>\n";
    echo "<input type='submit' name='Really_Remove_Analysis' value='Confirm Removal of Analysis'>\n";
  }
}
else if (array_key_exists("Really_Remove_Analysis",$_POST)) {
  if (remove_histo_analysis($_POST["aid"],$id)) 
    echo "Histogram Analysis ".$_POST["algo"]." deleted successfully<br><br>\n";
  else
    echo "<font color=red> <B>Got errors from remove_histo_analysis() </B></font><br><br>\n";
}
else if ($_POST["Update_analysis"] == 'Confirm') {
  if (update_histo_analysis())
    echo "Histogram Analysis updated successfully<br><br>\n";
  else
    echo "<font color=red> <B>Got errors from update_histo_analysis() </B></font><br><br>\n";
} 
else if ($_POST["Update_analysis"] == 'Set Parameters') {
  histo_analysis($id,$_POST["htype"],"newana");
}
else {
  echo "Please check your data and confirm <br><br>";
  histo_analysis($id,$_POST["htype"],"update");
}

ocilogoff($conn);
echo "<br> <a href='../Histogram.php?".strtolower($_POST["htype"])."=${id}> Back to Histogram Record $id </a><br>";
?>

</body>
</html>
