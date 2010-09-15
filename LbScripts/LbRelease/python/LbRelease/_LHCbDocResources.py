"""
Module to store resources (e.g. files) used by LHCbDoc.
"""

################################################################################
## DoxygenLayout.xml (valid at least for 1.5.9 <= Doxygen <= 1.7.0)
layout = """<doxygenlayout version="1.0">
  <!-- Navigation index tabs for HTML output -->
  <navindex>
    <tab type="mainpage" visible="yes" title=""/>
    <tab type="pages" visible="yes" title=""/>
    <tab type="modules" visible="yes" title=""/>
    <tab type="namespaces" visible="yes" title="">
      <tab type="namespaces" visible="yes" title=""/>
      <tab type="namespacemembers" visible="yes" title=""/>
    </tab>
    <tab type="classindex" visible="yes" title="Classes">
      <tab type="classindex" visible="$ALPHABETICAL_INDEX" title=""/>
      <tab type="classes" visible="yes" title=""/>
      <tab type="hierarchy" visible="yes" title=""/>
      <tab type="classmembers" visible="yes" title=""/>
    </tab>
    <tab type="files" visible="yes" title="">
      <tab type="files" visible="yes" title=""/>
      <tab type="globals" visible="yes" title=""/>
    </tab>
    <tab type="dirs" visible="yes" title=""/>
    <tab type="examples" visible="yes" title=""/>
  </navindex>

  <!-- Layout definition for a class page -->
  <class>
    <briefdescription visible="yes"/>
    <includes visible="$SHOW_INCLUDE_FILES"/>
    <inheritancegraph visible="$CLASS_GRAPH"/>
    <collaborationgraph visible="$COLLABORATION_GRAPH"/>
    <allmemberslink visible="yes"/>
    <memberdecl>
      <nestedclasses visible="yes" title=""/>
      <publictypes title=""/>
      <publicslots title=""/>
      <signals title=""/>
      <publicmethods title=""/>
      <publicstaticmethods title=""/>
      <publicattributes title=""/>
      <publicstaticattributes title=""/>
      <protectedtypes title=""/>
      <protectedslots title=""/>
      <protectedmethods title=""/>
      <protectedstaticmethods title=""/>
      <protectedattributes title=""/>
      <protectedstaticattributes title=""/>
      <packagetypes title=""/>
      <packagemethods title=""/>
      <packagestaticmethods title=""/>
      <packageattributes title=""/>
      <packagestaticattributes title=""/>
      <properties title=""/>
      <events title=""/>
      <privatetypes title=""/>
      <privateslots title=""/>
      <privatemethods title=""/>
      <privatestaticmethods title=""/>
      <privateattributes title=""/>
      <privatestaticattributes title=""/>
      <friends title=""/>
      <related title="" subtitle=""/>
      <membergroups visible="yes"/>
    </memberdecl>
    <detaileddescription title=""/>
    <memberdef>
      <typedefs title=""/>
      <enums title=""/>
      <constructors title=""/>
      <functions title=""/>
      <related title=""/>
      <variables title=""/>
      <properties title=""/>
      <events title=""/>
    </memberdef>
    <usedfiles visible="$SHOW_USED_FILES"/>
    <authorsection visible="yes"/>
  </class>

  <!-- Layout definition for a namespace page -->
  <namespace>
    <briefdescription visible="yes"/>
    <memberdecl>
      <nestednamespaces visible="yes" title=""/>
      <classes visible="yes" title=""/>
      <typedefs title=""/>
      <enums title=""/>
      <functions title=""/>
      <variables title=""/>
      <membergroups visible="yes"/>
    </memberdecl>
    <detaileddescription title=""/>
    <memberdef>
      <typedefs title=""/>
      <enums title=""/>
      <functions title=""/>
      <variables title=""/>
    </memberdef>
    <authorsection visible="yes"/>
  </namespace>

  <!-- Layout definition for a file page -->
  <file>
    <briefdescription visible="yes"/>
    <includes visible="$SHOW_INCLUDE_FILES"/>
    <includegraph visible="$INCLUDE_GRAPH"/>
    <includedbygraph visible="$INCLUDED_BY_GRAPH"/>
    <sourcelink visible="yes"/>
    <memberdecl>
      <classes visible="yes" title=""/>
      <namespaces visible="yes" title=""/>
      <defines title=""/>
      <typedefs title=""/>
      <enums title=""/>
      <functions title=""/>
      <variables title=""/>
      <membergroups visible="yes"/>
    </memberdecl>
    <detaileddescription title=""/>
    <memberdef>
      <defines title=""/>
      <typedefs title=""/>
      <enums title=""/>
      <functions title=""/>
      <variables title=""/>
    </memberdef>
    <authorsection/>
  </file>

  <!-- Layout definition for a group page -->
  <group>
    <briefdescription visible="yes"/>
    <groupgraph visible="$GROUP_GRAPHS"/>
    <memberdecl>
      <classes visible="yes" title=""/>
      <namespaces visible="yes" title=""/>
      <dirs visible="yes" title=""/>
      <nestedgroups visible="yes" title=""/>
      <files visible="yes" title=""/>
      <defines title=""/>
      <typedefs title=""/>
      <enums title=""/>
      <enumvalues title=""/>
      <functions title=""/>
      <variables title=""/>
      <signals title=""/>
      <publicslots title=""/>
      <protectedslots title=""/>
      <privateslots title=""/>
      <events title=""/>
      <properties title=""/>
      <friends title=""/>
      <membergroups visible="yes"/>
    </memberdecl>
    <detaileddescription title=""/>
    <memberdef>
      <pagedocs/>
      <defines title=""/>
      <typedefs title=""/>
      <enums title=""/>
      <enumvalues title=""/>
      <functions title=""/>
      <variables title=""/>
      <signals title=""/>
      <publicslots title=""/>
      <protectedslots title=""/>
      <privateslots title=""/>
      <events title=""/>
      <properties title=""/>
      <friends title=""/>
    </memberdef>
    <authorsection visible="yes"/>
  </group>

  <!-- Layout definition for a directory page -->
  <directory>
    <briefdescription visible="yes"/>
    <directorygraph visible="yes"/>
    <memberdecl>
      <dirs visible="yes"/>
      <files visible="yes"/>
    </memberdecl>
    <detaileddescription title=""/>
  </directory>
</doxygenlayout>
"""

