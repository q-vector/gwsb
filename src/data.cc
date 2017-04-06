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

void
Record::Set::feed (Wind_Rose& wind_rose) const
{
   for (const Record& record : *this) { wind_rose.add_wind (record.wind); }
}

Sample*
Record::Set::get_gradient_temperature_sample_ptr () const
{
   Tuple tuple;
   for (const Record& r : *this) { tuple.push_back (r.gradient_temperature); }
   return new Sample (tuple);
}

void
Record::Set::sieve_by_gradient_wind (set<Record>& record_set,
                                     const Wind& gradient_wind,
                                     const Real threshold) const
{
   for (const Record& record : *this)
   {
      const Wind& difference = gradient_wind - record.gradient_wind;
      const bool match = gradient_wind.is_naw () || gsl_isnan (threshold) ||
                         (difference.get_speed () < threshold);
      if (match) { record_set.insert (record); }
   }
}

void
Record::Set::render_scatter_plot (const RefPtr<Context>& cr,
                                  const Transform_2D& transform,
                                  const Real dir_scatter,
                                  const Polygon& polygon) const
{

   const Real scatter_ring_size = 8;
   const Integer n = size ();
   const Real alpha = bound (50.0 / n, 0.30, 0.04);
   const Ring ring (scatter_ring_size);

   const Sample* sample_ptr = get_gradient_temperature_sample_ptr ();
   const Real mean = sample_ptr->get_mean ();
   const Real sd = sample_ptr->get_sd ();
   const Real min_temp = mean - 2 * sd;
   const Real max_temp = mean + 2 * sd;
   const Real delta_temp = max_temp - min_temp;
   delete sample_ptr;

   for (const Record& record : *this)
   {

      const Real gt_residual = record.gradient_temperature - mean;
      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }

      const Real hue = (gt_residual < 0) ? 0.666 : 0;
      const Real saturation = bound (fabs (gt_residual) / (2 * sd));
      const Real brightness = 0.5;
      const Color& color = Color::hsb (hue, saturation, brightness, alpha);

      const Real r = random (dir_scatter, -dir_scatter);
      const Real direction = wind.get_direction () + r;
      const Point_2D p = transform.transform (Point_2D (direction, speed));

      ring.cairo (cr, p);
      color.cairo (cr);
      cr->fill_preserve ();
      color.with_alpha (alpha * 2).cairo (cr);
      cr->stroke ();

      const Wind& gw = record.gradient_wind;
      const Real d_gw = gw.get_direction ();
      const Real s_gw = gw.get_speed () / multiplier;
      const Point_2D p_gw = transform.transform (Point_2D (d_gw, s_gw));
      Label ("G", p_gw, 'c', 'c').cairo (cr);

      cr->set_line_width (0.5);
      Dashes ("1:1").cairo (cr);
      Edge (p, p_gw).cairo (cr);

      if (polygon.contains (transform.transform (
          Point_2D (wind.get_direction (), speed))))
      {
         cr->save ();
         ring.cairo (cr, p);
         Color::green (0.5).cairo (cr);
         cr->stroke ();
         cr->restore ();
      }

   }

}

Record::Monthly::Monthly ()
{
   const Record::Set hourly;
   for (Integer hour = 0; hour <= 24; hour++)
   {
      const Dstring& hour_str = Dstring::render ("%02dZ", hour);
      insert (make_pair (hour_str, hourly));
   }
}

void
Record::Monthly::add (const Dstring& hour_str,
                      const Record& record)
{
   Record::Set& hourly = at (hour_str);
   hourly.insert (record);
}

void
Station_Data::add (const Dstring& month_str,
                   const Dstring& hour_str,
                   const Record& record)
{
   Record::Monthly& monthly = at (month_str);
   monthly.add (hour_str, record);
}

Station_Data::Station_Data ()
{
   const Record::Monthly monthly;
   insert (make_pair ("Jan", monthly));
   insert (make_pair ("Feb", monthly));
   insert (make_pair ("Mar", monthly));
   insert (make_pair ("Apr", monthly));
   insert (make_pair ("May", monthly));
   insert (make_pair ("Jun", monthly));
   insert (make_pair ("Jul", monthly));
   insert (make_pair ("Aug", monthly));
   insert (make_pair ("Sep", monthly));
   insert (make_pair ("Oct", monthly));
   insert (make_pair ("Nov", monthly));
   insert (make_pair ("Dec", monthly));
}

void
Station_Data::read (const Dstring& file_path)
{

   igzstream file (file_path.get_string ());

   for (string il; std::getline (file, il); )
   {

      const Dstring input_line (il);
      const Tokens tokens (input_line, ":");

      const Dtime& dtime (tokens[0]);

      const Real gw_direction = stof (tokens[1]);
      const Real gw_speed = stof (tokens[2]);
      const Real gradient_temperature = stof (tokens[3]);
      const Real direction = stof (tokens[4]);
      const Real speed = stof (tokens[5]);

      const Wind& gwind = Wind::direction_speed (gw_direction, gw_speed);
      const Wind& wind = Wind::direction_speed (direction, speed);

      const Dstring& month_str = dtime.get_string ("%b");
      const Dstring& hour_str = dtime.get_string ("%HZ");
      const Record record (dtime, gwind, gradient_temperature, wind);

      add (month_str, hour_str, record);

   }

   file.close ();

}

Record::Set*
Station_Data::get_record_set_ptr (const Dstring& month_str,
                                  const Dstring& hour_str,
                                  const Wind& gradient_wind,
                                  const Real threshold) const
{
   Record::Set* record_set_ptr = new Record::Set ();
   const Record::Set& hourly = at (month_str).at (hour_str);
   hourly.sieve_by_gradient_wind (*record_set_ptr, gradient_wind, threshold);
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

   if (has_station_data)
   {
      return iterator->second;
   }
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


