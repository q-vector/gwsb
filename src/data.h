#ifndef GWSB_DATA_H
#define GWSB_DATA_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include <denise/stat.h>

using namespace std;

namespace gwsb
{

   class Criteria;
   class Gwsb;

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

               Sample*
               get_gradient_temperature_sample_ptr () const;

               void
               render_scatter_plot (const RefPtr<Context>& cr,
                                    const Wind_Disc& wind_disc,
                                    const Real dir_scatter) const;

         };

         class Monthly : public map<Integer, Record::Set>
         {

            public:
   
               Monthly ();

               void
               add (const Integer hour,
                    const Record& record);

         };

   };

   class Station_Data : public map<Integer, Record::Monthly>
   {

      private:

         void
         add (const Integer month,
              const Integer hour,
              const Record& record);

         bool
         match_gradient_wind (const Wind_Disc& wind_disc,
                              const set<Index_2D>& gradient_wind_index_set,
                              const Wind& gradient_wind) const;

      public:

         Station_Data ();

         void
         read (const Dstring& file_path);

         void
         feed (Wind_Rose& wind_rose,
               const Gwsb& gwsb) const;

         Record::Set*
         get_record_set_ptr (const Gwsb& gwsb) const;

         Record::Set*
         get_record_set_ptr (const set<Integer>& month_set,
                             const set<Integer>& hour_set,
                             const Wind_Disc& wind_disc,
                             const set<Index_2D>& gradient_wind_index_set) const;

         Record::Set*
         get_record_set_ptr (const set<Integer>& month_set,
                             const set<Integer>& hour_set,
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

#endif /* DENISE_GTKMM_H */

