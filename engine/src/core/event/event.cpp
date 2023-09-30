#include "event.hpp"

#include "core/logger/logger.hpp"

namespace Engine {

    Ref<EventSystem> EventSystem::instance = nullptr;

    Event::Event(EventType type, std::string listener, SlotType handler) {
        this->handler = handler;
        this->listener = listener;
        this->type = type;
    };

    b8 Event::Handle(EventContext data) {
        return this->handler(type, data);
    };

    b8 EventSystem::Initialize() {
        if (!instance) {
            instance = CreateRef<EventSystem>();
        }

        instance->codes.resize((u8)EventType::Max);

        DEBUG("EventSystem successfully initialized.");

        return true;
    };

    void EventSystem::Shutdown() {
        DEBUG("Shutting down EventSystem");
        instance = nullptr;
    };

    Ref<EventSystem> EventSystem::GetInstance() {
        if (instance) {
            return instance;
        }
        return nullptr;
    };

     b8 EventSystem::RegisterEvent(EventType type, std::string listener, SlotType handler) {
        u64 registered_count = this->codes[(u16)type].observers.size();
        Event event = Event(type, listener, handler);
        this->codes[(u16)type].observers.push_back(event);
        return true;
    };

    b8 EventSystem::UnregisterEvent(EventType type, std::string listener) {
        if (!this->codes[(u16)type].observers.size()) {
            return false;
        }

        u64 registered_count = this->codes[(u16)type].observers.size();
        for (u64 i = 0; i < registered_count; ++i) {
            Event e = this->codes[(u16)type].observers[i];
            if (listener == e.listener) {
                this->codes[(u16)type].observers.erase(this->codes[(u16)type].observers.begin() + i);
                return true;
            }
        }

        return false;
    };

    b8 EventSystem::FireEvent(EventType type, EventContext data) {
        if (!this->codes[(u16)type].observers.size()) {
            return false;
        }

        u64 registered_count = this->codes[(u16)type].observers.size();
        b8 handled = false;
        for(u64 i = 0; i < registered_count; ++i) {
            Event e = this->codes[(u16)type].observers[i];
            if(!e.Handle(data)) {
                return false;
            } else {
                handled = true;
            }
        }

        return handled;
    };

};