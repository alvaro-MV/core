#include <cstring>
using namespace QPI;

struct ServiceNFT252 { };

struct ServiceNFT : public ContractBase {
public:
    struct AddOffer_input {
        char companyName[64];
        char title[64];
        char imageRef[128];
        uint64 price;
        uint32 estimatedTime;
        char description[256];
    };
    struct AddOffer_output {
        uint64 newOfferId;
    };

    struct GetOffer_input {
        uint64 offerId;
    };
    struct GetOffer_output {
        char companyName[64];
        char title[64];
        char imageRef[128];
        uint64 price;
        uint32 estimatedTime;
        char description[256];
        uint8 status;
        uint8 averageRating;
        uint32 reviewCount;
    };

private:
    struct OfferData {
        char companyName[64];
        char title[64];
        char imageRef[128];
        uint64 price;
        uint32 estimatedTime;
        char description[256];
        uint8 status;
        uint32 totalRating;
        uint32 reviewCount;
        id owner;
    };

    OfferData offers[100];
    bool offerExists[100];
    uint64 nextOfferId;

    struct AddOffer_locals {
        uint64 id;
    };

    PUBLIC_PROCEDURE_WITH_LOCALS(AddOffer)
        qpi.require(qpi.getEntity(qpi.invocator()), "Caller must be an entity");

        locals.id = state.nextOfferId;
        qpi.require(locals.id < 100, "Max offers reached");

        OfferData& offer = state.offers[locals.id];
        offer.price = input.price;
        offer.estimatedTime = input.estimatedTime;
        offer.status = 0;
        offer.totalRating = 0;
        offer.reviewCount = 0;
        offer.owner = qpi.invocator();

        std::memcpy(offer.companyName, input.companyName, sizeof(input.companyName));
        std::memcpy(offer.title,       input.title,       sizeof(input.title));
        std::memcpy(offer.imageRef,    input.imageRef,    sizeof(input.imageRef));
        std::memcpy(offer.description, input.description, sizeof(input.description));

        state.offerExists[locals.id] = true;
        state.nextOfferId = locals.id + 1;

        output.newOfferId = locals.id;
    _

    PUBLIC_FUNCTION(GetOffer)
        uint64 id = input.offerId;
        if (id < state.nextOfferId && state.offerExists[id]) {
            const OfferData& offer = state.offers[id];

            std::memcpy(output.companyName, offer.companyName, sizeof(output.companyName));
            std::memcpy(output.title,       offer.title,       sizeof(output.title));
            std::memcpy(output.imageRef,    offer.imageRef,    sizeof(output.imageRef));
            std::memcpy(output.description, offer.description, sizeof(output.description));

            output.price         = offer.price;
            output.estimatedTime = offer.estimatedTime;
            output.status        = offer.status;
            output.averageRating = (offer.reviewCount > 0)
                ? (uint8)(offer.totalRating / offer.reviewCount)
                : 0;
            output.reviewCount   = offer.reviewCount;
        } else {
            output.status = 255;
        }
    _

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(AddOffer, 1);
        REGISTER_USER_FUNCTION(GetOffer, 1);
    _

    INITIALIZE
        state.nextOfferId = 0;
        for (uint64 i = 0; i < 100; ++i) {
            state.offerExists[i] = false;
        }
    _
};
