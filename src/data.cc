#include "data.h"
#include "gwsb.h"
#include "gw.h"

using namespace std;
using namespace denise;
using namespace gwsb;

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
                                  const Clusters& clusters) const
{

   const Real scatter_ring_size = 8;
   const Integer n = size ();
   const Real alpha = bound (50.0 / n, 0.45, 0.05);
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

      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }

      const Integer i = clusters.get_index (
         transform.transform (Point_2D (wind.get_direction (), speed)));
      const Color& color = (i<0 ? Color::gray (0.5, alpha) : Color (i, alpha));

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
            const Tokens& station_tokens)
   : data_path (data_path),
     station_tokens (station_tokens)
{
   if (station_tokens.size () == 0) { survey (); }
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

Cluster::Cluster ()
   : histogram (1, 0.5)
{
}

Gaussian_Distribution
Cluster::get_gaussian_distribution () const
{
   const Sample sample (tuple);
   const Real mean = sample.get_mean ();
   const Real variance = sample.get_variance ();
   return Gaussian_Distribution (mean, variance);
}

Clusters::Clusters ()
   : defining (-1)
{
}

Clusters::~Clusters ()
{
   clear ();
}

bool
Clusters::is_defining () const
{
   return (defining >= 0);
}

Integer
Clusters::get_index (const Point_2D& point) const
{

   for (Integer i = 0; i < size (); i++)
   {
      const Cluster& cluster = get_cluster (i);
      const bool b = cluster.contains (point);
      if (b) { return i; }
   }

   return -1;

}

Cluster&
Clusters::get_cluster (const Integer index)
{
   return *at (index);
}

const Cluster&
Clusters::get_cluster (const Integer index) const
{
   return *at (index);
}

Cluster&
Clusters::get_cluster (const Point_2D& point)
{
   const Integer i = get_index (point);
   if (i < 0) { throw Exception ("No matching cluster."); }
   return get_cluster (i);
}

const Cluster&
Clusters::get_cluster (const Point_2D& point) const
{
   const Integer i = get_index (point);
   if (i < 0) { throw Exception ("No matching cluster."); }
   return get_cluster (i);
}

Cluster&
Clusters::get_defining_cluster ()
{
   return *at (defining);
}

const Cluster&
Clusters::get_defining_cluster (const Integer index) const
{
   return *at (defining);
}

void
Clusters::add (const Point_2D& point,
               const Integer index)
{
   this->defining = (index < 0 ? size () : index);
   get_cluster (defining).add (point);
}

void
Clusters::remove (const Integer index)
{
   const Integer i = (index < 0 ? defining : index);
   if (i < 0 || i > size () - 1) { return; }
   erase (begin () + i);
   if (i == defining) { defining = -1; }
}

void
Clusters::clear ()
{
   for (Cluster* cluster_ptr : *this) { delete cluster_ptr; }
   vector<Cluster*>::clear ();
   defining = -1;
}

void
Clusters::render (const RefPtr<Context>& cr,
                  const Real alpha) const
{

   cr->save ();
   cr->set_line_width (6);
   Dashes ("2:10").cairo (cr);

   for (Integer i = 0; i < size (); i++)
   {
      Color (i, alpha).cairo (cr);
      at (i)->cairo (cr);
      cr->stroke ();
   }

   cr->restore ();

}

void
Clusters::render_defining (const RefPtr<Context>& cr) const
{

   if (defining < 0) { return; }

   cr->save ();
   cr->set_line_width (6);
   Dashes ("2:10").cairo (cr);

   const Cluster& cluster = get_cluster (defining);
   Color (defining).cairo (cr);
   cluster.cairo (cr);
   cr->stroke ();

   cr->restore ();

}

void
Clusters::cluster_analysis (const Record::Set& record_set,
                            const Transform_2D& transform,
                            const Predictor& predictor)
{

   Cluster cluster_rest;
   const Integer n = record_set.size ();

   for (Integer i = 0; i < size (); i++)
   {
      Cluster& cluster = *(at (i));
      cluster.tuple.clear ();
   }

   for (const Record& record : record_set)
   {

      const Wind& wind = record.wind;
      const Real multiplier = 0.51444444;
      const Real speed = wind.get_speed () / multiplier;

      if (wind.is_naw ()) { continue; }

      const Integer i = get_index (
         transform.transform (Point_2D (wind.get_direction (), speed)));

      if (i < 0)
      {
         cluster_rest.histogram.increment (record.temperature_925);
         cluster_rest.tuple.push_back (record.temperature_925);
      }
      else
      {
         Cluster& cluster = *(at (i));
         cluster.histogram.increment (record.temperature_925);
         cluster.tuple.push_back (record.temperature_925);
      }

   }

   const Gaussian_Distribution& gd = cluster_rest.get_gaussian_distribution ();
   const Real likelihood = gd.get_P (predictor.temperature_925 + 0.5) ;
                           - gd.get_P (predictor.temperature_925 - 0.5);
   const Real share = Real (cluster_rest.tuple.size ()) / Real (n);
   const Real term = (likelihood * share);
   Real denominator = (cluster_rest.tuple.size () > 0 ? term : 0);
cout << "prob rest " << denominator << endl;

   for (Integer j = 0; j < size (); j++)
   {
      const Cluster& cluster = *(at (j));
      const Gaussian_Distribution& gd = cluster.get_gaussian_distribution ();
      const Real likelihood = gd.get_P (predictor.temperature_925 + 0.5) ;
                              - gd.get_P (predictor.temperature_925 - 0.5);
      const Real share = Real (cluster.tuple.size ()) / Real (n);
      const Real term = (likelihood * share);
      denominator += (cluster.tuple.size () > 0 ? term : 0);
cout << "denominator j " << j << " " << (likelihood * share) << " " << likelihood << " " << share << " | " << cluster.tuple.size () << " " << n << endl;
   }

cout << "denominator " << denominator << endl;
   for (Integer i = 0; i < size (); i++)
   {
      Cluster& cluster = *(at (i));
      const Gaussian_Distribution &gd = cluster.get_gaussian_distribution ();
      const Real likelihood = gd.get_P (predictor.temperature_925 + 0.5) ;
                              - gd.get_P (predictor.temperature_925 - 0.5);
      const Real share = Real (cluster.tuple.size ()) / Real (n);
      const Real term = (likelihood * share);
      cluster.probability = term / denominator;
cout << "Cluster " << i << " " << cluster.probability << endl;
   }

}


