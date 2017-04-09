#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Predictor::Predictor (const Wind& wind_925,
                      const Real temperature_925)
   : wind_925 (wind_925),
     temperature_925 (temperature_925)
{
}

const set<Dtime>&
Predictor::Sequence::get_time_set () const
{
   return time_set;
}

void
Predictor::Sequence::ingest (const Dtime& dtime,
                             const Predictor& predictor)
{
   time_set.insert (dtime);
   map<Dtime, Predictor>::insert (make_pair (dtime, predictor));
}

Predictor::Sequence::Map::Map (const Dstring& dir_path)
{
   const Reg_Exp re ("^[A-Za-z].*.gws$");
   const Tokens& dir_listing = get_dir_listing (dir_path, re, true);
   for (const Dstring& file_path : dir_listing) { ingest (file_path); }
}

void
Predictor::Sequence::Map::ingest (const Dstring& sequence_file_path)
{

   Dstring this_station, station;
   Predictor::Sequence sequence;
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
      const Real u_925 = stof (tokens[3]) * multiplier;
      const Real v_925 = stof (tokens[4]) * multiplier;
      const Wind wind_925 (u_925, v_925);

      const Real t_925 = stof (tokens[5]);

      if (station != this_station)
      {
         station_tokens.push_back (station);
         insert (make_pair (station, sequence));
         this_station = station;
      }
  
      const Dtime dtime (base_time.t + forecast_hour);
      at (station).ingest (dtime, Predictor (wind_925, t_925));

   }

   file.close ();

}

const Tokens&
Predictor::Sequence::Map::get_station_tokens () const
{
   return station_tokens;
}


