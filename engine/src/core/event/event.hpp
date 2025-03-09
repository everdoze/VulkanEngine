#pragma once

#include "defines.hpp"

namespace Engine {

    enum class EventType {
        None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender, AppQuit,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        Debug1, Debug2, Debug3, Debug4, Debug5,
        RenderTargetsRefresh,
        Max
    }; 

    // enum class EventCategory {
	// 	None = 0,
	// 	Application    = BIT(0),
	// 	Input          = BIT(1),
	// 	Keyboard       = BIT(2),
	// 	Mouse          = BIT(3)
	// };

    // #define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; };\
	// 							   virtual EventType GetEventType() const override { return GetStaticType(); };\
	// 							   virtual const char* GetName() const override { return #type; };

    // #define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; };

    #define EventBind(func) [this](Engine::EventType type, Engine::EventContext& context) { return this->func(type, context); }

    typedef struct ENGINE_API EventContext {
        // 128 bytes
        union {
            i64 i64[2];
            u64 u64[2];
            f64 f64[2];

            i32 i32[4];
            u32 u32[4];
            f32 f32[4];

            i16 i16[8];
            u16 u16[8];

            i8 i8[16];
            u8 u8[16];

            char c[16];
        } data;
    } EventContext;

    using SlotType = std::function<b8(EventType type, EventContext& context)>;

    class ENGINE_API Event {
        public:
            Event(EventType type, std::string listener, SlotType handler);

            b8 Handle(EventContext data);

            std::string listener;
        private:
            SlotType handler;
            EventType type;
    };

    class EventDispatcher {
        public:
            std::vector<Event> observers;
    };

    class ENGINE_API EventSystem {
        public:
            static b8 Initialize();
            static void Shutdown();
            static EventSystem* GetInstance();


            b8 RegisterEvent(EventType type, std::string listener, SlotType handler);
            b8 UnregisterEvent(EventType type, std::string listener);
            b8 FireEvent(EventType type, EventContext data);
        private:
            std::vector<EventDispatcher> codes;
            static EventSystem* instance;
    };

};