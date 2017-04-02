#include <getopt.h>
#include <iostream>
#include <denise/gtkmm.h>
#include "gwsb.h"
#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

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

      const string data_path = getenv ("GWSB_DATA");

      bool command_line = false;
      string station_string;
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
               if (station_string.size () != 0) { station_string += ":"; }
               station_string += Dstring (optarg);
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

      Data data (data_path, station_string);
      Wind_Disc wind_disc (number_of_directions, threshold_tuple,
         Point_2D (size_2d.i / 2, size_2d.j / 2), size_2d.j / 2.4,
         speed_label_tuple, max_speed);

      if (sequence_dir_path != "")
      {

         const Nwp_Gw::Sequence::Map sequence_map (sequence_dir_path);

         Gtk::Main gtk_main (argc, argv);

         Gtk::Window* window_ptr = new Gtk::Window ();
         Gtk::Window& window = *window_ptr;
         window.set_resizable (false);
         window.resize (size_2d.i, size_2d.j);
         window.set_title ("gwsb");

         Gwsb gwsb (window_ptr, size_2d, sequence_map, data, wind_disc);

         window.add (gwsb);
         gwsb.show ();

         Gtk::Main::run (window);

      }
      else
      if (command_line)
      {

/*
         const bool outline = false;
         const bool with_noise = true;

         const Dstring station = station_string;
         Station_Data& station_data = data.get_station_data (station);

         set<Integer> month_set;
         month_set.insert (2);
         set<Integer> hour_set;
         hour_set.insert (6);

         const Record::Set* record_set_ptr =
            station_data.get_record_set_ptr (month_set, hour_set,
               gradient_wind, gradient_wind_threshold);

         RefPtr<Surface> surface = denise::get_surface (
            size_2d, "png", "./gwsb.png");
         RefPtr<Context> cr = denise::get_cr (surface);

         Color::white ().cairo (cr);
         cr->paint ();

         wind_disc.render_bg (cr);
         record_set_ptr->render_scatter_plot (cr, wind_disc, 5);

         //const Box_2D viewport (Index_2D (10, 50), Size_2D (980, 740));
         //Gwsb::render (cr, wind_disc, *record_set_ptr,
         //   gradient_wind_index_set, viewport, outline, with_noise);

         delete record_set_ptr;

         surface->write_to_png ("./gwsb.png");
*/

      }
      else
      {

/*
         Gtk::Main gtk_main (argc, argv);

         Gtk::Window* window_ptr = new Gtk::Window ();
         Gtk::Window& window = *window_ptr;
         window.set_resizable (false);
         window.resize (size_2d.i, size_2d.j);
         window.set_title ("gwsb");

         Gwsb_Free gwsb_free (window_ptr, size_2d, data, wind_disc);

         window.add (gwsb_free);
         gwsb_free.show ();

         Gtk::Main::run (window);
*/

      }

   }
   catch (const Exception& e)
   {
      cerr << e << endl;
   }

}

