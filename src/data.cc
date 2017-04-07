#include "data.h"
#include "gwsb.h"

using namespace std;
using namespace denise;
using namespace gwsb;

Group::Group ()
   : histogram (1, 0.5)
{
}

Groups::Groups ()
   : defining (-1)
{
}

Groups::~Groups ()
{
   clear ();
}

bool
Groups::is_defining () const
{
   return (defining >= 0);
}

Integer
Groups::get_index (const Point_2D& point) const
{

   for (Integer i = 0; i < size (); i++)
   {
      const Group& group = get_group (i);
      const bool b = group.contains (point);
      if (b) { return i; }
   }

   return -1;

}

Group&
Groups::get_group (const Integer index)
{
   return *at (index);
}

const Group&
Groups::get_group (const Integer index) const
{
   return *at (index);
}

Group&
Groups::get_group (const Point_2D& point)
{
   const Integer i = get_index (point);
   if (i < 0) { throw Exception ("No matching group."); }
   return get_group (i);
}

const Group&
Groups::get_group (const Point_2D& point) const
{
   const Integer i = get_index (point);
   if (i < 0) { throw Exception ("No matching group."); }
   return get_group (i);
}

Group&
Groups::get_defining_group ()
{
   return *at (defining);
}

const Group&
Groups::get_defining_group (const Integer index) const
{
   return *at (defining);
}

void
Groups::add (const Point_2D& point,
             const Integer index)
{
   this->defining = (index < 0 ? size () : index);
   get_group (defining).add (point);
}

void
Groups::remove (const Integer index)
{
   const Integer i = (index < 0 ? defining : index);
   if (i < 0 || i > size () - 1) { return; }
   erase (begin () + i);
   if (i == defining) { defining = -1; }
}

void
Groups::clear ()
{
   for (Group* group_ptr : *this) { delete group_ptr; }
   vector<Group*>::clear ();
   defining = -1;
}

void
Groups::cairo (const RefPtr<Context>& cr) const
{

   if (defining < 0) { return; }

   cr->save ();
   cr->set_line_width (6);
   Dashes ("2:10").cairo (cr);

   const Group& group = get_group (defining);
   Color (defining).cairo (cr);
   group.cairo (cr);
   cr->stroke ();

   cr->restore ();

}


Record::Record (const Dtime& dtime,
                const Wind& wind_925,
                const Real temperature_925,
                const Wind& wind)
   : dtime (dtime),
     wind_925 (wind_925),
     temperature_925 (temperature_925),
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
Record::Set::get_temperature_925_sample_ptr () const
{
   Tuple tuple;
   for (const Record& r : *this) { tuple.push_back (r.temperature_925); }
   return new Sample (tuple);
}

void
Record::Set::render_scatter_plot (const RefPtr<Context>& cr,
                                  const Transform_2D& transform,
                                  const Real dir_scatter,
                                  Groups& groups) const
{

   const Real scatter_ring_size = 8;
   const Integer n = size ();
   const Real alpha = bound (50.0 / n, 0.45, 0.15);
   const Ring ring (scatter_ring_size);

   const Sample* sample_ptr = get_temperature_925_sample_ptr ();
   const Real mean = sample_ptr->get_mean ();
   const Real sd = sample_ptr->get_sd ();
   const Real min_temp = mean - 2 * sd;
   const Real max_temp = mean + 2 * sd;
   const Real delta_temp = max_temp - min_temp;
   delete sample_ptr;

   srand (0);

   cr->save ();
   cr->set_line_width (0.5);
   Dashes ("1:2").cairo (cr);

   for (const Record& record : *this)
   {

      const Real gt_residual = record.temperature_925 - mean;
      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }

      const Integer i = groups.get_index (
         transform.transform (Point_2D (wind.get_direction (), speed)));
      const Color& color = (i<0 ? Color::gray (0.5, alpha) : Color (i, alpha));
      if (i >= 0) { groups.at (i)->histogram.increment (record.temperature_925); }

      const Real r = random (dir_scatter, -dir_scatter);
      const Real direction = wind.get_direction () + r;
      const Point_2D p = transform.transform (Point_2D (direction, speed));

      ring.cairo (cr, p);
      color.cairo (cr);
      cr->fill ();

      const Wind& gw = record.wind_925;
      const Real d_gw = gw.get_direction ();
      const Real s_gw = gw.get_speed () / multiplier;
      const Point_2D p_gw = transform.transform (Point_2D (d_gw, s_gw));
      Label ("G", p_gw, 'c', 'c').cairo (cr);

      cr->save ();
      Edge (p, p_gw).cairo (cr);
      cr->stroke ();
      cr->restore ();

   }

   cr->restore ();

}

