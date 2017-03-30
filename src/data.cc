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

bool
Record::operator == (const Record& record) const
{
   return (dtime == record.dtime);
}

bool
Record::operator > (const Record& record) const
{
   return (dtime > record.dtime);
}

bool
Record::operator < (const Record& record) const
{
   return (dtime < record.dtime);
}

Sample*
Record::Set::get_gradient_temperature_sample_ptr () const
{
   Tuple tuple;
   for (const Record& r : *this) { tuple.push_back (r.gradient_temperature); }
   return new Sample (tuple);
}

void
Record::Set::render_scatter_plot (const RefPtr<Context>& cr,
                                  const Wind_Disc& wind_disc,
                                  const Real dir_scatter) const
{

   const Real scatter_ring_size = 8;

   Real alpha = 50.0 / wind_disc.get_total_count ();
   if (alpha < 0.04) { alpha = 0.04; }
   if (alpha > 0.30) { alpha = 0.30; }

   const Ring ring (scatter_ring_size);

   const Wind_Disc::Transform& t = wind_disc.get_transform ();
   const Transform_2D& transform = t;
   const Real max_speed = t.get_max_speed ();
   const Real calm_threshold = wind_disc.get_thresholds ().front ().value;

   const Sample* sample_ptr = get_gradient_temperature_sample_ptr ();
   const Real mean = sample_ptr->get_mean ();
   const Real sd = sample_ptr->get_sd ();
   const Real min_temp = mean - 2 * sd;
   const Real max_temp = mean + 2 * sd;
   const Real delta_temp = max_temp - min_temp;
   delete sample_ptr;

   for (const Record& record : *this)
   {

      const Real gradient_temperature = record.gradient_temperature;
      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }
      if (speed < calm_threshold) { continue; }
      if (speed > max_speed) { continue; }

      Real hue = (gradient_temperature - mean < 0) ? 0.666 : 0;
      Real saturation = bound (fabs (gradient_temperature - mean) / (2 * sd));
      Real brightness = 0.5;
      const Color& color = Color::hsb (hue, saturation, brightness, alpha);

      const Real r = random (dir_scatter, -dir_scatter);
      const Real direction = wind.get_direction () + r;

      ring.cairo (cr, transform.transform (Point_2D (direction, speed)));
      color.cairo (cr);
      cr->fill_preserve ();
      color.with_alpha (alpha * 2).cairo (cr);
      cr->stroke ();

   }

}

Record::Monthly::Monthly ()
{
   const Record::Set hourly;
   for (Integer hour = 0; hour <= 24; hour++)
   {
      insert (make_pair (hour, hourly));
   }
}

void
Record::Monthly::add (const Integer hour,
                      const Record& record)
{
   Record::Set& hourly = at (hour);
   hourly.insert (record);
}

void
Station_Data::add (const Integer month,
                   const Integer hour,
                   const Record& record)
{
   Record::Monthly& monthly= at (month);
   monthly.add (hour, record);
}

bool
Station_Data::match_gradient_wind (const Wind_Disc& wind_disc,
                                   const set<Index_2D>& gradient_wind_index_set,
                                   const Wind& gradient_wind) const
{

   if (gradient_wind_index_set.size () == 0) { return true; }

   const Index_2D& gw_index = wind_disc.get_index (gradient_wind);
   for (const Index_2D& i2d : gradient_wind_index_set)
   {
      if (i2d == gw_index) { return true; }
   }

   return false;

}

Station_Data::Station_Data ()
{
   const Record::Monthly monthly;
   for (Integer month = 1; month <= 12; month++)
   {
      insert (make_pair (month, monthly));
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

      const Record::Monthly& monthly = at (month);

      for (const Integer& hour : hour_set)
      {

         const Record::Set& hourly = monthly.at (hour);

         for (const Record& record : hourly)
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

Record::Set*
Station_Data::get_record_set_ptr (const Gwsb& gwsb) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   const set<Integer>& month_set = gwsb.get_month_set ();
   const set<Integer>& hour_set = gwsb.get_hour_set ();

   for (const Integer& month : month_set)
   {

      const Record::Monthly& monthly = at (month);

      for (const Integer& hour : hour_set)
      {

         const Record::Set& hourly = monthly.at (hour);

         for (const Record& record : hourly)
         {

            const Wind& gradient_wind = record.gradient_wind;

            if (gwsb.match_gradient_wind (gradient_wind))
            {
               record_set_ptr->insert (record);
            }

         }

      }

   }

   return record_set_ptr;

}

Record::Set*
Station_Data::get_record_set_ptr (const set<Integer>& month_set,
                                  const set<Integer>& hour_set,
                                  const Wind_Disc& wind_disc,
                                  const set<Index_2D>& gradient_wind_index_set) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   for (const Integer& month : month_set)
   {

      const Record::Monthly& monthly = at (month);

      for (const Integer& hour : hour_set)
      {

         const Record::Set& hourly = monthly.at (hour);

         for (const Record& record : hourly)
         {
            const Wind& gradient_wind = record.gradient_wind;
            const bool match = match_gradient_wind (
               wind_disc, gradient_wind_index_set, gradient_wind);
            if (match) { record_set_ptr->insert (record); }
         }

      }

   }

   return record_set_ptr;

}

Record::Set*
Station_Data::get_record_set_ptr (const set<Integer>& month_set,
                                  const set<Integer>& hour_set,
                                  const Wind& gradient_wind,
                                  const Real threshold) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   for (const Integer& month : month_set)
   {

      const Record::Monthly& monthly = at (month);

      for (const Integer& hour : hour_set)
      {

         const Record::Set& hourly = monthly.at (hour);

         for (const Record& record : hourly)
         {
            const Wind& difference = gradient_wind - record.gradient_wind;
            const bool match = gradient_wind.is_naw () ||
                               gsl_isnan (threshold) ||
                               (difference.get_speed () < threshold);
            if (match) { record_set_ptr->insert (record); }
         }

      }

   }

   return record_set_ptr;

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


