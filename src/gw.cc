#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Predictor::Predictor (const Dstring& station,
                      const Dtime& base_time,
                      const Real forecast_hour,
                      const Wind& wind_925,
                      const Real temperature_925)
   : station (station),
     base_time (base_time),
     forecast_hour (forecast_hour),
     wind_925 (wind_925),
     temperature_925 (temperature_925)
{
}

Dtime
Predictor::get_time () const
{
   return Dtime (base_time.t + forecast_hour);
}

bool
Predictor::operator == (const Predictor& predictor) const
{
   return (get_time () == predictor.get_time ());
}

bool
Predictor::operator > (const Predictor& predictor) const
{
   return (get_time () > predictor.get_time ());
}

bool
Predictor::operator < (const Predictor& predictor) const
{
   return (get_time () < predictor.get_time ());
}

const set<Dtime>&
Predictor::Sequence::get_time_set () const
{
   return time_set;
}

void
Predictor::Sequence::ingest (const Predictor& predictor)
{
   const Dtime& dtime = predictor.get_time ();
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
  
      at (station).ingest (Predictor (station, base_time, forecast_hour, gw, t));

   }

   file.close ();

}

const Tokens&
Predictor::Sequence::Map::get_station_tokens () const
{
   return station_tokens;
}


