using namespace QPI;  // Interfaz de Qubic

// Identificador del contrato (se suele definir una struct vacía con nombre único)
struct ServiceNFT252 { };  // Ejemplo de identificador (nombre arbitrario único)

// Definición del contrato inteligente, heredando de ContractBase
struct ServiceNFT : public ContractBase {
public:
    // Estructuras de entrada y salida para las funciones/procedimientos públicos:

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

    struct AddReview_input {
        uint64 offerId;
        uint8 rating;            // Puntuación 0-5 (por ejemplo)
        // (Opcionalmente un campo de comentario corto podría añadirse aquí)
    };
    struct AddReview_output {
        bool success;
    };

    struct UpdateStatus_input {
        uint64 offerId;
        uint8 newStatus;         // 0=activo, 1=reservado, 2=completado
    };
    struct UpdateStatus_output {
        bool success;
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
    // Estructura interna para almacenar datos de una oferta (NFT):
    struct OfferData {
        char companyName[64];
        char title[64];
        char imageRef[128];
        uint64 price;
        uint32 estimatedTime;
        char description[256];
        uint8 status;
        uint32 totalRating;   // suma de todas las puntuaciones de reseñas
        uint32 reviewCount;   // número de reseñas recibidas
        id owner;             // identidad del creador/propietario de la oferta
    };

    // Almacenamiento de las ofertas (NFTs). Supongamos un máximo de 100 ofertas:
    OfferData offers[100];
    bool offerExists[100];
    uint64 nextOfferId;

    // **Procedimiento público**: Agregar nueva oferta (crear NFT)
    PUBLIC_PROCEDURE(AddOffer)
        // Validar entradas básicas (p.ej., strings con terminación null implícita)
        require(qpi.getEntity(), "Caller must be an entity"); // ejemplo de require
        // Buscar un ID para la nueva oferta
        uint64 id = state.nextOfferId;
        require(id < 100, "Max offers reached");
        // Copiar datos de entrada a la nueva oferta en estado
        state.offers[id].price = input.price;
        state.offers[id].estimatedTime = input.estimatedTime;
        state.offers[id].status = 0;  // estado inicial: 0 (activo)
        state.offers[id].totalRating = 0;
        state.offers[id].reviewCount = 0;
        state.offers[id].owner = qpi.invocator();  // dueño = quien invoca
        // Copiar cadenas de texto de forma segura (asegurando terminación)
        memcpy(state.offers[id].companyName, input.companyName, sizeof(input.companyName));
        memcpy(state.offers[id].title,       input.title,       sizeof(input.title));
        memcpy(state.offers[id].imageRef,    input.imageRef,    sizeof(input.imageRef));
        memcpy(state.offers[id].description, input.description, sizeof(input.description));
        // Marcar oferta como existente y preparar el siguiente ID
        state.offerExists[id] = true;
        state.nextOfferId = id + 1;
        // Salida: devolver el ID asignado
        output.newOfferId = id;
    _  // fin del procedimiento AddOffer

    // **Procedimiento público**: Agregar reseña a una oferta
    PUBLIC_PROCEDURE(AddReview)
        require(qpi.getEntity(), "Caller must be an entity");
        uint64 id = input.offerId;
        require(id < state.nextOfferId && state.offerExists[id], "Offer does not exist");
        require(input.rating <= 5, "Invalid rating");
        // Actualizar acumuladores de reseñas
        state.offers[id].totalRating += input.rating;
        state.offers[id].reviewCount += 1;
        output.success = true;
    _  // fin del procedimiento AddReview

    // **Procedimiento público**: Actualizar estado de la oferta (activo/reservado/completado)
    PUBLIC_PROCEDURE(UpdateStatus)
        require(qpi.getEntity(), "Caller must be an entity");
        uint64 id = input.offerId;
        require(id < state.nextOfferId && state.offerExists[id], "Offer does not exist");
        // Opcional: solo permitir al dueño o a ciertas direcciones cambiar estado
        require(qpi.invocator() == state.offers[id].owner, "Not authorized");
        require(input.newStatus <= 2, "Invalid status");
        state.offers[id].status = input.newStatus;
        output.success = true;
    _  // fin del procedimiento UpdateStatus

    // **Función pública**: Obtener datos de una oferta (NFT) existente
    PUBLIC_FUNCTION(GetOffer)
        uint64 id = input.offerId;
        if (id < state.nextOfferId && state.offerExists[id]) {
            // Copiar datos de la oferta al struct de salida
            memcpy(output.companyName, state.offers[id].companyName, sizeof(output.companyName));
            memcpy(output.title,       state.offers[id].title,       sizeof(output.title));
            memcpy(output.imageRef,    state.offers[id].imageRef,    sizeof(output.imageRef));
            memcpy(output.description, state.offers[id].description, sizeof(output.description));
            output.price         = state.offers[id].price;
            output.estimatedTime = state.offers[id].estimatedTime;
            output.status        = state.offers[id].status;
            // Calcular promedio de puntuación (si hay reseñas)
            if (state.offers[id].reviewCount > 0) {
                output.averageRating = (uint8)(state.offers[id].totalRating / state.offers[id].reviewCount);
            } else {
                output.averageRating = 0;
            }
            output.reviewCount   = state.offers[id].reviewCount;
        } else {
            // Si la oferta no existe, devolver campos vacíos o valores por defecto
            output.status = 255;  // por ejemplo, 255 indica "no existe"
        }
    _  // fin de la función GetOffer

    // Registrar funciones y procedimientos con sus índices (IDs de entrada):
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(AddOffer, 1);
        REGISTER_USER_PROCEDURE(AddReview, 2);
        REGISTER_USER_PROCEDURE(UpdateStatus, 3);
        REGISTER_USER_FUNCTION(GetOffer, 1);
    _

    // Inicialización del estado (se ejecuta una vez al desplegar el contrato)
    INITIALIZE
        state.nextOfferId = 0;
        // Marcar todas las ofertas como no existentes inicialmente
        for (uint64 i = 0; i < 100; ++i) {
            state.offerExists[i] = false;
        }
    _
};
