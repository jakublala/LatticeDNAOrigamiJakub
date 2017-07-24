// met_movetypes.h

#ifndef MET_STAPLE_MOVETYPES_H
#define MET_STAPLE_MOVETYPES_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "hash.h"
#include "movetypes.h"
#include "utility.h"

namespace MetMovetypes {

    using std::ostream;
    using std::string;
    using std::unordered_map;
    using std::vector;

    using DomainContainer::Domain;
    using Movetypes::MovetypeTracking;
    using Movetypes::RegrowthMCMovetype;
    using Utility::StapleExchangeTracking;
    using Utility::StapleRegrowthTracking;

    class MetMCMovetype: public RegrowthMCMovetype {
        public:
            using RegrowthMCMovetype::RegrowthMCMovetype;

            void reset_internal() override;
            string m_label() override {return "Met";};

        protected:
            void grow_chain(vector<Domain*> domains) override;
            void add_external_bias() override;
            void subtract_external_bias() override;
            void unassign_domains(vector<Domain*> domains);

            double m_delta_e {0};
    };

    class MetStapleExchangeMCMovetype: public MetMCMovetype {
        public:
            using MetMCMovetype::MetMCMovetype;

            bool attempt_move(long long int step) override;
            void reset_internal() override;
            string m_label() override final {return "Met staple exchange";}
            void write_log_summary(ostream* log_stream) override final;

        protected:

            // These can be overidden for a derived class the excludes misbinding
            int preconstrained_df {1};

            // I select by domains not by sites, so this is consistent
            int m_insertion_sites {m_origami_system.num_domains()};

        private:
            bool staple_insertion_accepted(int c_i_ident);
            bool staple_deletion_accepted(int c_i_ident);
            bool insert_staple();
            bool delete_staple();

            StapleExchangeTracking m_tracker {};
            unordered_map<StapleExchangeTracking, MovetypeTracking> m_tracking {};
    };

    class MetStapleRegrowthMCMovetype: public MetMCMovetype {
        public:
            using MetMCMovetype::MetMCMovetype;

            bool attempt_move(long long int step) override final;
            string m_label() override final {return "Met staple regrowth";}
            void write_log_summary(ostream* log_stream) override final;

            StapleRegrowthTracking m_tracker {};
            unordered_map<StapleRegrowthTracking, MovetypeTracking> m_tracking {};
    };
}

#endif // MET_MOVETYPES_H
