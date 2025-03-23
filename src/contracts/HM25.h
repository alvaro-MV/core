using namespace QPI;  // Interfaz de Qubic

// Identificador del contrato
struct ServiceNFT252 { };  // Ejemplo de identificador (nombre arbitrario único)

// Definición del contrato inteligente, heredando de ContractBase
struct ServiceNFT : public ContractBase {
public:
    struct AddOffer_input {
        char companyName[64];
        char title[64];
        char imageRef[128];      // URL o hash de la imagen (ej. IPFS CID)
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

    PUBLIC_PROCEDURE(AddOffer)
        require(qpi.getEntity(), "Caller must be an entity");
        uint64 id = state.nextOfferId;
        require(id < 100, "Max offers reached");

        state.offers[id].price = input.price;
        state.offers[id].estimatedTime = input.estimatedTime;
        state.offers[id].status = 0;
        state.offers[id].totalRating = 0;
        state.offers[id].reviewCount = 0;
        state.offers[id].owner = qpi.invocator();

        memcpy(state.offers[id].companyName, input.companyName, sizeof(input.companyName));
        memcpy(state.offers[id].title,       input.title,       sizeof(input.title));
        memcpy(state.offers[id].imageRef,    input.imageRef,    sizeof(input.imageRef));
        memcpy(state.offers[id].description, input.description, sizeof(input.description));

        state.offerExists[id] = true;
        state.nextOfferId = id + 1;

        output.newOfferId = id;
    _

    PUBLIC_FUNCTION(GetOffer)
        uint64 id = input.offerId;
        if (id < state.nextOfferId && state.offerExists[id]) {
            memcpy(output.companyName, state.offers[id].companyName, sizeof(output.companyName));
            memcpy(output.title,       state.offers[id].title,       sizeof(output.title));
            memcpy(output.imageRef,    state.offers[id].imageRef,    sizeof(output.imageRef));
            memcpy(output.description, state.offers[id].description, sizeof(output.description));
            output.price         = state.offers[id].price;
            output.estimatedTime = state.offers[id].estimatedTime;
            output.status        = state.offers[id].status;
            output.averageRating = (state.offers[id].reviewCount > 0)
                ? (uint8)(state.offers[id].totalRating / state.offers[id].reviewCount)
                : 0;
            output.reviewCount   = state.offers[id].reviewCount;
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
