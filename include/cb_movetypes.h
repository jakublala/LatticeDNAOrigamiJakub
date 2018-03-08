// cb_movetypes.h

#ifndef CB_STAPLE_MOVETYPES_H
#define CB_STAPLE_MOVETYPES_H

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "domain.h"
#include "files.h"
#include "hash.h"
#include "ideal_random_walk.h"
#include "movetypes.h"
#include "origami_system.h"
#include "parser.h"
#include "random_gens.h"
#include "top_constraint_points.h"
#include "utility.h"

/** Movetypes involving configuration bias */
namespace movetypes {

    using std::ostream;
    using std::pair;
    using std::set;
    using std::string;
    using std::unordered_map;
    using std::vector;

    using domainContainer::Domain;
    using files::OrigamiInputFile;
    using files::OrigamiOutputFile;
    using idealRandomWalk::IdealRandomWalks;
    using movetypes::MovetypeTracking;
    using movetypes::RegrowthMCMovetype;
    using movetypes::add_tracker;
    using origami::OrigamiSystem;
    using parser::InputParameters;
    using randomGen::RandomGens;
    using topConstraintPoints::Constraintpoints;
    using utility::VectorThree;
    using utility::StapleRegrowthTracking;
    using utility::CTCBScaffoldRegrowthTracking;
    using utility::CTCBLinkerRegrowthTracking;
    using utility::VectorThree;

    typedef vector<pair<VectorThree, VectorThree>> configsT;

    /**
      * Base for CB moves
      */
    class CBMCMovetype:
        virtual public RegrowthMCMovetype {

        public:
            CBMCMovetype(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params);
            CBMCMovetype(const CBMCMovetype&) = delete;
            CBMCMovetype& operator=(const CBMCMovetype&) = delete;

            virtual void reset_internal() override;

        protected:

            /** Include external bias on whole configuration in Rosenbluth */
            void add_external_bias() override;

            /** Calculate the CB trial weights and rosenbluth-type weight */
            virtual vector<double> calc_bias(
                    const vector<double> bfactors, // Boltzman weights
                    const configsT& configs, // Configs
                    Domain* domain) = 0; // Domain being regrown

            /**
              * Calculate the Boltzmann weights for all configurations
              *
              * Fills the configuration and weights arrays with the Boltzmann
              * weights for all configurations available to the given domain.
              * If all orientations are permissible for a given position, it
              * will output zero-orientation and multiply the weight by the
              * number of possible orientations.
              */
            void calc_biases(
                    const VectorThree p_prev, // Position of growthpoint domain
                    Domain& domain, // Domain to calculate weights for
                    configsT& configs, // Configs considered
                    vector<double>& bfactors); // Weights for configs

            /** Select and set a configuration for a domain using CB */
            void select_and_set_config(
                    const int i, // Index in segment array of domain to regrow
                    vector<Domain*> domains); // Segment being regrown

            /** Select and set a configuration from given configs and weights */
            void select_and_set_new_config(
                    const vector<double> weights, // Weights
                    const configsT configs, // Config
                    Domain& domain); // Domain being regrown

            /** Set configuration to old */
            void select_and_set_old_config(Domain& domain);

            /** Set growthpoint without checking constraints */
            double set_old_growth_point(
                    Domain& growth_domain_new,
                    Domain& growth_domain_old);

            /**
              * Test move acceptance and take appropriate action
              *
              * If accepted it will revert to the new configuration (as the
              * system will currently be in the old configuration), otherwise
              * it will clear the reset arrays so that reset origami has no
              * effect.
              */
            bool test_cb_acceptance();

            /** Unassign all given domains and store old configurations */
            void unassign_domains(vector<Domain*>);

            /** Prepare interals for regrowing old configuration */
            void setup_for_regrow_old();

            /** Update the external bias without adding */
            void update_external_bias();

            long double m_bias {1}; // Rosenbluth-type weight
            long double m_new_bias {1}; // Storage for new config's Rosenbluth
            double m_new_modifier {1}; // Storage for new config's modifier
            bool m_regrow_old {false}; // Regrowing old configuration
            unordered_map<pair<int, int>, VectorThree> m_old_pos {};
            unordered_map<pair<int, int>, VectorThree> m_old_ore {};
    };

