#pragma once
#include <cyphal/definitions.h>
#ifdef __linux__

#include <string>

#include <cyphal/providers/provider.h>

class LinuxCAN : public AbstractCANProvider {
public:
    typedef const std::string& Handler;
private:
    int socketcan_handler;
    LinuxCAN(Handler can_interface, size_t queue_len, UtilityConfig& utilities);
public:

    template <class T, class... Args> static LinuxCAN* create_bss(
        std::byte** inout_buffer,
        Handler handler,
        CanardNodeID node_id,
        size_t queue_len,
        Args&&... args,
        UtilityConfig& utilities
    ) {
        std::byte* allocator_loc = *inout_buffer;
        auto allocator_ptr = new (allocator_loc) T(queue_len * sizeof(CanardTxQueueItem) * 2.5, args..., utilities);
    
        std::byte* provider_loc = allocator_loc + sizeof(T);
        auto ptr = new (provider_loc) LinuxCAN(handler, queue_len, utilities);
    
        ptr->setup<T>(allocator_ptr, node_id);

        *inout_buffer = provider_loc + sizeof(LinuxCAN);
        return ptr;
    }

    template <class T, class... Args> static LinuxCAN* create_heap(
        Handler handler,
        CanardNodeID node_id,
        size_t queue_len,
        Args&&... args,
        UtilityConfig& utilities
    ) {
        auto allocator_ptr = new T(queue_len * sizeof(CanardTxQueueItem) * 2.5, args..., utilities);
        auto ptr = new LinuxCAN(handler, queue_len, utilities);
        ptr->setup<T>(allocator_ptr, node_id);

        return ptr;
    }

    uint32_t len_to_dlc(size_t len) override;
    size_t dlc_to_len(uint32_t dlc) override;
    void can_loop() override;
    bool read_frame(CanardFrame*)  override;
    int write_frame(const CanardTxQueueItem* ti) override;
};
#endif
