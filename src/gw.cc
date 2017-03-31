#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Nwp_Gw::Nwp_Gw (const Dstring& station,
                const Dtime& base_time,
                const Real forecast_hour,
                const Wind& wind)
   : Wind (wind),
     station (station),
     base_time (base_time),
     forecast_hour (forecast_hour)
{
}

Dtime
Nwp_Gw::get_dtime () const
{
   return Dtime (base_time.t + forecast_hour);
}

bool
Nwp_Gw::operator == (const Nwp_Gw& nwp_gw) const
{
   return (get_dtime () == nwp_gw.get_dtime ());
}

bool
Nwp_Gw::operator > (const Nwp_Gw& nwp_gw) const
{
   return (get_dtime () > nwp_gw.get_dtime ());
}

bool
Nwp_Gw::operator < (const Nwp_Gw& nwp_gw) const
{
   return (get_dtime () < nwp_gw.get_dtime ());
}

vector<Dtime>
Nwp_Gw::Sequence::get_time_vector () const
{

   vector<Dtime> time_vector;

   for (const Nwp_Gw& nwp_gw : *this)
   {
      const Dtime& dtime = nwp_gw.get_dtime ();
      time_vector.push_back (dtime);
   }

   return time_vector;

}

void
Nwp_Gw::Sequence::run (Data& data,
                       const Size_2D& size_2d,
                       const Wind_Disc& wind_disc) const
{

   const bool outline = false;
   const bool with_noise = true;
   const Real gradient_wind_threshold = 5;

   for (const Nwp_Gw& nwp_gw : *this)
   {

      const Dstring& station = nwp_gw.station;
      const Dtime& base_time = nwp_gw.base_time;
      const Real forecast_hour = nwp_gw.forecast_hour;
      const Dtime dtime (base_time.t + forecast_hour);
      const Integer month = dtime.get_month ();
      const Integer hour = dtime.get_hour ();

      set<Integer> month_set, hour_set;
      month_set.insert (month);
      hour_set.insert (hour);

      const Record::Set* record_set_ptr =
         data.get_station_data (station).get_record_set_ptr (
            month_set, hour_set, nwp_gw, gradient_wind_threshold);

      const Dstring png_file_path (station + "_" + dtime.get_string () + ".png");

      RefPtr<Surface> surface = denise::get_surface (
         size_2d, "png", png_file_path);
      RefPtr<Context> cr = denise::get_cr (surface);

      Color::white ().cairo (cr);
      cr->paint ();

      wind_disc.render_bg (cr);
      record_set_ptr->render_scatter_plot (cr, wind_disc, 5);

      //const Box_2D viewport (Index_2D (10, 50), Size_2D (980, 740));
      //Gwsb::render (cr, wind_disc, *record_set_ptr,
      //   gradient_wind_index_set, viewport, outline, with_noise);

      delete record_set_ptr;

      surface->write_to_png (png_file_path);

   }

}

Nwp_Gw::Sequence::Map::Map (const Dstring& dir_path)
{

   const Reg_Exp re ("[A-Z]...*.gw");
   const Tokens& dir_listing = get_dir_listing (dir_path, re, true);

   for (const Dstring& file_path : dir_listing)
   {
      cout << file_path << endl;
   }

}

void
Nwp_Gw::Sequence::Map::ingest (const Dstring& sequence_file_path)
{

   Dstring station;
   Nwp_Gw::Sequence sequence;
   igzstream file (sequence_file_path.get_string ());

   for (string il; std::getline (file, il); )
   {

      const Dstring input_line (il);
      if (input_line[0] == '"') { continue; }

      const Tokens tokens (input_line, ",");
      const Real forecast_hour = Real (stoi (tokens[0]));
      const Integer yyyy = stoi (tokens[1].substr (6, 4));
      const Integer mm = stoi (tokens[1].substr (3, 2));
      const Integer dd = stoi (tokens[1].substr (0, 2));
      const Integer hh = stoi (tokens[1].substr (11, 2));
      const Dtime base_time (yyyy, mm, dd, hh);

      station = tokens[2];
      const Wind gw (stof (tokens[3]), stof (tokens[4]));

      sequence.insert (Nwp_Gw (station, base_time, forecast_hour, gw));

   }

   file.close ();

   keys.push_back (station);
   insert (make_pair (station, sequence));

}