################################################################################
stylesheet = """/* LHCb CSS for doxygen, based on the standard one */

body, table, div, p, dl {
    font-family: Lucida Grande, Verdana, Geneva, Arial, sans-serif;
    font-size: 12px;
}

/* @group Heading Levels */

h1 {
    text-align: center;
    font-size: 150%;
}

h2 {
    font-size: 120%;
}

h3 {
    font-size: 100%;
}

dt {
    font-weight: bold;
}

div.multicol {
    -moz-column-gap: 1em;
    -webkit-column-gap: 1em;
    -moz-column-count: 3;
    -webkit-column-count: 3;
}

p.startli, p.startdd, p.starttd {
    margin-top: 2px;
}

p.endli {
    margin-bottom: 0px;
}

p.enddd {
    margin-bottom: 4px;
}

p.endtd {
    margin-bottom: 2px;
}

/* @end */

caption {
    font-weight: bold;
}

span.legend {
        font-size: 70%;
        text-align: center;
}

h3.version {
        font-size: 90%;
        text-align: center;
}

div.qindex, div.navtab{
    background-color: #e8eef2;
    border: 1px solid #84b0c7;
    text-align: center;
    margin: 2px;
    padding: 2px;
}

div.qindex, div.navpath {
    width: 100%;
    line-height: 140%;
}

div.navtab {
    margin-right: 15px;
}

/* @group Link Styling */

a {
    color: #153788;
    font-weight: normal;
    text-decoration: none;
}

.contents a:visited {
    color: #1b77c5;
}

a:hover {
    text-decoration: underline;
}

a.qindex {
    font-weight: bold;
}

a.qindexHL {
    font-weight: bold;
    background-color: #6666cc;
    color: #ffffff;
    border: 1px double #9295C2;
}

.contents a.qindexHL:visited {
        color: #ffffff;
}

a.el {
    font-weight: bold;
}

a.elRef {
}

a.code {
    color: #3030f0;
}

a.codeRef {
    color: #3030f0;
}

/* @end */

dl.el {
    margin-left: -1cm;
}

.fragment {
    font-family: monospace, fixed;
    font-size: 105%;
}

pre.fragment {
    border: 1px solid #CCCCCC;
    background-color: #f5f5f5;
    padding: 4px 6px;
    margin: 4px 8px 4px 2px;
    overflow: auto;
    word-wrap: break-word;
    font-size:  9pt;
    line-height: 125%;
}

div.ah {
    background-color: black;
    font-weight: bold;
    color: #ffffff;
    margin-bottom: 3px;
    margin-top: 3px
}

div.groupHeader {
    margin-left: 16px;
    margin-top: 12px;
    margin-bottom: 6px;
    font-weight: bold;
}

div.groupText {
    margin-left: 16px;
    font-style: italic;
}

body {
    background: white;
    color: black;
    margin-right: 20px;
    margin-left: 20px;
}

td.indexkey {
    background-color: #e8eef2;
    font-weight: bold;
    border: 1px solid #CCCCCC;
    margin: 2px 0px 2px 0;
    padding: 2px 10px;
}

td.indexvalue {
    background-color: #e8eef2;
    border: 1px solid #CCCCCC;
    padding: 2px 10px;
    margin: 2px 0px;
}

tr.memlist {
    background-color: #f0f0f0;
}

p.formulaDsp {
    text-align: center;
}

img.formulaDsp {

}

img.formulaInl {
    vertical-align: middle;
}

div.center {
    text-align: center;
        margin-top: 0px;
        margin-bottom: 0px;
        padding: 0px;
}

div.center img {
    border: 0px;
}

img.footer {
    border: 0px;
    vertical-align: middle;
}

/* @group Code Colorization */

span.keyword {
    color: #008000
}

span.keywordtype {
    color: #604020
}

span.keywordflow {
    color: #e08000
}

span.comment {
    color: #800000
}

span.preprocessor {
    color: #806020
}

span.stringliteral {
    color: #002080
}

span.charliteral {
    color: #008080
}

span.vhdldigit {
    color: #ff00ff
}

span.vhdlchar {
    color: #000000
}

span.vhdlkeyword {
    color: #700070
}

span.vhdllogic {
    color: #ff0000
}

/* @end */

.search {
    color: #003399;
    font-weight: bold;
}

form.search {
    margin-bottom: 0px;
    margin-top: 0px;
}

input.search {
    font-size: 75%;
    color: #000080;
    font-weight: normal;
    background-color: #e8eef2;
}

td.tiny {
    font-size: 75%;
}

.dirtab {
    padding: 4px;
    border-collapse: collapse;
    border: 1px solid #84b0c7;
}

th.dirtab {
    background: #e8eef2;
    font-weight: bold;
}

hr {
    height: 0;
    border: none;
    border-top: 1px solid #666;
}

/* @group Member Descriptions */

.mdescLeft, .mdescRight,
.memItemLeft, .memItemRight,
.memTemplItemLeft, .memTemplItemRight, .memTemplParams {
    background-color: #FAFAFA;
    border: none;
    margin: 4px;
    padding: 1px 0 0 8px;
}

.mdescLeft, .mdescRight {
    padding: 0px 8px 4px 8px;
    color: #555;
}

.memItemLeft, .memItemRight, .memTemplParams {
    border-top: 1px solid #ccc;
}

.memItemLeft, .memTemplItemLeft {
        white-space: nowrap;
}

.memTemplParams {
    color: #606060;
        white-space: nowrap;
}

/* @end */

/* @group Member Details */

/* Styles for detailed member documentation */

.memtemplate {
    font-size: 80%;
    color: #606060;
    font-weight: normal;
    margin-left: 3px;
}

.memnav {
    background-color: #e8eef2;
    border: 1px solid #84b0c7;
    text-align: center;
    margin: 2px;
    margin-right: 15px;
    padding: 2px;
}

.memitem {
    padding: 0;
    margin-bottom: 10px;
}

.memname {
    white-space: nowrap;
    font-weight: bold;
}

.memproto, .memdoc {
    border: 1px solid #84b0c7;
}

.memproto {
    padding: 0;
    background-color: #d5e1e8;
    font-weight: bold;
    -webkit-border-top-left-radius: 8px;
    -webkit-border-top-right-radius: 8px;
        -webkit-box-shadow: 5px 5px 5px rgba(0, 0, 0, 0.15);
    -moz-border-radius-topleft: 8px;
    -moz-border-radius-topright: 8px;
        -moz-box-shadow: rgba(0, 0, 0, 0.15) 5px 5px 5px;

}

.memdoc {
    padding: 2px 5px;
    background-color: #eef3f5;
    border-top-width: 0;
    -webkit-border-bottom-left-radius: 8px;
    -webkit-border-bottom-right-radius: 8px;
        -webkit-box-shadow: 5px 5px 5px rgba(0, 0, 0, 0.15);
    -moz-border-radius-bottomleft: 8px;
    -moz-border-radius-bottomright: 8px;
        -moz-box-shadow: rgba(0, 0, 0, 0.15) 5px 5px 5px;
}

.paramkey {
    text-align: right;
}

.paramtype {
    white-space: nowrap;
}

.paramname {
    color: #602020;
    white-space: nowrap;
}
.paramname em {
    font-style: normal;
}

/* @end */

/* @group Directory (tree) */

/* for the tree view */

.ftvtree {
    font-family: sans-serif;
    margin: 0.5em;
}

/* these are for tree view when used as main index */

.directory {
    font-size: 9pt;
    font-weight: bold;
}

.directory h3 {
    margin: 0px;
    margin-top: 1em;
    font-size: 11pt;
}

/*
The following two styles can be used to replace the root node title
with an image of your choice.  Simply uncomment the next two styles,
specify the name of your image and be sure to set 'height' to the
proper pixel height of your image.
*/

/*
.directory h3.swap {
    height: 61px;
    background-repeat: no-repeat;
    background-image: url("yourimage.gif");
}
.directory h3.swap span {
    display: none;
}
*/

.directory > h3 {
    margin-top: 0;
}

.directory p {
    margin: 0px;
    white-space: nowrap;
}

.directory div {
    display: none;
    margin: 0px;
}

.directory img {
    vertical-align: -30%;
}

/* these are for tree view when not used as main index */

.directory-alt {
    font-size: 100%;
    font-weight: bold;
}

.directory-alt h3 {
    margin: 0px;
    margin-top: 1em;
    font-size: 11pt;
}

.directory-alt > h3 {
    margin-top: 0;
}

.directory-alt p {
    margin: 0px;
    white-space: nowrap;
}

.directory-alt div {
    display: none;
    margin: 0px;
}

.directory-alt img {
    vertical-align: -30%;
}

/* @end */

address {
    font-style: normal;
    color: #333333;
}

table.doxtable {
    border-collapse:collapse;
}

table.doxtable td, table.doxtable th {
    border: 1px solid #153788;
    padding: 3px 7px 2px;
}

table.doxtable th {
    background-color: #254798;
    color: #FFFFFF;
    font-size: 110%;
    padding-bottom: 4px;
    padding-top: 5px;
    text-align:left;
}

/* LHCb-Specific */
p.subtitle {
    text-align: center;
    font-style: italic;
}
"""

################################################################################
class_php = """<?php
// Small php script to redirect to the reference page of a class.
// Usage:  class.php?n=MyClass
// If the class is found, it redirects to the reference page, otherwise
// to the search resuls.
// If the class is not specified, redirects to the classes index page.

// Open the database of classes.
$classes = dba_open("classes.db", "r-", "db4");
if (!$classes) {
  echo "Cannot open the database of classes.\n";
  exit;
}

// check if we have the parameter 'n'
if ($n) {
  // check if it is in the database
  if (dba_exists($n, $classes)) {
    // get the url
    $url = dba_fetch($n, $classes);
  } else {
    // not found: search for it
    $url = "search.php?query=" . urlencode($n);
  }
} else {
  // not given, go to all classes.
  $url = "classes.html";
}

// Close the database
dba_close($classes);

// Redirect
header("Location: ". $url);
?>
"""

################################################################################
