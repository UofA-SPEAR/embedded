#ifndef __ENCODER_HPP
#define __ENCODER_HPP

#include "ch.h"
class Encoder {
    public:
        enum enc_t {
            none,
            linear_actuator,
            as5600,
            potentiometer,
        };
        void init(enc_t type) {
            _type = type;
        }
        enc_t getCurrType() {
            return _type;
        }
        virtual float readAngular() {return 0.0;}
        virtual float readSpeed() {return 0.0;}
    protected:
        enc_t _type;


};

class AS5600 : public Encoder {
// TODO: Insert AS5600 code here
};

class Potentiometer : public Encoder {
    private:
        long long pos = 0;
        float speed;
        long long prev;
    public:
        void init();
        float readSpeed() {
            chDbgAssert(false, "Potentiometer cannot read speed");
            return 0.0;
        }
        float readAngular(uint16_t adcValue);
        
};

class Quadrature : public Encoder {
    private:
        long long pos = 0;
        float speed, anglePerRev;
        long long prev;
    public:
        void init(int gearRatio);
        void enable();
        void set(uint16_t curr, sysinterval_t interval);
        float readAngular();
        long long getPos() {return pos/anglePerRev;}
        float readSpeed() {return speed/anglePerRev;}

};

#endif