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
   class Gwsb_Free;

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
                                    const Wind_Disc& wind_disc,
                                    const Real dir_scatter) const;

               void
               sieve_by_gradient_wind (set<Record>& record_set,
                                       const Wind& gradient_wind,
                                       const Real threshold) const;

         };

         class Monthly : public map<Dstring, Record::Set>
         {

            public:
   
               Monthly ();

               void
               add (const Dstring& hour_str,
                    const Record& record);

         };

   };

   class Station_Data : public map<Dstring, Record::Monthly>
   {

      private:

         void
         add (const Dstring& month_str,
              const Dstring& hour_str,
              const Record& record);

      public:

         Station_Data ();

         void
         read (const Dstring& file_path);

         Record::Set*
         get_record_set_ptr (const Dstring& month_str,
                             const Dstring& hour_str,
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

