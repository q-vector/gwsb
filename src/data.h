#ifndef GWSB_DATA_H
#define GWSB_DATA_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include <denise/stat.h>
#include "selection.h"

using namespace std;

namespace gwsb
{

   class Criteria;

   class Group : public denise::Polygon
   {

      public:

         Group ();

   };

   class Groups : public vector<Group*>
   {

      public:

         Integer
         defining;

         Groups ();

         ~Groups ();

         bool
         is_defining () const;

         Integer
         get_index (const Point_2D& point) const;

         Group&
         get_group (const Integer index);

         const Group&
         get_group (const Integer index) const;

         Group&
         get_group (const Point_2D& point);

         const Group&
         get_group (const Point_2D& point) const;

         Group&
         get_defining_group ();

         const Group&
         get_defining_group (const Integer index) const;

         void
         add (const Point_2D& point,
              const Integer index = -1);

         void
         remove (const Integer index = -1);

         void
         clear ();

         void
         cairo (const RefPtr<Context>& cr) const;

   };

   class Record
   {

      public:

         Dtime
         dtime;

         Wind
         gradient_wind;

         Real
         gradient_temperature;

         Wind
         wind;

         Record (const Dtime& dtime,
                 const Wind& gradient_wind,
                 const Real gradient_temperature,
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
               get_gradient_temperature_sample_ptr () const;

               void
               render_scatter_plot (const RefPtr<Context>& cr,
                                    const Transform_2D& transform,
                                    const Real dir_scatter,
                                    const Groups& groups) const;

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
                             const Wind& gradient_wind,
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
               const Dstring& station_string);

         const Tokens&
         get_station_tokens () const;

         Station_Data&
         get_station_data (const Dstring& station);

   };

};

#endif /* GWSB_DATA_H */

