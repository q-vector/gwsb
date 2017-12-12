#ifndef NINE2FIVE_GW_H
#define NINE2FIVE_GW_H

#include <set>
#include <iostream>
#include <denise/gtkmm.h>
#include <denise/met.h>
#include <denise/stat.h>
#include "data.h"

using namespace std;

namespace nine2five
{

   class Criteria;
   class Nine2five;

   class Predictor
   {

      public:

         Wind
         wind_925;

         Real
         temperature_925;

         Predictor (const Wind& wind_925,
                    const Real temperature_925);

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
               ingest (const Dtime& dtime,
                       const Predictor& predictor);

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

#endif /* NINE2FIVE_GW_H */

