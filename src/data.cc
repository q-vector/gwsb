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
   const Real n = Real (wind_disc.get_total_count ());
   const Real alpha = bound (50.0 / n, 0.30, 0.04);
   const Ring ring (scatter_ring_size);

   const Sample* sample_ptr = get_gradient_temperature_sample_ptr ();
   const Real mean = sample_ptr->get_mean ();
   const Real sd = sample_ptr->get_sd ();
   const Real min_temp = mean - 2 * sd;
   const Real max_temp = mean + 2 * sd;
   const Real delta_temp = max_temp - min_temp;
   delete sample_ptr;

   const Wind_Disc::Transform& t = wind_disc.get_transform ();

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
      const Point_2D p = t.transform (Point_2D (direction, speed));

      ring.cairo (cr, p);
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

void
Station_Data::feed (Wind_Rose& wind_rose,
                    const Gwsb_Free& gwsb_free) const
{

   const Selection_Panel::Status& month_status = gwsb_free.get_month_status ();
   const Selection_Panel::Status& hour_status = gwsb_free.get_hour_status ();

   for (auto& m : month_status)
   {

      if (!m.second) { continue; }
      const Record::Monthly& monthly = at (m.first);

      for (auto& h : hour_status)
      {

         if (!h.second) { continue; }
         const Record::Set& hourly = monthly.at (h.first);

         for (const Record& record : hourly)
         {

            const Wind& gradient_wind = record.gradient_wind;

            if (gwsb_free.match_gradient_wind (gradient_wind))
            {
               wind_rose.add_wind (record.wind);
            }

         }

      }

   }

}

Record::Set*
Station_Data::get_record_set_ptr (const Gwsb_Free& gwsb_free) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   const Selection_Panel::Status& month_status = gwsb_free.get_month_status ();
   const Selection_Panel::Status& hour_status = gwsb_free.get_hour_status ();

   for (auto& m : month_status)
   {

      if (!m.second) { continue; }
      const Record::Monthly& monthly = at (m.first);

      for (auto& h : hour_status)
      {

         if (!h.second) { continue; }
         const Record::Set& hourly = monthly.at (h.first);

         for (const Record& record : hourly)
         {

            const Wind& gradient_wind = record.gradient_wind;

            if (gwsb_free.match_gradient_wind (gradient_wind))
            {
               record_set_ptr->insert (record);
            }

         }

      }

   }

   return record_set_ptr;

}

Record::Set*
Station_Data::get_record_set_ptr (const Selection_Panel::Status& month_status,
                                  const Selection_Panel::Status& hour_status,
                                  const Wind_Disc& wind_disc,
                                  const set<Index_2D>& gradient_wind_index_set) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   for (auto& m : month_status)
   {

      if (!m.second) { continue; }
      const Record::Monthly& monthly = at (m.first);

      for (auto& h : hour_status)
      {

         if (!h.second) { continue; }
         const Record::Set& hourly = monthly.at (h.first);

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
Station_Data::get_record_set_ptr (const Selection_Panel::Status& month_status,
                                  const Selection_Panel::Status& hour_status,
                                  const Wind& gradient_wind,
                                  const Real threshold) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   for (auto& m : month_status)
   {

      if (!m.second) { continue; }
      const Record::Monthly& monthly = at (m.first);

      for (auto& h : hour_status)
      {

         if (!h.second) { continue; }
         const Record::Set& hourly = monthly.at (h.first);

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

Record::Set*
Station_Data::get_record_set_ptr (const Dstring& month_str,
                                  const Dstring& hour_str,
                                  const Wind& gradient_wind,
                                  const Real threshold) const
{

   Record::Set* record_set_ptr = new Record::Set ();

   const Record::Monthly& monthly = at (month_str);
   const Record::Set& hourly = monthly.at (hour_str);

   for (const Record& record : hourly)
   {
      const Wind& difference = gradient_wind - record.gradient_wind;
      const bool match = gradient_wind.is_naw () ||
                         gsl_isnan (threshold) ||
                         (difference.get_speed () < threshold);
      if (match) { record_set_ptr->insert (record); }
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


