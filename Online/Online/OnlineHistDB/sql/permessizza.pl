#!/usr/bin/perl
# automatically create sql macros for setting permissions
# to be used by HIST_ADMIN 
open PERM,">permissions.sql";
open SYNO,">synonyms.sql";

print PERM "
grant create SYNONYM to HIST_READER;
grant EXECUTE on ONLINEHISTDB  to HIST_READER;
grant create SYNONYM to HIST_WRITER;
grant EXECUTE on ONLINEHISTDB  to HIST_WRITER;
grant EXECUTE on INTLIST to HIST_WRITER;
grant EXECUTE on ANALIST to HIST_WRITER;
grant EXECUTE on PARAMETERS to HIST_WRITER;
grant EXECUTE on THRESHOLDS to HIST_WRITER;
grant EXECUTE on INTLIST to HIST_READER;
grant EXECUTE on ANALIST to HIST_READER;
grant EXECUTE on PARAMETERS to HIST_READER;
grant EXECUTE on THRESHOLDS to HIST_READER;
";

print SYNO "
CREATE OR REPLACE SYNONYM ONLINEHISTDB FOR HIST_ADMIN.ONLINEHISTDB;
CREATE OR REPLACE SYNONYM INTLIST FOR HIST_ADMIN.INTLIST;
CREATE OR REPLACE SYNONYM ANALIST FOR HIST_ADMIN.ANALIST;
CREATE OR REPLACE SYNONYM PARAMETERS FOR HIST_ADMIN.PARAMETERS;
CREATE OR REPLACE SYNONYM THRESHOLDS FOR HIST_ADMIN.THRESHOLDS;\n";

for (`cat tables.sql`) {
  $name=0;
  if (/create +table +(\w+)/i) {
    $name=$1;
  }
  if (/create +(or +replace|) *view +(\w+)/i) {
    $name=$2;
  }
  if ($name) {
    print SYNO "CREATE OR REPLACE SYNONYM ${name} FOR HIST_ADMIN.${name};\n";
    print PERM "GRANT SELECT ON ${name} TO HIST_READER;\n";
    print PERM "GRANT SELECT ON ${name} TO HIST_WRITER;\n";
    print PERM "GRANT UPDATE ON ${name} TO HIST_WRITER;\n";
    print PERM "GRANT INSERT ON ${name} TO HIST_WRITER;\n";
    print PERM "GRANT DELETE ON ${name} TO HIST_WRITER;\n";
  }
}

exit;
