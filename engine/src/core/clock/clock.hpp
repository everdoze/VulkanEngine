#pragma once

#include "defines.hpp"

namespace Engine {

    class ENGINE_API Clock {
        public:
            void Update();
            void Start();
            void Stop();

            f64 GetElapsed();
            f64 GetStartTime();
        private:
            f64 start_time;
            f64 elapsed;
    };
      
};
