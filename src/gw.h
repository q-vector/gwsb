#ifndef GWSB_GW_H
#define GWSB_GW_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include <denise/stat.h>
#include "data.h"

using namespace std;

namespace gwsb
{

   class Criteria;
   class Gwsb;

   class Predictor
   {

      public:

         Dstring
         station;

         Dtime
         base_time;

         Real
         forecast_hour;

         Wind
         wind_925;

         Real
         temperature_925;

         Predictor (const Dstring& station,
                    const Dtime& base_time,
                    const Real forecast_hour,
                    const Wind& wind_925,
                    const Real temperature_925);

         Dtime
         get_time () const;

         bool
         operator == (const Predictor& predictor) const;
         
         bool
         operator > (const Predictor& predictor) const;
         
         bool
         operator < (const Predictor& predictor) const;

         void
         render (const RefPtr<Context>& cr,
                 const Wind_Disc& wind_disc) const;

         class Sequence : public map<Dtime, Predictor>
         {

            private:

               set<Dtime>
               time_set;

            public:

               const set<Dtime>&
               get_time_set () const;

               void
               ingest (const Predictor& predictor);

               class Map : public map <Dstring, Sequence>
               {

                  private:

                     Tokens
                     station_tokens;

                  public:

                     Map (const Dstring& dir_path);

                     void
                     ingest (const Dstring& sequence_file_path);

                     const Tokens&
                     get_station_tokens () const;

               };

         };

   };

};

#endif /* GWSB_GW_H */

