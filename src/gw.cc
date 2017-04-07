#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Nwp_Gw::Nwp_Gw (const Dstring& station,
                const Dtime& base_time,
                const Real forecast_hour,
                const Wind& wind,
                const Real temperature)
   : Wind (wind),
     station (station),
     base_time (base_time),
     forecast_hour (forecast_hour),
     temperature (temperature)
{
}

Dtime
Nwp_Gw::get_time () const
{
   return Dtime (base_time.t + forecast_hour);
}

bool
Nwp_Gw::operator == (const Nwp_Gw& nwp_gw) const
{
   return (get_time () == nwp_gw.get_time ());
}

bool
Nwp_Gw::operator > (const Nwp_Gw& nwp_gw) const
{
   return (get_time () > nwp_gw.get_time ());
}

bool
Nwp_Gw::operator < (const Nwp_Gw& nwp_gw) const
{
   return (get_time () < nwp_gw.get_time ());
}

const set<Dtime>&
Nwp_Gw::Sequence::get_time_set () const
{
   return time_set;
}

void
Nwp_Gw::Sequence::ingest (const Nwp_Gw& nwp_gw)
{
   const Dtime& dtime = nwp_gw.get_time ();
   time_set.insert (dtime);
   map<Dtime, Nwp_Gw>::insert (make_pair (dtime, nwp_gw));
}

Nwp_Gw::Sequence::Map::Map (const Dstring& dir_path)
{
   const Reg_Exp re ("^[A-Za-z].*.gws$");
   const Tokens& dir_listing = get_dir_listing (dir_path, re, true);
   for (const Dstring& file_path : dir_listing) { ingest (file_path); }
}

void
Nwp_Gw::Sequence::Map::ingest (const Dstring& sequence_file_path)
{

   Dstring this_station, station;
   Nwp_Gw::Sequence sequence;
   igzstream file (sequence_file_path.get_string ());

   const Real multiplier = 0.5144444;

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
      const Real u = stof (tokens[3]) * multiplier;
      const Real v = stof (tokens[4]) * multiplier;
      const Wind gw (u, v);

      const Real t = stof (tokens[5]);

      if (station != this_station)
      {
         station_tokens.push_back (station);
         insert (make_pair (station, sequence));
         this_station = station;
      }
  
      at (station).ingest (Nwp_Gw (station, base_time, forecast_hour, gw, t));

   }

   file.close ();

}

const Tokens&
Nwp_Gw::Sequence::Map::get_station_tokens () const
{
   return station_tokens;
}


