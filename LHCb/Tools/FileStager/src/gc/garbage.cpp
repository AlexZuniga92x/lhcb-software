// stdlig
#include <csignal>
#include <iostream>
#include <string>

// boost
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
using boost::posix_time::seconds;

using namespace std ;


// global dirs
string tmpdir;

void cleanup( int )
{
   fs::path p( tmpdir );
   fs::remove_all( tmpdir );
   exit( 0 );
}

int main( int argc, char* argv[] )
{
   // Setup the command line option parser
   po::options_description config("Possible options");
   config.add_options()
      ( "help,h", "print help message" );

   po::options_description hidden( "Hidden options" );
   hidden.add_options()
      ( "pid",  po::value< pid_t >(), "pid to watch" )
      ( "tmpdir", po::value< string >(), "tempdir" );

   po::options_description cmdline_options;
   cmdline_options.add( config ).add( hidden );

   po::positional_options_description p;
   p.add( "pid", 1 );
   p.add( "tmpdir", 1 );

   // Parse the command line
   po::variables_map vm;
   store( po::command_line_parser( argc, argv ).
          options( cmdline_options ).positional( p ).run(), vm );
   notify( vm );

   if ( vm.count( "help" ) ) {
      cout << "usage: garbage.exe pid dir" << endl;
      cout << config << endl;
      return 0;
   }

   // Open the input file
   tmpdir = vm[ "tmpdir" ].as< string >();
   pid_t pID = vm[ "pid" ].as< pid_t >();

   // register the signals now, we no the tmpdir
   signal( SIGTERM, cleanup );
   signal( SIGKILL, cleanup );

   // poll for the main process 
   while (1) {
      if ( kill( pID, 0 ) == 0 ) {
         boost::this_thread::sleep( seconds( 10 ) );
      } else {
         break;
      }
   }

   cleanup( 0 );

   return 0;
}

