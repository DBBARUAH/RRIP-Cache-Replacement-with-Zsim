#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"


/* Generic replacement policy interface. A replacement policy is initialized by the cache (by calling setTop/BottomCC) and used by the cache array. Usage follows two models:
 * - On lookups, update() is called if the replacement policy is to be updated on a hit
 * - On each replacement, rank() is called with the req and a list of replacement candidates.
 * - When the replacement is done, replaced() is called. (See below for more detail.)
 */
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        uint64_t* array;
        uint32_t numLines;
        uint64_t rpvMax;
        bool isReplaced;

    public:
        explicit SRRIPReplPolicy(uint32_t _numLines, uint64_t _rpvMax)
        : numLines(_numLines), rpvMax(_rpvMax) {
            array = gm_calloc<uint64_t>(numLines);
            for (uint32_t i = 0; i < numLines; i++) array[i] = rpvMax;
        }
        ~SRRIPReplPolicy() {
            gm_free(array);
        }
        //If there is a cache hit
        void update(uint32_t id, const MemReq* req) {
            if (!isReplaced) { // if 'replaced' wasn't executed before this then update else isReplaced = False
                array[id] = 0;
            }
            isReplaced = false;
        }

        // Call if there is a replacement to decrease the other rpv values in the cache
        void replaced(uint32_t id) {
            array[id] = rpvMax - (uint64_t) 1;
            isReplaced = true; //To show that replacement had occured earlier

        }

        // To be called when replacement
        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            while (true) {
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                    if (array[*ci] == rpvMax) { // Replace the first block on the left of the array with RRPV of 3
                        return *ci;
                    }
                }
                // increment all RRPVs, if termination condition is not met
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                    array[*ci] = MIN(rpvMax, array[*ci] + (uint64_t) 1);
                }
            }
        }
        DECL_RANK_BINDINGS;
};
#endif // RRIP_REPL_H_
