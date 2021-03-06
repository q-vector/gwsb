#include <getopt.h>
#include <iostream>
#include <denise/gtkmm.h>
#include "nine2five.h"
#include "predictor.h"

using namespace std;
using namespace denise;
using namespace nine2five;

int
main (int argc,
      char** argv)
{

   static struct option long_options[] =
   {
      { "command-line",               0, 0, 'c' },
      { "gradient-wind",              1, 0, 'G' },
      { "geometry",                   1, 0, 'g' },
      { "speed-label-tuple",          1, 0, 'l' },
      { "number-of-directions",       1, 0, 'n' },
      { "Sequence",                   1, 0, 'S' },
      { "station",                    1, 0, 's' },
      { "thresholds-tuple",           1, 0, 't' },
      { "max-speed",                  1, 0, 'x' }
   };

   try
   {

      const string data_path = getenv ("NINE2FIVE_DATA");

      bool command_line = false;
      Tokens station_tokens;
      Size_2D size_2d (1000, 800);
      Integer number_of_directions = 16;
      Tuple threshold_tuple ("2:5:10:15:20:25:30:35");
      Tuple speed_label_tuple ("10:20:30");
      Real max_speed = GSL_NAN;
      Wind gradient_wind (GSL_NAN, GSL_NAN);
      Real gradient_wind_threshold = GSL_NAN;
      Dstring sequence_dir_path ("");

      int c;
      int option_index = 0;
      char optstring[] = "cG:g:l:n:S:s:t:x:";

      while ((c = getopt_long (argc, argv, optstring,
             long_options, &option_index)) != -1)
      {

         switch (c)
         {

            case 'c':
            {
               command_line = true;
               break;
            }

            case 'G':
            {
               const Tokens tokens (Dstring (optarg), ":");
               const Integer n = tokens.size ();
               gradient_wind = Wind (tokens[0]);
               if (n > 1) { gradient_wind_threshold = stof (tokens[1]); }
               break;
            }

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

            case 'S':
            {
               sequence_dir_path = Dstring (optarg);
               break;
            }

            case 's':
            {
               for (const Dstring& token : Tokens (Dstring (optarg), ":"))
               {
                  station_tokens.push_back (token);
               }
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

      const Real size = size_2d.j / 2.4;
      const Point_2D origin (size_2d.i * 0.5, size_2d.j * 0.5);
      Data data (data_path, station_tokens);
      Wind_Disc wind_disc (number_of_directions, threshold_tuple,
         origin, size * 0.2, speed_label_tuple, max_speed);

      if (sequence_dir_path != "")
      {

         const Predictor::Sequence::Map sequence_map (sequence_dir_path);

         Gtk::Main gtk_main (argc, argv);

         Gtk::Window* window_ptr = new Gtk::Window ();
         Gtk::Window& window = *window_ptr;
         window.set_resizable (false);
         window.resize (size_2d.i, size_2d.j);
         window.set_title ("nine2five");

         Nine2five nine2five (window_ptr, size_2d,
            sequence_map, data, wind_disc);

         window.add (nine2five);
         nine2five.show ();

         Gtk::Main::run (window);

      }
      else
      if (command_line)
      {
      }
      else
      {
/*
         Gtk::Main gtk_main (argc, argv);

         Gtk::Window* window_ptr = new Gtk::Window ();
         Gtk::Window& window = *window_ptr;
         window.set_resizable (false);
         window.resize (size_2d.i, size_2d.j);
         window.set_title ("nine2five");

         Nine2five_Free nine2five_free (window_ptr, size_2d,
            station_tokens, data, wind_disc);

         window.add (nine2five_free);
         nine2five_free.show ();

         Gtk::Main::run (window);
*/
      }

   }
   catch (const Exception& e)
   {
      cerr << e << endl;
   }

}

