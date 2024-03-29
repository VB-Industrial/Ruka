#pragma once

#include <cyphal/cyphal.h>
#include <cyphal/interfaces.h>
#include <libcanard/canard.h>
#include <memory>

#define OFFSET_PRIORITY 26U
#define OFFSET_SUBJECT_ID 8U
#define OFFSET_SERVICE_ID 14U
#define OFFSET_DST_NODE_ID 7U

#define FLAG_SERVICE_NOT_MESSAGE (UINT32_C(1) << 25U)
#define FLAG_ANONYMOUS_MESSAGE (UINT32_C(1) << 24U)
#define FLAG_REQUEST_NOT_RESPONSE (UINT32_C(1) << 24U)
#define FLAG_RESERVED_23 (UINT32_C(1) << 23U)
#define FLAG_RESERVED_07 (UINT32_C(1) << 7U)

typedef const std::shared_ptr<CyphalInterface> InterfacePtr;

template <typename T>
class AbstractSubscription : public IListener<CanardRxTransfer*> {
    typedef typename T::Type Type;
protected:
    const CanardTransferKind kind;
    CanardRxSubscription sub = {};
    InterfacePtr interface;

    void subscribe(CanardPortID port_id, CanardTransferKind kind) {
        sub.user_reference = reinterpret_cast<void*>(this);
        interface->subscribe(port_id, T::extent, kind, &sub);
    }

    virtual void handler(const Type&, CanardRxTransfer*) = 0;
public:
    AbstractSubscription(InterfacePtr interface, CanardPortID port_id)
        : AbstractSubscription(interface, port_id, CanardTransferKindMessage) {};
    AbstractSubscription(
        InterfacePtr interface,
        CanardPortID port_id,
        CanardTransferKind kind
    ): interface(interface), kind(kind) {
        subscribe(port_id, kind);
    };

    CanardFilter make_filter(CanardNodeID node_id) {
        CanardFilter out = {0};

        switch (kind) {
            case CanardTransferKindMessage:
                out.extended_can_id = (((uint32_t)sub.port_id) << OFFSET_SUBJECT_ID) |
                                      (((uint32_t) node_id)    << OFFSET_DST_NODE_ID);
                out.extended_mask = FLAG_SERVICE_NOT_MESSAGE | FLAG_RESERVED_07 | (CANARD_SUBJECT_ID_MAX << OFFSET_SUBJECT_ID) |
                                    (CANARD_NODE_ID_MAX << OFFSET_DST_NODE_ID);
                break;
            case CanardTransferKindRequest:
            case CanardTransferKindResponse:
                out = canardMakeFilterForService(sub.port_id,node_id);
                break;
        }

        return out;
    }
    void accept(CanardRxTransfer* transfer) {
        Type object;
        interface->deserialize_transfer<T>(&object, transfer);
        handler(object, transfer);
    }
};