    /**
      * CB regrowth of staples
      */
    class CBStapleRegrowthMCMovetype:
        public CBMCMovetype {

        public:
            CBStapleRegrowthMCMovetype(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params);
            CBStapleRegrowthMCMovetype(const
                    CBStapleRegrowthMCMovetype&) = delete;
            CBStapleRegrowthMCMovetype& operator=(const
                    CBStapleRegrowthMCMovetype&) = delete;

            void write_log_summary(ostream* log_entry) override;

        private:
            bool internal_attempt_move() override;
            virtual void add_tracker(bool accepted) override;

            /** Grow out domains from first in given array */
            void grow_chain(vector<Domain*> domains) override;

            /** Calculate the CB trial weights and rosenbluth-type weight */
            vector<double> calc_bias(
                    const vector<double> bfactors, // Boltzman weights
                    const configsT& configs, // Configs
                    Domain* domain) override; // Domain being regrown

            /** Set given growthpoint and grow staple */
            void set_growthpoint_and_grow_staple(
                    domainPairT growthpoint,
                    vector<Domain*> selected_chain);

            StapleRegrowthTracking m_tracker {};
            unordered_map<StapleRegrowthTracking, MovetypeTracking> m_tracking {};
    };

    /**
      * Base for conserved topology CB regrowth moves
      */
    class CTCBRegrowthMCMovetype:
        virtual public CBMCMovetype,
        virtual public CTRegrowthMCMovetype {

        public:
            CTCBRegrowthMCMovetype(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params,
                    int num_excluded_staples,
                    int max_regrowth);
            CTCBRegrowthMCMovetype(const
                    CTCBRegrowthMCMovetype&) = delete;
            CTCBRegrowthMCMovetype& operator=(const
                    CTCBRegrowthMCMovetype&) = delete;

            void reset_internal() override;

        protected:

            /** Grow out domains from first in given array */
            void grow_chain(vector<Domain*> domains) final override;

            /** Calculate the CB trial weights and rosenbluth-type weight */
            vector<double> calc_bias(
                    const vector<double> bfactors, // Boltzman weights
                    const configsT& configs, // Configs
                    Domain* domain) override; // Domain being regrown

            /** Grow given staple and update fixed-end biases */
            void grow_staple_and_update_endpoints(
                    Domain* growth_domain_old);
    };

    /**
      * CTCB regrowth of scaffold segments and bound staples
      */
    class CTCBScaffoldRegrowthMCMovetype:
        public CTCBRegrowthMCMovetype {

        public:
            CTCBScaffoldRegrowthMCMovetype(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params,
                    int num_excluded_staples,
                    int max_regrowth);
            CTCBScaffoldRegrowthMCMovetype(const
                    CTCBScaffoldRegrowthMCMovetype&) = delete;
            CTCBScaffoldRegrowthMCMovetype& operator=(const
                    CTCBScaffoldRegrowthMCMovetype&) = delete;

            void write_log_summary(ostream* log_entry) override;

        private:
            bool internal_attempt_move() override;
            virtual void add_tracker(bool accepted) override;

            CTCBScaffoldRegrowthTracking m_tracker {};
            unordered_map<CTCBScaffoldRegrowthTracking, MovetypeTracking> m_tracking {};
    };

