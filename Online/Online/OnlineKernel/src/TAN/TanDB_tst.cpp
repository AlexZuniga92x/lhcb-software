#include <cstdio>
#include "RTL/rtl.h"
#include "TAN/TanDB.h"
#define LINE(ll,x)   { \
  for ( int i=0; i < ll; i++ ) putchar(x);    \
  putchar('\n');   \
}

extern "C" int rtl_tandb_test( int /* argc */, char** /*argv */ )  {
  char buff[32];
  ::printf("Sizees: TANDB_ENTRY=%ld TanMessage=%ld Bytes\n",
    long(sizeof(TANDB_ENTRY)),long(sizeof(TanMessage)));

  // Start with clean pubarea...
  TanDataBase& db = TanDataBase::Instance();    

  LINE(80,'=');
  db.Dump( stdout );
  LINE(80,'=');

  TANDB_ENTRY* entry[128];
#ifndef _OSK
  int num_task = 22, num_loop = 10;
#else
  int num_task = 4, num_loop = 10;
#endif
  for ( int i = 0; i < num_task; i++ )  {
    // Insert entry
    TANDB_ENTRY* e = entry[i] = db.AllocateEntry(i);
    sprintf(e->_Message()._Name(),"MYTASK_%02d",i);
    db.allocatePort (e);

    // Insert alias(s)
    for ( int j = 0; j < i; j++ )  {
      sprintf(e->_Message()._Name(),"MYTASK_%02d_%02d",i,j);
      db.insertAlias (e);
    }
  }
  // Dump contents
  LINE(80,'=');
  db.Dump( stdout );
  LINE(80,'=');

#ifdef _VMS
  db.Close( entry[8] );

  db.Dump( stdout );
  LINE(80,'=');

  db.Close( entry[9] );
  db.Close( entry[7] );
  db.Close( entry[6] );
  db.Close( entry[5] );
  db.Close( entry[4] );
#endif
  db.Close( entry[3] );
  db.Dump( stdout );
  LINE(80,'=');
  db.Close( entry[2] );
  db.Dump( stdout );
  LINE(80,'=');
  db.Close( entry[1] );
  db.Dump( stdout );
  LINE(80,'=');
  db.Close( entry[0] );
  db.Dump( stdout );
  LINE(80,'=');

  LINE(80,'-');
  printf("   Create and delete %d x %d tasks with 0...%d aliases\n",
    num_loop,num_task,num_task-1);
  printf(" ... including to retrieve inserted port entries by task/alias name\n");
  TanMessage msg;
  for ( int k = 0; k < num_loop; k++ )  {
    for ( int i = 0; i < num_task; i++ )  {
      // Insert entry
      TANDB_ENTRY* e = entry[i] = db.AllocateEntry(i);
      sprintf(e->_Message()._Name(),"MYTASK_%02d",i);
      strcpy(msg._Name(),e->_Message()._Name());
      NetworkChannel::Port port = db.allocatePort (e);
      NetworkChannel::Port fnd  = db.findPort(msg);
      if ( fnd != port ) {
        printf("!!!!!!!!!!! Port allocated:%X found:%X\n",port,fnd);
      }

      // Insert alias(s)
      for ( int j = 0; j < i; j++ )  {
        sprintf(e->_Message()._Name(),"MYTASK_%02d_%02d",i,j);
        strcpy(msg._Name(),e->_Message()._Name());
        int status = db.insertAlias (e);
        NetworkChannel::Port fnd  = db.findPort(msg);
        if ( fnd != port ) {
          printf("!!!!!!!!!!! Port allocated:%X found:%X status:%X\n",port,fnd,status);
        }
      }
    }
    //      fprintf(stdout,"Type return to delete tasks:");
    //      fflush(stdout);
    //      scanf("%c",&buff[0]);
    for ( int i = 0; i < num_task; i++ )    {
      sprintf(buff,"MYTASK_%02d",i);
      TANDB_ENTRY* e = db.FindEntry ( buff );
      db.Close( e );
    }
  }
  LINE(80,'-');
  LINE(80,'=');
  LINE(80,'-');
  db.Dump( stdout );
  LINE(80,'=');
  printf("All done ....\n");
  return lib_rtl_default_return();
}
