#ifndef GWSB_DATA_H
#define GWSB_DATA_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/histogram.h>
#include <denise/met.h>
#include <denise/stat.h>
//#include "selection.h"

using namespace std;

namespace gwsb
{

   class Criteria;
   class Clusters;
   class Predictor;

   class Record
   {

      public:

         Dtime
         dtime;

         Wind
         wind_925;

         Real
         temperature_925;

         Wind
         wind;

         Record (const Dtime& dtime,
                 const Wind& wind_925,
                 const Real temperature_925,
                 const Wind& wind);

         bool
         operator == (const Record& record) const;
         
         bool
         operator > (const Record& record) const;
         
         bool
         operator < (const Record& record) const;

         class Set : public set<Record>
         {

            public:

               void
               feed (Wind_Rose& wind_rose) const;

               Sample*
               get_temperature_925_sample_ptr () const;

         };

         class Daily : public map<Integer, Record::Set>
         {

            public:
   
               Daily ();

               void
               add (const Integer hour,
                    const Record& record);

               static bool
               match_day_of_year (const Integer day_of_year_a,
                                  const Integer day_of_year_b,
                                  const Integer day_of_year_threshold);


               static bool
               match_hour (const Integer hour_a,
                           const Integer hour_b,
                           const Integer hour_threshold);


         };

   };

   class Station_Data : public map<Integer, Record::Daily>
   {

      private:

         void
         add (const Integer day_of_year,
              const Integer hour,
              const Record& record);

      public:

         Station_Data ();

         void
         read (const Dstring& file_path);

         Record::Set*
         get_record_set_ptr (const Integer day_of_year,
                             const Integer day_of_year_threshold,
                             const Integer hour,
                             const Integer hour_threshold,
                             const Wind& wind_925,
                             const Real threshold = 2.5) const;

   };

   class Data : public map<Dstring, Station_Data>
   {

      private:

         const Dstring
         data_path;

         Tokens
         station_tokens;

         void
         survey ();

      public:

         Data (const Dstring& data_path,
               const Tokens& station_tokens);

         const Tokens&
         get_station_tokens () const;

         Station_Data&
         get_station_data (const Dstring& station);

   };

   class Cluster : public denise::Polygon
   {

      public:

         Tuple
         tuple;

         Histogram_1D
         histogram;

         Real
         probability;

         Wind
         mean_wind;

         Cluster ();

         Gaussian_Distribution
         get_gaussian_distribution () const;

         Real
         get_likelihood (const Real temperature_925,
                         const Integer n) const;
         
   };

   class Clusters : public vector<Cluster*>
   {

      public:

         Integer
         defining;

         Clusters ();

         ~Clusters ();

         bool
         is_defining () const;

         Integer
         get_index (const Point_2D& point) const;

         Cluster&
         get_cluster (const Integer index);

         const Cluster&
         get_cluster (const Integer index) const;

         Cluster&
         get_cluster (const Point_2D& point);

         const Cluster&
         get_cluster (const Point_2D& point) const;

         Cluster&
         get_defining_cluster ();

         const Cluster&
         get_defining_cluster (const Integer index) const;

         void
         add (const Point_2D& point,
              const Integer index = -1);

         void
         remove (const Integer index = -1);

         void
         clear ();

         void
         reset ();

         void
         render (const RefPtr<Context>& cr,
                 const Real alpha) const;

         void
         render_defining (const RefPtr<Context>& cr) const;

         void
         cluster_analysis (const Record::Set& record_set,
                           const Transform_2D& transform,
                           const Predictor& predictor);

   };

};

#endif /* GWSB_DATA_H */

