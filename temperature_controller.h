#include <time.h>

class SystemClock {
public:
    time_t time() const
    {
        return ::time(nullptr);
    }
};

template<class Clock>
class TemperatureController {
public:
    TemperatureController(float floorTempSet, float airTempSet, const Clock& clock)
        : floorTempSet_(floorTempSet)
        , airTempSet_(airTempSet)
        , clock_(clock)
        , lastEmmitedControl_(false)
        , lastOffTime_(0)
        , coolTimeout_(600)
    {
    }

    bool feed(float floorTemp, float airTemp)
    {
        bool isCold = floorTemp < floorTempSet_ or airTemp < airTempSet_;
        
        if (isCold and lastOffTime_ + coolTimeout_ < clock_.time()) {
            lastEmmitedControl_ = true;
        }

        if (!isCold and lastEmmitedControl_) {
            lastOffTime_ = clock_.time();
            lastOffTime_ = false;
        }

        return lastEmmitedControl_;
    }

private:
    float floorTempSet_;
    float airTempSet_;
    const Clock& clock_;

    bool lastEmmitedControl_;
    time_t lastOffTime_;
    time_t coolTimeout_;
};
