#include "coms.h"
#include "spear/drive/WheelOdom.h"

void coms_odom_broadcast(uint8_t wheel, float delta) {
    uint8_t msg_buf[20];
    spear_drive_WheelOdom msg;

    msg.wheel_id = wheel;
    msg.delta = delta;

    uint8_t len = spear_drive_WheelOdom_encode(&msg, msg_buf);

    if (len > 0) {
        canardBroadcast(&m_canard_ins,
            SPEAR_DRIVE_WHEELODOM_SIGNATURE,
            SPEAR_DRIVE_WHEELODOM_ID,
            &inout_transfer_id,
            0,
            msg_buf,
            len);
    }
}