#include "data.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Record::Record (const Dtime& dtime,
                const Wind& gradient_wind,
                const Real gradient_temperature,
                const Wind& wind)
   : dtime (dtime),
     gradient_wind (gradient_wind),
     gradient_temperature (gradient_temperature),
     wind (wind)
{
}

Monthly_Data::Monthly_Data ()
{
   const Hourly_Data hourly_data;
   for (Integer hour = 0; hour <= 24; hour++)
   {
      insert (make_pair (hour, hourly_data));
   }
}

void
Monthly_Data::add (const Integer hour,
                   const Record& record)
{
   Hourly_Data& hourly_data = at (hour);
   hourly_data.push_back (record);
}

void
Station_Data::add (const Integer month,
                   const Integer hour,
                   const Record& record)
{
   Monthly_Data& monthly_data = at (month);
   monthly_data.add (hour, record);
}

Station_Data::Station_Data ()
{
   const Monthly_Data monthly_data;
   for (Integer month = 1; month <= 12; month++)
   {
      insert (make_pair (month, monthly_data));
   }
}

void
Station_Data::read (const Dstring& file_path)
{

   const Integer line_size = 128;  
   char c_input_line[line_size];
   gzFile file = get_gzfile (file_path);

   while (gz_readline (c_input_line, line_size, file) != NULL)
   {

      const Dstring input_line (c_input_line);
      const Tokens tokens (input_line, ":");

      const Dtime& dtime (tokens[0]);

      const Real gw_direction = atof (tokens[1].c_str ());
      const Real gw_speed = atof (tokens[2].c_str ());
      const Real gradient_temperature = atof (tokens[3].c_str ());
      const Real direction = atof (tokens[4].c_str ());
      const Real speed = atof (tokens[5].c_str ());

      const Wind& gwind = Wind::direction_speed (gw_direction, gw_speed);
      const Wind& wind = Wind::direction_speed (direction, speed);

      const Integer month = dtime.get_month ();
      const Integer hour = dtime.get_hour ();
      const Record record (dtime, gwind, gradient_temperature, wind);

      add (month, hour, record);

   }

   gzclose (file);

}

void
Station_Data::feed (Wind_Rose& wind_rose,
                    const Gwsb& gwsb) const
{

   const set<Integer>& month_set = gwsb.get_month_set ();
   const set<Integer>& hour_set = gwsb.get_hour_set ();

   for (const Integer& month : month_set)
   {

      const Monthly_Data& monthly_data = at (month);

      for (const Integer& hour : hour_set)
      {

         const Hourly_Data& hourly_data = monthly_data.at (hour);

         for (const Record& record : hourly_data)
         {

            const Wind& gradient_wind = record.gradient_wind;

            if (gwsb.match_gradient_wind (gradient_wind))
            {
               wind_rose.add_wind (record.wind);
            }

         }

      }

   }

}

vector<Record>*
Station_Data::get_record_vector_ptr (const Gwsb& gwsb) const
{

   vector<Record>* record_vector_ptr = new vector<Record>;

   const set<Integer>& month_set = gwsb.get_month_set ();
   const set<Integer>& hour_set = gwsb.get_hour_set ();

   for (const Integer& month : month_set)
   {

      const Monthly_Data& monthly_data = at (month);

      for (const Integer& hour : hour_set)
      {

         const Hourly_Data& hourly_data = monthly_data.at (hour);

         for (const Record& record : hourly_data)
         {

            const Wind& gradient_wind = record.gradient_wind;

            if (gwsb.match_gradient_wind (gradient_wind))
            {
               record_vector_ptr->push_back (record);
            }

         }

      }

   }

   return record_vector_ptr;

}

void
Data::survey ()
{

   const Dstring search_str ("[A-Z][A-Z][A-Z][A-Z].gz");
   const Tokens& dir_listing = get_dir_listing (data_path, search_str);

   for (const Dstring& file_name : dir_listing)
   {
      const Tokens tokens (file_name, ".");
      const Dstring& station = tokens[0];
      station_tokens.push_back (station);
   }

}

Data::Data (const Dstring& data_path,
            const Dstring& station_string)
   : data_path (data_path)
{
   if (station_string.size () == 0) { survey (); }
   else { station_tokens = Tokens (station_string, ":"); }
}

const Tokens&
Data::get_station_tokens () const
{
   return station_tokens;
}

Station_Data&
Data::get_station_data (const Dstring& station)
{

   Data::iterator iterator = find (station);
   const bool has_station_data = (iterator != end ());

   if (has_station_data) { return iterator->second; }
   else
   {

      const Station_Data station_data;
      insert (make_pair (station, station_data));

      Station_Data& sd = find (station)->second;
      const string& file_path = data_path + "/" + station + ".gz";
      sd.read (file_path);
      return sd;

   }

}


