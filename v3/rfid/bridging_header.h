#ifndef BRIDGING_HEADER_H
#define BRIDGING_HEADER_H
#ifdef __cplusplus
extern "C"
{
#endif
    void rfid_wrapper_setup();
    void rfid_wrapper_setdown();
    int rfid_wrapper_hasDetected();
#ifdef __cplusplus
}
#endif
#endif
