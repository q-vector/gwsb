#include <getopt.h>
#include <iostream>
#include <denise/gtkmm.h>
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

int
main (int argc,
      char** argv)
{

   static struct option long_options[] =
   {
      { "geometry",                   1, 0, 'g' },
      { "thresholds",                 1, 0, 't' }
   };

   try
   {

      const string data_path = getenv ("GWSB_DATA");

      string station_string;
      Size_2D size_2d (1000, 800);
      Integer number_of_directions = 16;
      Tuple threshold_tuple ("2:5:10:15:20:25:30:35");
      Tuple speed_label_tuple ("10:20:30");
      Real max_speed = GSL_NAN;

      int c;
      int option_index = 0;
      char optstring[] = "g:l:n:s:t:x:";

      while ((c = getopt_long (argc, argv, optstring,
             long_options, &option_index)) != -1)
      {

         switch (c)
         {

            case 'g':
            {
               const Tokens tokens (Dstring (optarg), "x");
               size_2d.i = stof (tokens[0]);
               size_2d.j = stof (tokens[1]);
               break;
            }

            case 'l':
            {
               speed_label_tuple = Tuple (Dstring (optarg));
               break;
            }

            case 'n':
            {
               number_of_directions = stoi (Dstring (optarg));
               break;
            }

            case 's':
            {
               station_string += ":" + Dstring (optarg);
               break;
            }

            case 't':
            {
               threshold_tuple = Tuple (Dstring (optarg));
               break;
            }

            case 'x':
            {
               max_speed = stoi (Dstring (optarg));
               break;
            }

            default:
            {
               cerr << "Error options " << c << endl;
               break;
            }

         }

      }

      Gtk::Main gtk_main (argc, argv);

      Gtk::Window* window_ptr = new Gtk::Window ();
      Gtk::Window& window = *window_ptr;
      window.set_resizable (false);
      window.resize (size_2d.i, size_2d.j);
      window.set_title ("gwsb");

      Gwsb gwsb (window_ptr, data_path, station_string, size_2d,
         ".", number_of_directions, threshold_tuple, speed_label_tuple,
         max_speed);

      window.add (gwsb);
      gwsb.show ();

      Gtk::Main::run (window);

   }
   catch (const Exception& e)
   {
      cerr << e << endl;
   }

}