Record::Daily::Daily ()
{
   const Record::Set hourly;
   for (Integer hour = 0; hour <= 24; hour++)
   {
      insert (make_pair (hour, hourly));
   }
}

void
Record::Daily::add (const Integer hour,
                    const Record& record)
{
   Record::Set& hourly = at (hour);
   hourly.insert (record);
}

bool
Record::Daily::match_day_of_year (const Integer a,
                                  const Integer b,
                                  const Integer threshold)
{
   const Integer n = 365;
   if (abs (a - b) <= threshold) { return true; }
   if (((b + n) - a) <= threshold) { return true; }
   if (((a + n) - b) <= threshold) { return true; }
   return false;
}

bool
Record::Daily::match_hour (const Integer a,
                           const Integer b,
                           const Integer threshold)
{
   const Integer n = 24;
   if (abs (a - b) <= threshold) { return true; }
   if (((b + n) - a) <= threshold) { return true; }
   if (((a + n) - b) <= threshold) { return true; }
   return false;
}

void
Station_Data::add (const Integer day_of_year,
                   const Integer hour,
                   const Record& record)
{
   Record::Daily& daily = at (day_of_year);
   daily.add (hour, record);
}

Station_Data::Station_Data ()
{
   for (Integer i = i; i <= 366; i++)
   {
      const Record::Daily daily;
      insert (make_pair (i, daily));
   }
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

      const Real direction_925 = stof (tokens[1]);
      const Real speed_925 = stof (tokens[2]);
      const Real temperature_925 = stof (tokens[3]);
      const Real direction = stof (tokens[4]);
      const Real speed = stof (tokens[5]);

      const Wind& wind_925 = Wind::direction_speed (direction_925, speed_925);
      const Wind& wind = Wind::direction_speed (direction, speed);

      const Integer j = stoi (dtime.get_string ("%j"));
      const Integer h = stoi (dtime.get_string ("%H"));
      const Record record (dtime, wind_925, temperature_925, wind);

      add (j, h, record);

   }

   file.close ();

}

Record::Set*
Station_Data::get_record_set_ptr (const Integer day_of_year,
                                  const Integer day_of_year_threshold,
                                  const Integer hour,
                                  const Integer hour_threshold,
                                  const Wind& wind_925,
                                  const Real threshold) const
{

   const Integer n = 365;
   Record::Set* record_set_ptr = new Record::Set ();

   for (auto jj = begin (); jj != end (); jj++)
   {

      const Integer j = jj->first;
      const Record::Daily& daily = jj->second;

      if (!Record::Daily::match_day_of_year (
         j, day_of_year, day_of_year_threshold))
      {
         continue;
      }

      for (auto hh = daily.begin (); hh != daily.end (); hh++)
      {

         const Integer h = hh->first;
         const Record::Set& hourly = hh->second;

         if (!Record::Daily::match_hour (h, hour, hour_threshold))
         {
            continue;
         }

         for (const Record& record : hourly)
         {
            const Wind& difference = wind_925 - record.wind_925;
            const bool match = wind_925.is_naw () ||
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