    /**
      * Transformation of a segment and regrowth of its linkers
      */
    class CTCBLinkerRegrowthMCMovetype:
        public CTCBRegrowthMCMovetype {

        public:
            CTCBLinkerRegrowthMCMovetype(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params,
                    int num_excluded_staples,
                    int max_disp,
                    int max_turns);
            CTCBLinkerRegrowthMCMovetype(const
                    CTCBLinkerRegrowthMCMovetype&) = delete;
            CTCBLinkerRegrowthMCMovetype& operator=(const
                    CTCBLinkerRegrowthMCMovetype&) = delete;

            void write_log_summary(ostream* log_entry) override;
            void reset_internal() override;

        protected:
            bool internal_attempt_move() override;
            virtual void add_tracker(bool accepted) override;

            /** Unasssign domains reset arrays for resetting */
            void reset_segment(vector<Domain*> segment, size_t last_di);

            /**
              * Select segment to be transformed and both linkers
              *
              * Returns staples to be regrown. Also sets up the constraints for
              * the fixed end biases.
              */
            virtual set<int> select_and_setup_segments(
                    vector<Domain*>& linker1,
                    vector<Domain*>& linker2,
                    vector<Domain*>& central_segment);

            virtual pair<int, int> select_internal_endpoints(
                    vector<Domain*> domains);

            /**
              * Setup constraints for fixed end biases
              *
              * Returns staples to be regrown.
              */
            set<int> setup_fixed_end_biases(
                    vector<Domain*>& linker1,
                    vector<Domain*>& linker2);

            bool domains_bound_externally(vector<Domain*> domains);
            bool scan_for_external_scaffold_domain(
                    Domain* domain,
                    vector<Domain*> domains,
                    set<int>& participating_chains);

            /** Transform the central segment */
            void transform_segment(
                    vector<Domain*> linker1,
                    vector<Domain*> linker2,
                    vector<Domain*> central_segment,
                    vector<Domain*> central_domains);

            /** Translate and rotate segment */
            bool apply_transformation(
                    vector<Domain*> central_domains,
                    VectorThree disp,
                    VectorThree center,
                    VectorThree axis,
                    int turns);

            /** Check if enough steps to regrow linkers */
            bool steps_less_than_distance(
                    vector<Domain*> linker1,
                    vector<Domain*> linker2);

            void revert_transformation(vector<Domain*> central_domains);

            int m_max_disp;
            int m_max_turns;

            CTCBLinkerRegrowthTracking m_tracker {};
            unordered_map<CTCBLinkerRegrowthTracking, MovetypeTracking> m_tracking {};
            vector<Domain*> m_linker_endpoints;
    };

    /**
      * Transformation of a segment and regrowth of its linkers
      *
      * Constrasts with parent in selection of segment to be transformed. Will
      * select domains that are contiguously bound out from the seed domain.
      */
    class ClusteredCTCBLinkerRegrowth:
        public CTCBLinkerRegrowthMCMovetype {

        public:
            ClusteredCTCBLinkerRegrowth(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params,
                    int num_excluded_staples,
                    int max_disp,
                    int max_turns);
            ClusteredCTCBLinkerRegrowth(const
                    ClusteredCTCBLinkerRegrowth&) = delete;
            ClusteredCTCBLinkerRegrowth& operator=(const
                    ClusteredCTCBLinkerRegrowth&) = delete;

        protected:
            pair<int, int> select_internal_endpoints(
                    vector<Domain*> domains) override;
    };

    /**
      * Transformation of a segment and regrowth of its linkers
      *
      * Constrasts with parent in selection of segment to be transformed. Will
      * select domains that are contiguously bound out from the seed domain.
      */
    class Clustered2CTCBLinkerRegrowth:
        public CTCBLinkerRegrowthMCMovetype {

        public:
            Clustered2CTCBLinkerRegrowth(
                    OrigamiSystem& origami_system,
                    RandomGens& random_gens,
                    IdealRandomWalks& ideal_random_walks,
                    vector<OrigamiOutputFile*> config_files,
                    string label,
                    SystemOrderParams& ops,
                    SystemBiases& biases,
                    InputParameters& params,
                    int num_excluded_staples,
                    int max_disp,
                    int max_turns,
                    int m_max_regrowth);
            Clustered2CTCBLinkerRegrowth(const
                    ClusteredCTCBLinkerRegrowth&) = delete;
            Clustered2CTCBLinkerRegrowth& operator=(const
                    ClusteredCTCBLinkerRegrowth&) = delete;

        private:
            set<int> select_and_setup_segments(
                    vector<Domain*>& linker1,
                    vector<Domain*>& linker2,
                    vector<Domain*>& central_segment) override;
            vector<Domain*> cyclic_select_internal_endpoints();
            vector<Domain*> linear_select_internal_endpoints();
    };
}

#endif // CB_MOVETYPES_H
